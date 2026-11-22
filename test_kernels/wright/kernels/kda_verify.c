// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Speculative verify of batched KDA decode: Tq tokens of each of B sequences (columns b * Tq + t), each
// sequence's tokens in order from its arena state, which is not written. The math per token is kda_decode's. Each
// token's recurrent update is recorded instead: the state after token t is S_t = diag(e_t) S_{t-1} + k_t d_t^T per
// head, so pend keeps (k_t, d_t, e_t) and kda_commit replays the accepted tokens' updates.
// pconv keeps each token's conv inputs.
// tensors: 0 qkv (3C, B Tq) f32  1 fg (C, B Tq) f32  2 small (>= 256 + nh, B Tq) f32  3 conv_w (3C, 4) f32
//          4 arena (128, PR, NB) f32  5 bases (4, B) i32 [conv_src, ssm_src, -, -]  6 par (C + nh) f32
//          7 gate (C, B Tq) f32  8 w (128) f32
//        | 9 out (C, B Tq) bf16  10 pend (128, 3, nh, Tq, B) f32 [k | d | e]  11 pconv (3C, Tq, B) f32
// params: lb, eps. index space: [nh, B]
#include "common.h"

#define DK 128

__local__ float64 S[2 * DK];      // the head's state rows
__local__ float64 K4[8];          // k / q / e dual groups replicated (group g = rows 16g..16g+15)
__local__ float64 Q4[8];
__local__ float64 E4[8];
__local__ float64 YL[6];          // conv+SiLU outputs (q0 q1 k0 k1 v0 v1)

void main(tensor qkv, tensor fg, tensor small, tensor conv_w, tensor arena, tensor bases, tensor par,
          tensor gate, tensor w, tensor out, tensor pend, tensor pconv, float lb, float eps)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    const int C = get_dim_size(out, 0), C3 = 3 * C;
    const int Tq = get_dim_size(pend, 3);
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
        const int crow = conv_src % PR, cblk = conv_src / PR;
        const int srow0 = ssm_src % PR, sblk0 = ssm_src / PR;
        for (int h = off[0]; h < end[0]; h++) {
            // load the old state
            if (ssm_src >= 0) {
                int srow = srow0 + h * DK, sblk = sblk0;
                while (srow >= PR) { srow -= PR; sblk++; }
                int5 lc = {0, srow, sblk, 0, 0};
                for (int i = 0; i < DK; i++) {
                    lc[1] = srow + i;
                    lc[0] = 0;  S[2 * i] = v_f32_ld_tnsr_b(lc, arena);
                    lc[0] = 64; S[2 * i + 1] = v_f32_ld_tnsr_b(lc, arena);
                }
            } else {
                #pragma unroll 4
                for (int i = 0; i < 2 * DK; i++) S[i] = zero;
            }
            for (int t = 0; t < Tq; t++) {
                const int col = b * Tq + t;
                // ---- conv + SiLU: position t - 3 + k: the conv state, or an earlier token of this sequence
                #pragma unroll 1
                for (int c = 0; c < 6; c++) {
                    const int chan = (c >> 1) * C + h * DK + 64 * (c & 1);
                    float64 a = 0.0f;
                    for (int k = 0; k < 3; k++) {
                        const int pp = t - 3 + k;
                        float64 xv = zero;
                        if (pp >= 0) {
                            int5 xc = {chan, b * Tq + pp, 0, 0, 0};
                            xv = v_f32_ld_tnsr_b(xc, qkv);
                        } else if (conv_src >= 0) {
                            int r = crow + ((3 + pp) * C3 + chan) / DK, q = cblk;
                            if (r >= PR) { r -= PR; q++; }
                            int5 sc = {chan % DK, r, q, 0, 0};
                            xv = v_f32_ld_tnsr_b(sc, arena);
                        }
                        int5 wc = {chan, k, 0, 0, 0};
                        a = v_f32_mac_b(xv, v_f32_ld_tnsr_b(wc, conv_w), a);
                    }
                    int5 wc = {chan, 3, 0, 0, 0};
                    int5 xc = {chan, col, 0, 0, 0};
                    const float64 x3 = v_f32_ld_tnsr_b(xc, qkv);
                    a = v_f32_mac_b(x3, v_f32_ld_tnsr_b(wc, conv_w), a);
                    YL[c] = nl_silus(a);
                    int5 pc = {chan, t, b, 0, 0};
                    v_f32_st_tnsr(pc, pconv, x3);
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
                int5 fc = {h * DK, col, 0, 0, 0};
                const float64 fa = v_f32_ld_tnsr_b(fc, fg);
                fc[0] += 64;
                const float64 fb = v_f32_ld_tnsr_b(fc, fg);
                const float64 ea = nl_exp(lb * nl_sigm(aexp * (fa + dta)));
                const float64 eb = nl_exp(lb * nl_sigm(aexp * (fb + dtb)));
                int5 sb = {256 + h, col, 0, 0, 0};
                const float64 bl = BCAST_DG(v_f32_ld_tnsr_b(sb, small), 0);
                const float64 beta = nl_sigm(v_f32_shuffle_b(bl, IDXL[0], 0, bl));
                K4[0] = BCAST_DG(k0, 0); K4[1] = BCAST_DG(k0, 1); K4[2] = BCAST_DG(k0, 2); K4[3] = BCAST_DG(k0, 3);
                K4[4] = BCAST_DG(k1, 0); K4[5] = BCAST_DG(k1, 1); K4[6] = BCAST_DG(k1, 2); K4[7] = BCAST_DG(k1, 3);
                Q4[0] = BCAST_DG(q0, 0); Q4[1] = BCAST_DG(q0, 1); Q4[2] = BCAST_DG(q0, 2); Q4[3] = BCAST_DG(q0, 3);
                Q4[4] = BCAST_DG(q1, 0); Q4[5] = BCAST_DG(q1, 1); Q4[6] = BCAST_DG(q1, 2); Q4[7] = BCAST_DG(q1, 3);
                E4[0] = BCAST_DG(ea, 0); E4[1] = BCAST_DG(ea, 1); E4[2] = BCAST_DG(ea, 2); E4[3] = BCAST_DG(ea, 3);
                E4[4] = BCAST_DG(eb, 0); E4[5] = BCAST_DG(eb, 1); E4[6] = BCAST_DG(eb, 2); E4[7] = BCAST_DG(eb, 3);
                // pass 1: S[r] *= e[r], a = S^T k
                float64 a00 = 0.0f, a01 = 0.0f, a10 = 0.0f, a11 = 0.0f, a20 = 0.0f, a21 = 0.0f, a30 = 0.0f, a31 = 0.0f;
                for (int g = 0; g < 8; g++) {
                    const float64 k4 = K4[g], e4 = E4[g];
                    #pragma unroll
                    for (int e = 0; e < 16; e += 4) {
                        const int i = g * 16 + e;
                        const float64 kb0 = v_f32_shuffle_b(k4, IDXL[e], 0, k4), eg0 = v_f32_shuffle_b(e4, IDXL[e], 0, e4);
                        const float64 kb1 = v_f32_shuffle_b(k4, IDXL[e + 1], 0, k4), eg1 = v_f32_shuffle_b(e4, IDXL[e + 1], 0, e4);
                        const float64 kb2 = v_f32_shuffle_b(k4, IDXL[e + 2], 0, k4), eg2 = v_f32_shuffle_b(e4, IDXL[e + 2], 0, e4);
                        const float64 kb3 = v_f32_shuffle_b(k4, IDXL[e + 3], 0, k4), eg3 = v_f32_shuffle_b(e4, IDXL[e + 3], 0, e4);
                        const float64 r00 = S[2 * i] * eg0,     r01 = S[2 * i + 1] * eg0;
                        const float64 r10 = S[2 * i + 2] * eg1, r11 = S[2 * i + 3] * eg1;
                        const float64 r20 = S[2 * i + 4] * eg2, r21 = S[2 * i + 5] * eg2;
                        const float64 r30 = S[2 * i + 6] * eg3, r31 = S[2 * i + 7] * eg3;
                        S[2 * i] = r00; S[2 * i + 1] = r01; S[2 * i + 2] = r10; S[2 * i + 3] = r11;
                        S[2 * i + 4] = r20; S[2 * i + 5] = r21; S[2 * i + 6] = r30; S[2 * i + 7] = r31;
                        a00 = v_f32_mac_b(r00, kb0, a00); a01 = v_f32_mac_b(r01, kb0, a01);
                        a10 = v_f32_mac_b(r10, kb1, a10); a11 = v_f32_mac_b(r11, kb1, a11);
                        a20 = v_f32_mac_b(r20, kb2, a20); a21 = v_f32_mac_b(r21, kb2, a21);
                        a30 = v_f32_mac_b(r30, kb3, a30); a31 = v_f32_mac_b(r31, kb3, a31);
                    }
                }
                const float64 d0 = (YL[4] - ((a00 + a10) + (a20 + a30))) * beta;
                const float64 d1 = (YL[5] - ((a01 + a11) + (a21 + a31))) * beta;
                int5 rc = {0, 0, h, t, b};
                v_f32_st_tnsr(rc, pend, k0);
                rc[0] = 64; v_f32_st_tnsr(rc, pend, k1);
                rc[0] = 0; rc[1] = 1; v_f32_st_tnsr(rc, pend, d0);
                rc[0] = 64; v_f32_st_tnsr(rc, pend, d1);
                rc[0] = 0; rc[1] = 2; v_f32_st_tnsr(rc, pend, ea);
                rc[0] = 64; v_f32_st_tnsr(rc, pend, eb);
                // pass 2: S += k d^T, o = S^T q
                float64 b00 = 0.0f, b01 = 0.0f, b10 = 0.0f, b11 = 0.0f, b20 = 0.0f, b21 = 0.0f, b30 = 0.0f, b31 = 0.0f;
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
                        S[2 * i] = r00; S[2 * i + 1] = r01; S[2 * i + 2] = r10; S[2 * i + 3] = r11;
                        S[2 * i + 4] = r20; S[2 * i + 5] = r21; S[2 * i + 6] = r30; S[2 * i + 7] = r31;
                        b00 = v_f32_mac_b(r00, qb0, b00); b01 = v_f32_mac_b(r01, qb0, b01);
                        b10 = v_f32_mac_b(r10, qb1, b10); b11 = v_f32_mac_b(r11, qb1, b11);
                        b20 = v_f32_mac_b(r20, qb2, b20); b21 = v_f32_mac_b(r21, qb2, b21);
                        b30 = v_f32_mac_b(r30, qb3, b30); b31 = v_f32_mac_b(r31, qb3, b31);
                    }
                }
                const float64 o0 = (b00 + b10) + (b20 + b30);
                const float64 o1 = (b01 + b11) + (b21 + b31);
                float64 ms;
                NL_REDUCE_ADD(v_f32_mac_b(o0, o0, o1 * o1), IDXL, ms);
                const float64 rr = nl_rsqrt(ms * (1.0f / 128.0f) + eps);
                int5 gc = {h * DK, col, 0, 0, 0};
                const float64 ga = v_f32_ld_tnsr_b(gc, gate);
                gc[0] += 64;
                const float64 gb = v_f32_ld_tnsr_b(gc, gate);
                int5 oc = {h * DK, col, 0, 0, 0};
                v_bf16_st_tnsr(oc, out, pack_sigma(o0 * rr * wa * nl_sigm(ga), o1 * rr * wb * nl_sigm(gb)));
            }
        }
    }
}
