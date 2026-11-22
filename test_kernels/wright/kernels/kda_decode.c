// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Batched KDA decode (GLM-5.3-Flash linear attention): one token of each of B sequences, the recurrent state
// read from and written back to the cache arena. Per member (head h, sequence b), gdn_decode's structure with
// the math of tools/glm_np.py kda_recurrent / kda_module: short conv (state = the last 3 inputs) + SiLU,
// l2-norm q (x dk^-0.5) / k, g = lb sigmoid(exp(A_log) (f + dt_bias)) per channel, beta = sigmoid(b), then
//   S <- diag(e^g) S;  kv = S^T k;  d = beta (v - kv);  S <- S + k d^T;  o = S^T q
// and the gated RMSNorm out = o / rms(o) w sigmoid(gate) (bf16, sigma order: the o_proj operand).
// S rows = key channels. Unlike GDN the decay is per row, and each head owns its q / k / v channels.
// Sequence b's rows come from bases (4, B) = [conv_src, ssm_src, -, ssm_dst]:
// - recurrent state: 128 rows per head, row 128 h + i holds S[h][i][.]; the base is a multiple of 128; written
//   back to the arena;
// - conv state: row j C3 / 128 + (channel / 128) of conv_src for j = 0..2 (C3 = 3C); the new one goes to
//   cst_out for state_store.
// A source of -1 reads zeros (a fresh sequence); ssm_dst = -1 skips the store (padding).
// tensors: 0 qkv (3C, B) f32  1 fg (C, B) f32  2 small (>= 256 + nh, B) f32 (row 256 + h = b logit)
//          3 conv_w (3C, 4) f32  4 arena (128, PR, NB) f32  5 bases (4, B) i32  6 par (C + nh) f32
//          [dt_bias | A_log]  7 gate (C, B) f32  8 w (128) f32 | 9 out (C, B) bf16  10 cst_out (128, CR, B) f32
// params: lb, eps. index space: [nh, B]
#include "common.h"

#define DK 128

__local__ float64 S[2 * DK];      // the head's state rows
__local__ float64 K4[8];          // k / q / e dual groups replicated (group g = rows 16g..16g+15)
__local__ float64 Q4[8];
__local__ float64 E4[8];
__local__ float64 YL[6];          // conv+SiLU outputs (q0 q1 k0 k1 v0 v1)

void main(tensor qkv, tensor fg, tensor small, tensor conv_w, tensor arena, tensor bases, tensor par,
          tensor gate, tensor w, tensor out, tensor cst_out, float lb, float eps)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    const int C = get_dim_size(out, 0), C3 = 3 * C;
    scalar_cache_reset();
    const float qs = 0.08838834764831845f;   // 128^-0.5

    uchar256 IDXL[16];            // uniform shuffle controls
    #pragma unroll
    for (int e = 0; e < 16; e++) IDXL[e] = SHUF_IDX(e);

    int5 z5 = {0, 0, 0, 0, 0};
    const float64 wa = v_f32_ld_tnsr_b(z5, w);
    z5[0] = 64;
    const float64 wb = v_f32_ld_tnsr_b(z5, w);
    const float64 zero = 0.0f;

    for (int b = off[1]; b < end[1]; b++) {
        int5 bc = {0, b, 0, 0, 0};
        const int conv_src = s_i32_ld_g(gen_addr(bc, bases));
        bc[0] = 1;
        const int ssm_src = s_i32_ld_g(gen_addr(bc, bases));
        bc[0] = 3;
        const int ssm_dst = s_i32_ld_g(gen_addr(bc, bases));
        // block and row of each base, once per sequence
        const int crow = conv_src % PR, cblk = conv_src / PR;
        const int srow0 = ssm_src % PR, sblk0 = ssm_src / PR;
        const int drow0 = ssm_dst % PR, dblk0 = ssm_dst / PR;
        for (int h = off[0]; h < end[0]; h++) {
            int srow = srow0 + h * DK, sblk = sblk0, drow = drow0 + h * DK, dblk = dblk0;
            while (srow >= PR) { srow -= PR; sblk++; }
            while (drow >= PR) { drow -= PR; dblk++; }
            const bool store = ssm_dst >= 0;
            // ---- conv + SiLU: taps 0..2 from the conv state (inputs t-3..t-1), tap 3 the token
            #pragma unroll 1
            for (int c = 0; c < 6; c++) {
                const int chan = (c >> 1) * C + h * DK + 64 * (c & 1);
                float64 a = 0.0f, x1 = zero, x2 = zero;
                #pragma unroll
                for (int k = 0; k < 3; k++) {
                    float64 xv = zero;
                    if (conv_src >= 0) {
                        int r = crow + (k * C3 + chan) / DK, q = cblk;
                        if (r >= PR) { r -= PR; q++; }
                        int5 sc = {chan % DK, r, q, 0, 0};
                        xv = v_f32_ld_tnsr_b(sc, arena);
                    }
                    if (k == 1) x1 = xv;
                    if (k == 2) x2 = xv;
                    int5 wc = {chan, k, 0, 0, 0};
                    a = v_f32_mac_b(xv, v_f32_ld_tnsr_b(wc, conv_w), a);
                }
                int5 wc = {chan, 3, 0, 0, 0};
                int5 xc = {chan, b, 0, 0, 0};
                const float64 x3 = v_f32_ld_tnsr_b(xc, qkv);
                a = v_f32_mac_b(x3, v_f32_ld_tnsr_b(wc, conv_w), a);
                YL[c] = nl_silus(a);
                // new conv state: inputs t-2, t-1, t
                #pragma unroll
                for (int j = 0; j < 3; j++) {
                    const int ow = j * C3 + chan;
                    int5 oc = {ow % DK, ow / DK, b, 0, 0};
                    v_f32_st_tnsr(oc, cst_out, j == 0 ? x1 : (j == 1 ? x2 : x3));
                }
            }
            float64 sq, sk;
            NL_REDUCE_ADD(v_f32_mac_b(YL[0], YL[0], YL[1] * YL[1]), IDXL, sq);
            NL_REDUCE_ADD(v_f32_mac_b(YL[2], YL[2], YL[3] * YL[3]), IDXL, sk);
            const float64 iq = nl_rsqrt(sq + 1e-6f) * qs;
            const float64 ik = nl_rsqrt(sk + 1e-6f);
            const float64 q0 = YL[0] * iq, q1 = YL[1] * iq;
            const float64 k0 = YL[2] * ik, k1 = YL[3] * ik;
            // ---- per-channel decay and beta
            int5 ac = {C + h, 0, 0, 0, 0};
            const float64 al = BCAST_DG(v_f32_ld_tnsr_b(ac, par), 0);
            const float64 aexp = nl_exp(v_f32_shuffle_b(al, IDXL[0], 0, al));
            int5 dc = {h * DK, 0, 0, 0, 0};
            const float64 dta = v_f32_ld_tnsr_b(dc, par);
            dc[0] += 64;
            const float64 dtb = v_f32_ld_tnsr_b(dc, par);
            int5 fc = {h * DK, b, 0, 0, 0};
            const float64 fa = v_f32_ld_tnsr_b(fc, fg);
            fc[0] += 64;
            const float64 fb = v_f32_ld_tnsr_b(fc, fg);
            const float64 ea = nl_exp(lb * nl_sigm(aexp * (fa + dta)));
            const float64 eb = nl_exp(lb * nl_sigm(aexp * (fb + dtb)));
            int5 sb = {256 + h, b, 0, 0, 0};
            const float64 bl = BCAST_DG(v_f32_ld_tnsr_b(sb, small), 0);
            const float64 beta = nl_sigm(v_f32_shuffle_b(bl, IDXL[0], 0, bl));
            // ---- delta rule
            K4[0] = BCAST_DG(k0, 0); K4[1] = BCAST_DG(k0, 1); K4[2] = BCAST_DG(k0, 2); K4[3] = BCAST_DG(k0, 3);
            K4[4] = BCAST_DG(k1, 0); K4[5] = BCAST_DG(k1, 1); K4[6] = BCAST_DG(k1, 2); K4[7] = BCAST_DG(k1, 3);
            Q4[0] = BCAST_DG(q0, 0); Q4[1] = BCAST_DG(q0, 1); Q4[2] = BCAST_DG(q0, 2); Q4[3] = BCAST_DG(q0, 3);
            Q4[4] = BCAST_DG(q1, 0); Q4[5] = BCAST_DG(q1, 1); Q4[6] = BCAST_DG(q1, 2); Q4[7] = BCAST_DG(q1, 3);
            E4[0] = BCAST_DG(ea, 0); E4[1] = BCAST_DG(ea, 1); E4[2] = BCAST_DG(ea, 2); E4[3] = BCAST_DG(ea, 3);
            E4[4] = BCAST_DG(eb, 0); E4[5] = BCAST_DG(eb, 1); E4[6] = BCAST_DG(eb, 2); E4[7] = BCAST_DG(eb, 3);
            // pass 1: S[r] *= e[r], a = S^T k
            float64 a00 = 0.0f, a01 = 0.0f, a10 = 0.0f, a11 = 0.0f, a20 = 0.0f, a21 = 0.0f, a30 = 0.0f, a31 = 0.0f;
            if (ssm_src >= 0) {
                int5 lc = {0, srow, sblk, 0, 0};
                for (int g = 0; g < 8; g++) {
                    const float64 k4 = K4[g], e4 = E4[g];
                    #pragma unroll
                    for (int e = 0; e < 16; e += 4) {
                        const int i = g * 16 + e;
                        const float64 kb0 = v_f32_shuffle_b(k4, IDXL[e], 0, k4), eg0 = v_f32_shuffle_b(e4, IDXL[e], 0, e4);
                        const float64 kb1 = v_f32_shuffle_b(k4, IDXL[e + 1], 0, k4), eg1 = v_f32_shuffle_b(e4, IDXL[e + 1], 0, e4);
                        const float64 kb2 = v_f32_shuffle_b(k4, IDXL[e + 2], 0, k4), eg2 = v_f32_shuffle_b(e4, IDXL[e + 2], 0, e4);
                        const float64 kb3 = v_f32_shuffle_b(k4, IDXL[e + 3], 0, k4), eg3 = v_f32_shuffle_b(e4, IDXL[e + 3], 0, e4);
                        lc[1] = srow + i;
                        lc[0] = 0;  const float64 r00 = v_f32_ld_tnsr_b(lc, arena) * eg0;
                        lc[0] = 64; const float64 r01 = v_f32_ld_tnsr_b(lc, arena) * eg0;
                        lc[1]++;
                        lc[0] = 0;  const float64 r10 = v_f32_ld_tnsr_b(lc, arena) * eg1;
                        lc[0] = 64; const float64 r11 = v_f32_ld_tnsr_b(lc, arena) * eg1;
                        lc[1]++;
                        lc[0] = 0;  const float64 r20 = v_f32_ld_tnsr_b(lc, arena) * eg2;
                        lc[0] = 64; const float64 r21 = v_f32_ld_tnsr_b(lc, arena) * eg2;
                        lc[1]++;
                        lc[0] = 0;  const float64 r30 = v_f32_ld_tnsr_b(lc, arena) * eg3;
                        lc[0] = 64; const float64 r31 = v_f32_ld_tnsr_b(lc, arena) * eg3;
                        S[2 * i] = r00; S[2 * i + 1] = r01; S[2 * i + 2] = r10; S[2 * i + 3] = r11;
                        S[2 * i + 4] = r20; S[2 * i + 5] = r21; S[2 * i + 6] = r30; S[2 * i + 7] = r31;
                        a00 = v_f32_mac_b(r00, kb0, a00); a01 = v_f32_mac_b(r01, kb0, a01);
                        a10 = v_f32_mac_b(r10, kb1, a10); a11 = v_f32_mac_b(r11, kb1, a11);
                        a20 = v_f32_mac_b(r20, kb2, a20); a21 = v_f32_mac_b(r21, kb2, a21);
                        a30 = v_f32_mac_b(r30, kb3, a30); a31 = v_f32_mac_b(r31, kb3, a31);
                    }
                }
            } else {
                #pragma unroll 4
                for (int i = 0; i < 2 * DK; i++) S[i] = zero;
            }
            const float64 d0 = (YL[4] - ((a00 + a10) + (a20 + a30))) * beta;
            const float64 d1 = (YL[5] - ((a01 + a11) + (a21 + a31))) * beta;
            // pass 2: S += k d^T, o = S^T q
            float64 b00 = 0.0f, b01 = 0.0f, b10 = 0.0f, b11 = 0.0f, b20 = 0.0f, b21 = 0.0f, b30 = 0.0f, b31 = 0.0f;
            int5 sc = {0, drow, dblk, 0, 0};
            for (int g = 0; g < 8; g++) {
                const float64 k4 = K4[g], q4 = Q4[g];
                #pragma unroll
                for (int e = 0; e < 16; e += 4) {
                    const int i = g * 16 + e;
                    const uchar256 x0 = IDXL[e], x1 = IDXL[e + 1], x2 = IDXL[e + 2], x3 = IDXL[e + 3];
                    const float64 kb0 = v_f32_shuffle_b(k4, x0, 0, k4), qb0 = v_f32_shuffle_b(q4, x0, 0, q4);
                    const float64 kb1 = v_f32_shuffle_b(k4, x1, 0, k4), qb1 = v_f32_shuffle_b(q4, x1, 0, q4);
                    const float64 kb2 = v_f32_shuffle_b(k4, x2, 0, k4), qb2 = v_f32_shuffle_b(q4, x2, 0, q4);
                    const float64 kb3 = v_f32_shuffle_b(k4, x3, 0, k4), qb3 = v_f32_shuffle_b(q4, x3, 0, q4);
                    const float64 r00 = v_f32_mac_b(kb0, d0, S[2 * i]),     r01 = v_f32_mac_b(kb0, d1, S[2 * i + 1]);
                    const float64 r10 = v_f32_mac_b(kb1, d0, S[2 * i + 2]), r11 = v_f32_mac_b(kb1, d1, S[2 * i + 3]);
                    const float64 r20 = v_f32_mac_b(kb2, d0, S[2 * i + 4]), r21 = v_f32_mac_b(kb2, d1, S[2 * i + 5]);
                    const float64 r30 = v_f32_mac_b(kb3, d0, S[2 * i + 6]), r31 = v_f32_mac_b(kb3, d1, S[2 * i + 7]);
                    sc[1] = drow + i;
                    sc[0] = 0;  v_f32_st_tnsr(sc, arena, r00, 0, store, 0);
                    sc[0] = 64; v_f32_st_tnsr(sc, arena, r01, 0, store, 0);
                    sc[1]++;
                    sc[0] = 0;  v_f32_st_tnsr(sc, arena, r10, 0, store, 0);
                    sc[0] = 64; v_f32_st_tnsr(sc, arena, r11, 0, store, 0);
                    sc[1]++;
                    sc[0] = 0;  v_f32_st_tnsr(sc, arena, r20, 0, store, 0);
                    sc[0] = 64; v_f32_st_tnsr(sc, arena, r21, 0, store, 0);
                    sc[1]++;
                    sc[0] = 0;  v_f32_st_tnsr(sc, arena, r30, 0, store, 0);
                    sc[0] = 64; v_f32_st_tnsr(sc, arena, r31, 0, store, 0);
                    b00 = v_f32_mac_b(r00, qb0, b00); b01 = v_f32_mac_b(r01, qb0, b01);
                    b10 = v_f32_mac_b(r10, qb1, b10); b11 = v_f32_mac_b(r11, qb1, b11);
                    b20 = v_f32_mac_b(r20, qb2, b20); b21 = v_f32_mac_b(r21, qb2, b21);
                    b30 = v_f32_mac_b(r30, qb3, b30); b31 = v_f32_mac_b(r31, qb3, b31);
                }
            }
            const float64 o0 = (b00 + b10) + (b20 + b30);
            const float64 o1 = (b01 + b11) + (b21 + b31);
            // ---- gated RMSNorm
            float64 ms;
            NL_REDUCE_ADD(v_f32_mac_b(o0, o0, o1 * o1), IDXL, ms);
            const float64 rr = nl_rsqrt(ms * (1.0f / 128.0f) + eps);
            int5 gc = {h * DK, b, 0, 0, 0};
            const float64 ga = v_f32_ld_tnsr_b(gc, gate);
            gc[0] += 64;
            const float64 gb = v_f32_ld_tnsr_b(gc, gate);
            int5 oc = {h * DK, b, 0, 0, 0};
            v_bf16_st_tnsr(oc, out, pack_sigma(o0 * rr * wa * nl_sigm(ga), o1 * rr * wb * nl_sigm(gb)));
        }
    }
}
