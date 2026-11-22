// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Speculative verify of the batched gated-delta-net decode (FP8 path): Tq tokens of each of B
// sequences (columns b * Tq + t), each sequence's tokens in order from its arena state, which is
// not written. The math per token is gdn_decode's. Each token's recurrent update is recorded
// instead: the state after token t is S_t = eg_t * S_{t-1} + k_t d_t^T per head, so pend keeps
// (k_t, d_t, eg_t) and gdn_commit replays the accepted tokens' updates.
// pconv keeps each token's conv inputs (written by the channel's owner head).
// tensors: 0 p (C + 128 nv, B Tq) f32 [q | k | v | z] with per-token scale ps (1, B Tq)
//          2 ba (2 nv, B Tq)  3 conv_w (C, 4)  4 arena (128, PR, NB) f32  5 bases (4, B) i32
//          [conv_src, ssm_src, -, -]  6 ssm_a (nv)  7 dt_bias (nv)  8 ssm_norm (128)
//        | 9 out (128 nv, B Tq) f32  10 pend (128, 3, nv, Tq, B) f32 [k | d | eg]
//          11 pconv (C, Tq, B) f32
// params: eps, nk. index space: [nv, B]
#include "common.h"

// -DSIGGATE (Qwen3.8-Flash-Next): sigmoid output gate instead of silu
#ifdef SIGGATE
#define OGATE(z) nl_sigm(z)
#else
#define OGATE(z) nl_silu(z)
#endif

#define DK 128

__local__ float64 S[2 * DK];      // recurrent state
__local__ float64 K4[8];          // k / q dual groups replicated (group g = rows 16g..16g+15)
__local__ float64 Q4[8];
__local__ float64 YL[6];          // conv+SiLU outputs (q0 q1 k0 k1 v0 v1)
__local__ float64 HIST[18];       // per channel chunk: the conv inputs of tokens t-3 .. t-1

#define PLOAD(c) (v_f32_ld_tnsr_b(c, p) * tscale)

void main(tensor p, tensor ps, tensor ba, tensor conv_w, tensor arena, tensor bases,
          tensor ssm_a, tensor dt_bias, tensor ssm_norm, tensor out, tensor pend, tensor pconv,
          float eps, int nk)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    const int C = get_dim_size(conv_w, 0);
    const int nv = get_dim_size(out, 0) / DK;
    const int Tq = get_dim_size(pend, 3);
    scalar_cache_reset();
    const float qscale = 0.08838834764831845f;   // 1/sqrt(128)

    uchar256 IDXL[16];
    #pragma unroll
    for (int e = 0; e < 16; e++) IDXL[e] = SHUF_IDX(e);

    const int qoff = 0, koff = nk * DK, voff = 2 * nk * DK, zoff = voff + nv * DK;
    int5 z5 = {0, 0, 0, 0, 0};
    const float64 nw0 = v_f32_ld_tnsr_b(z5, ssm_norm);
    z5[0] = 64;
    const float64 nw1 = v_f32_ld_tnsr_b(z5, ssm_norm);
    const float64 zero = 0.0f;

    for (int b = off[1]; b < end[1]; b++) {
        int5 bc = {0, b, 0, 0, 0};
        const int conv_src = s_i32_ld_g(gen_addr(bc, bases));
        bc[0] = 1;
        const int ssm_src = s_i32_ld_g(gen_addr(bc, bases));
        const int crow = conv_src % PR, cblk = conv_src / PR;
        const int srow0 = ssm_src % PR, sblk0 = ssm_src / PR;
        for (int h = off[0]; h < end[0]; h++) {
            int kh = h;                        // shared q/k head = h % nk
            while (kh >= nk) kh -= nk;
            const bool owns_qk = h < nk;       // records the shared q/k channels' conv inputs
            int chan[6];
            chan[0] = qoff + kh * DK; chan[1] = qoff + kh * DK + 64;
            chan[2] = koff + kh * DK; chan[3] = koff + kh * DK + 64;
            chan[4] = voff + h * DK;  chan[5] = voff + h * DK + 64;
            if (ssm_src >= 0) {
                int srow = srow0 + h * DK, sblk = sblk0;
                while (srow >= PR) { srow -= PR; sblk++; }
                int5 ac = {0, srow, sblk, 0, 0};
                #pragma unroll 4
                for (int i = 0; i < DK; i++, ac[1]++) {
                    ac[0] = 0;  S[2 * i] = v_f32_ld_tnsr_b(ac, arena);
                    ac[0] = 64; S[2 * i + 1] = v_f32_ld_tnsr_b(ac, arena);
                }
            } else {
                #pragma unroll 4
                for (int i = 0; i < 2 * DK; i++) S[i] = zero;
            }
            #pragma unroll 1
            for (int c = 0; c < 6; c++) {
                #pragma unroll
                for (int k = 0; k < 3; k++) {
                    float64 xv = zero;
                    if (conv_src >= 0) {
                        int5 sc = {chan[c] % DK, crow + (k * C + chan[c]) / DK, cblk, 0, 0};
                        xv = v_f32_ld_tnsr_b(sc, arena);
                    }
                    HIST[3 * c + k] = xv;
                }
            }
            int5 hc = {h, 0, 0, 0, 0};
            const float64 A0 = BCAST_DG(v_f32_ld_tnsr_b(hc, ssm_a), 0);
            const float64 D0 = BCAST_DG(v_f32_ld_tnsr_b(hc, dt_bias), 0);
            const float64 Ah = v_f32_shuffle_b(A0, IDXL[0], 0, A0);
            const float64 DTh = v_f32_shuffle_b(D0, IDXL[0], 0, D0);

            for (int t = 0; t < Tq; t++) {
                const int col = b * Tq + t;
                const float64 tscale = TOKEN_SCALE(ps, col, IDXL[0]);
                // ---- conv + SiLU over the inputs t-3 .. t; record input t
                #pragma unroll 1
                for (int c = 0; c < 6; c++) {
                    float64 a = 0.0f;
                    #pragma unroll
                    for (int k = 0; k < 3; k++) {
                        int5 wc = {chan[c], k, 0, 0, 0};
                        a = v_f32_mac_b(v_f32_ld_tnsr_b(wc, conv_w), HIST[3 * c + k], a);
                    }
                    int5 wc = {chan[c], 3, 0, 0, 0};
                    int5 xc = {chan[c], col, 0, 0, 0};
                    const float64 x3 = PLOAD(xc);
                    a = v_f32_mac_b(v_f32_ld_tnsr_b(wc, conv_w), x3, a);
                    YL[c] = nl_silu(a);
                    HIST[3 * c] = HIST[3 * c + 1];
                    HIST[3 * c + 1] = HIST[3 * c + 2];
                    HIST[3 * c + 2] = x3;
                    if (c >= 4 || owns_qk) {
                        int5 pc = {chan[c], t, b, 0, 0};
                        v_f32_st_tnsr(pc, pconv, x3);
                    }
                }
                // ---- l2 norms: 1 / max(||x||, eps) == rsqrt(max(||x||^2, eps^2))
                float64 sq, sk2;
                NL_REDUCE_ADD(v_f32_mac_b(YL[0], YL[0], YL[1] * YL[1]), IDXL, sq);
                NL_REDUCE_ADD(v_f32_mac_b(YL[2], YL[2], YL[3] * YL[3]), IDXL, sk2);
                const float64 iq = nl_rsqrt(v_f32_max_b(sq, 1e-12f)) * qscale;
                const float64 ik = nl_rsqrt(v_f32_max_b(sk2, 1e-12f));
                const float64 q0 = YL[0] * iq, q1 = YL[1] * iq;
                const float64 k0 = YL[2] * ik, k1 = YL[3] * ik;
                // ---- gates (lane h of b / a)
                int5 gc = {h, col, 0, 0, 0};
                const float64 bv = BCAST_DG(v_f32_ld_tnsr_b(gc, ba), 0);
                gc[0] = nv + h;
                const float64 av = BCAST_DG(v_f32_ld_tnsr_b(gc, ba), 0);
                const float64 bh = v_f32_shuffle_b(bv, IDXL[0], 0, bv);
                const float64 ah = v_f32_shuffle_b(av, IDXL[0], 0, av) + DTh;
                const float64 sp = nl_softplus(ah);
                const float64 eg = nl_exp(Ah * sp);
                const float64 beta = nl_sigmoid(bh);
                // ---- delta rule
                K4[0] = BCAST_DG(k0, 0); K4[1] = BCAST_DG(k0, 1); K4[2] = BCAST_DG(k0, 2); K4[3] = BCAST_DG(k0, 3);
                K4[4] = BCAST_DG(k1, 0); K4[5] = BCAST_DG(k1, 1); K4[6] = BCAST_DG(k1, 2); K4[7] = BCAST_DG(k1, 3);
                Q4[0] = BCAST_DG(q0, 0); Q4[1] = BCAST_DG(q0, 1); Q4[2] = BCAST_DG(q0, 2); Q4[3] = BCAST_DG(q0, 3);
                Q4[4] = BCAST_DG(q1, 0); Q4[5] = BCAST_DG(q1, 1); Q4[6] = BCAST_DG(q1, 2); Q4[7] = BCAST_DG(q1, 3);
                // 4 accumulator pairs (rows i%4)
                float64 a00 = 0.0f, a01 = 0.0f, a10 = 0.0f, a11 = 0.0f, a20 = 0.0f, a21 = 0.0f, a30 = 0.0f, a31 = 0.0f;
                for (int g = 0; g < 8; g++) {
                    const float64 k4 = K4[g];
                    #pragma unroll
                    for (int e = 0; e < 16; e += 4) {
                        const int i = g * 16 + e;
                        const float64 kb0 = v_f32_shuffle_b(k4, IDXL[e], 0, k4);
                        const float64 kb1 = v_f32_shuffle_b(k4, IDXL[e + 1], 0, k4);
                        const float64 kb2 = v_f32_shuffle_b(k4, IDXL[e + 2], 0, k4);
                        const float64 kb3 = v_f32_shuffle_b(k4, IDXL[e + 3], 0, k4);
                        const float64 r00 = S[2 * i] * eg,       r01 = S[2 * i + 1] * eg;
                        const float64 r10 = S[2 * i + 2] * eg,   r11 = S[2 * i + 3] * eg;
                        const float64 r20 = S[2 * i + 4] * eg,   r21 = S[2 * i + 5] * eg;
                        const float64 r30 = S[2 * i + 6] * eg,   r31 = S[2 * i + 7] * eg;
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
                // ---- record the update (k, d, eg) for gdn_commit
                {
                    int5 rc = {0, 0, h, t, b};
                    v_f32_st_tnsr(rc, pend, k0);
                    rc[0] = 64; v_f32_st_tnsr(rc, pend, k1);
                    rc[0] = 0; rc[1] = 1; v_f32_st_tnsr(rc, pend, d0);
                    rc[0] = 64; v_f32_st_tnsr(rc, pend, d1);
                    rc[0] = 0; rc[1] = 2; v_f32_st_tnsr(rc, pend, eg);
                    rc[0] = 64; v_f32_st_tnsr(rc, pend, eg);
                }
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
                // ---- gated RMSNorm
                float64 ms;
                NL_REDUCE_ADD(v_f32_mac_b(o0, o0, o1 * o1), IDXL, ms);
                ms = ms * (1.0f / 128.0f);
                const float64 rr = nl_rsqrt(ms + eps);
                int5 zc = {zoff + h * DK, col, 0, 0, 0};
                const float64 z0 = PLOAD(zc);
                zc[0] += 64;
                const float64 z1 = PLOAD(zc);
                int5 oc = {h * DK, col, 0, 0, 0};
                v_f32_st_tnsr(oc, out, o0 * rr * nw0 * OGATE(z0));
                oc[0] += 64;
                v_f32_st_tnsr(oc, out, o1 * rr * nw1 * OGATE(z1));
            }
        }
    }
}
