// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Fused Qwen3.5 gated-delta-net block for one value head per member:
//   causal conv1d(k=4)+SiLU on the head's q/k/v channels, l2-norm(q,k), q*=1/sqrt(128),
//   gates beta = sigmoid(b), eg = exp(ssm_a * softplus(a + dt_bias)),
//   delta rule on the 128x128 f32 state,
//   gated RMSNorm: out = bf16( rms(o) * w * silu(z) )  (sigma order, feeds ssm_out GEMM).
// State layout S[h][i][j] (j = value dim, FCD).
// nk q/k heads, nv v heads (nv % nk == 0); v head h reads q/k head h % nk.
// With C = conv_dim = (2 nk + nv) * 128:
// tensors: 0 p (C + 128 nv + 2 nv, T) f32 [q | k | v | z (128 nv) | b (nv) | a (nv)]
//          1 conv_w (C,4) f32 [k][c]   2 cst_in (C,3) f32   3 sst_in (128,128,nv) f32
//          4 ssm_a (nv) f32   5 dt_bias (nv) f32   6 ssm_norm (128) f32   7 meta i32 [pos0, n_real]
//        | 8 out (128 nv,T) bf16   9 cst_out (C,3) f32   10 sst_out (128,128,nv) f32
// FP8 build (-DFP8): p is an FP8-GEMM output without b/a, p = [q | k | v | z] with per-token
// scale ps (p_true = p * ps[t]); b/a come from a separate BF16 GEMM ba (2 nv, T) = [b | a];
// the output is f32 (natural order) and gets quantized for the ssm_out FP8 GEMM by quant_fp8.
//   tensors: 0 p  1 ps (1,T)  2 ba  3 conv_w  4 cst_in  5 sst_in  6 ssm_a  7 dt_bias  8 ssm_norm
//            9 meta | 10 out (128 nv,T) f32  11 cst_out  12 sst_out
// params: eps, nk. index space: [nv heads]
// Decode build and prefill build (-DPREFILL), plus FP8 variants.
// -DMID (speculative decoding, T = 1 + drafts): two more outputs after sst_out, cst_mid (C,3,T-1) and
// sst_mid (128,128,nv,T-1): the conv / recurrent state after each token t < T-1 at index t (the rollback points
// when the second, drafted token is rejected).
// -DARR (pipelined speculative step, non-FP8): cst_in (C,3,S) / sst_in (128,128,nv,S) are read at slot meta[2]
// (the previous step's accepted index); cst_out (C,3,T) / sst_out (128,128,nv,T) hold the state after token t
// at slot t (every t). All T tokens are real.
#include "common.h"

// -DSIGGATE: sigmoid output gate instead of silu
#ifdef SIGGATE
#define OGATE(z) nl_sigm(z)
#else
#define OGATE(z) nl_silu(z)
#endif

#define DK 128

__local__ float64 S[2 * DK];      // recurrent state
__local__ float64 K4[8];          // k / q dual groups replicated (group g = rows 16g..16g+15)
__local__ float64 Q4[8];
__local__ float64 YL[6];          // conv+SiLU outputs of the current token (q0 q1 k0 k1 v0 v1)

#ifdef FP8
void main(tensor p, tensor ps, tensor ba, tensor conv_w, tensor cst_in, tensor sst_in, tensor ssm_a,
          tensor dt_bias, tensor ssm_norm, tensor meta, tensor out, tensor cst_out, tensor sst_out,
#ifdef MID
          tensor cst_mid, tensor sst_mid,
#endif
          float eps, int nk)
#define PLOAD(c, tok) (v_f32_ld_tnsr_b(c, p) * TOKEN_SCALE(ps, tok, IDXL[0]))
#else
void main(tensor p, tensor conv_w, tensor cst_in, tensor sst_in, tensor ssm_a, tensor dt_bias,
          tensor ssm_norm, tensor meta, tensor out, tensor cst_out, tensor sst_out,
#ifdef MID
          tensor cst_mid, tensor sst_mid,
#endif
          float eps, int nk)
#define PLOAD(c, tok) v_f32_ld_tnsr_b(c, p)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int T = get_dim_size(p, 1);
    scalar_cache_reset();
    int n_real = meta_i32(meta, 1);
    if (n_real > T) n_real = T;
#ifdef ARR
    const int sl = meta_i32(meta, 2);          // input state slot
#else
    const int sl = 0;
#endif
    const float qscale = 0.08838834764831845f;   // 1/sqrt(128)

    uchar256 IDXL[16];            // uniform shuffle controls
    #pragma unroll
    for (int e = 0; e < 16; e++) IDXL[e] = SHUF_IDX(e);

    const int nv = get_dim_size(sst_in, 2);
    const int qoff = 0, koff = nk * DK, voff = 2 * nk * DK;
    const int zoff = voff + nv * DK, boff = zoff + nv * DK, aoff = boff + nv;
    int5 z5 = {0, 0, 0, 0, 0};
    const float64 nw0 = v_f32_ld_tnsr_b(z5, ssm_norm);
    z5[0] = 64;
    const float64 nw1 = v_f32_ld_tnsr_b(z5, ssm_norm);

    for (int h = off[0]; h < end[0]; h++) {
        int kh = h;                        // shared q/k head = h % nk
        while (kh >= nk) kh -= nk;
        const bool owns_qk = h < nk;       // writes the conv state of the shared q/k channels
        int chan[6];
        chan[0] = qoff + kh * DK; chan[1] = qoff + kh * DK + 64;
        chan[2] = koff + kh * DK; chan[3] = koff + kh * DK + 64;
        chan[4] = voff + h * DK;  chan[5] = voff + h * DK + 64;
        int5 sc = {0, 0, h, sl, 0};
        #pragma unroll 4
        for (int i = 0; i < DK; i++) {
            sc[1] = i;
            sc[0] = 0;  S[2 * i] = v_f32_ld_tnsr_b(sc, sst_in);
            sc[0] = 64; S[2 * i + 1] = v_f32_ld_tnsr_b(sc, sst_in);
        }

        // per-head scalars: load at coordinate h (lane 0) and broadcast lane 0
        int5 hc = {h, 0, 0, 0, 0};
        const float64 A0 = BCAST_DG(v_f32_ld_tnsr_b(hc, ssm_a), 0);
        const float64 D0 = BCAST_DG(v_f32_ld_tnsr_b(hc, dt_bias), 0);
        const float64 Ah = v_f32_shuffle_b(A0, IDXL[0], 0, A0);
        const float64 DTh = v_f32_shuffle_b(D0, IDXL[0], 0, D0);

        for (int t = 0; t < T; t++) {
            int5 oc = {h * DK, t, 0, 0, 0};
            if (t >= n_real) {
#ifdef FP8
                v_f32_st_tnsr(oc, out, 0.0f);
                oc[0] += 64;
                v_f32_st_tnsr(oc, out, 0.0f);
#else
                v_bf16_st_tnsr(oc, out, (bfloat128)0.0f);
#endif
                continue;
            }
            // ---- conv + SiLU
            // taps: tap k multiplies input t-3+k (from p, or from the incoming state for t-3+k < 0)
#ifdef PREFILL
            #pragma unroll
#else
            #pragma unroll 1
#endif
            for (int c = 0; c < 6; c++) {
                float64 a = 0.0f;
                #pragma unroll
                for (int k = 0; k < 4; k++) {
                    const int tt = t - 3 + k;
                    int5 wc = {chan[c], k, 0, 0, 0};
                    const float64 wk = v_f32_ld_tnsr_b(wc, conv_w);
                    int5 xc = {chan[c], tt, 0, 0, 0};
                    int5 stc = {chan[c], tt + 3, sl, 0, 0};
                    const float64 xv = tt >= 0 ? PLOAD(xc, tt) : v_f32_ld_tnsr_b(stc, cst_in);
                    a = v_f32_mac_b(wk, xv, a);
                }
                YL[c] = nl_silu(a);
            }
            // ---- l2 norms: 1 / max(||.||, eps)
            // 1/max(||x||, eps) == rsqrt(max(||x||^2, eps^2))
            float64 sq, sk2;
            NL_REDUCE_ADD(v_f32_mac_b(YL[0], YL[0], YL[1] * YL[1]), IDXL, sq);
            NL_REDUCE_ADD(v_f32_mac_b(YL[2], YL[2], YL[3] * YL[3]), IDXL, sk2);
            const float64 iq = nl_rsqrt(v_f32_max_b(sq, 1e-12f)) * qscale;
            const float64 ik = nl_rsqrt(v_f32_max_b(sk2, 1e-12f));
            const float64 q0 = YL[0] * iq, q1 = YL[1] * iq;
            const float64 k0 = YL[2] * ik, k1 = YL[3] * ik;
            // ---- gates (lane h of b / a)
#ifdef FP8
            int5 gc = {h, t, 0, 0, 0};
            const float64 bv = BCAST_DG(v_f32_ld_tnsr_b(gc, ba), 0);
            gc[0] = nv + h;
            const float64 av = BCAST_DG(v_f32_ld_tnsr_b(gc, ba), 0);
#else
            int5 gc = {boff + h, t, 0, 0, 0};
            const float64 bv = BCAST_DG(v_f32_ld_tnsr_b(gc, p), 0);
            gc[0] = aoff + h;
            const float64 av = BCAST_DG(v_f32_ld_tnsr_b(gc, p), 0);
#endif
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
            int5 zc = {zoff + h * DK, t, 0, 0, 0};
            const float64 z0 = PLOAD(zc, t);
            zc[0] += 64;
            const float64 z1 = PLOAD(zc, t);
#ifdef FP8
            v_f32_st_tnsr(oc, out, o0 * rr * nw0 * OGATE(z0));
            oc[0] += 64;
            v_f32_st_tnsr(oc, out, o1 * rr * nw1 * OGATE(z1));
#else
            v_bf16_st_tnsr(oc, out, pack_sigma(o0 * rr * nw0 * OGATE(z0), o1 * rr * nw1 * OGATE(z1)));
#endif
#if defined(MID) || defined(ARR)
#ifdef ARR
#define cst_mid cst_out
#define sst_mid sst_out
            if (1) {                       // state after token t at slot t (every t)
#else
            if (t < T - 1) {               // state after token t (inputs t-2 .. t) at mid index t
#endif
                #pragma unroll 1
                for (int c = 0; c < 6; c++) {
                    if (c < 4 && !owns_qk)
                        continue;
                    #pragma unroll
                    for (int s2 = 0; s2 < 3; s2++) {
                        const int tt = t + s2 - 2;
                        int5 xc = {chan[c], tt, 0, 0, 0};
                        int5 stc = {chan[c], tt + 3, sl, 0, 0};
                        const float64 xv = tt >= 0 ? PLOAD(xc, tt) : v_f32_ld_tnsr_b(stc, cst_in);
                        int5 oc2 = {chan[c], s2, t, 0, 0};
                        v_f32_st_tnsr(oc2, cst_mid, xv);
                    }
                }
                int5 mc = sc;
                mc[3] = t;
                #pragma unroll 4
                for (int i = 0; i < DK; i++) {
                    mc[1] = i;
                    mc[0] = 0;  v_f32_st_tnsr(mc, sst_mid, S[2 * i]);
                    mc[0] = 64; v_f32_st_tnsr(mc, sst_mid, S[2 * i + 1]);
                }
            }
#endif
        }
#ifndef ARR
        // new conv state = last 3 real inputs (inputs n_real-3 .. n_real-1)
        #pragma unroll 1
        for (int c = 0; c < 6; c++) {
            if (c < 4 && !owns_qk)
                continue;
            #pragma unroll
            for (int s2 = 0; s2 < 3; s2++) {
                const int tt = n_real - 3 + s2;
                int5 xc = {chan[c], tt, 0, 0, 0};
                int5 stc = {chan[c], tt + 3, sl, 0, 0};
                const float64 xv = tt >= 0 ? PLOAD(xc, tt) : v_f32_ld_tnsr_b(stc, cst_in);
                int5 oc2 = {chan[c], s2, 0, 0, 0};
                v_f32_st_tnsr(oc2, cst_out, xv);
            }
        }
        #pragma unroll 4
        for (int i = 0; i < DK; i++) {
            sc[1] = i;
            sc[0] = 0;  v_f32_st_tnsr(sc, sst_out, S[2 * i]);
            sc[0] = 64; v_f32_st_tnsr(sc, sst_out, S[2 * i + 1]);
        }
#endif
    }
}
