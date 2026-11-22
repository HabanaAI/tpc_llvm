// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Chunked gated delta rule, output stage: o = gamma_t (Q S0)[t] + (P D)[t] (per head h, chunk c,
// token i; row-major 64 x 128 blocks, dims (128, 64, nv, NC)), then the gated RMSNorm
//   out[128 h + j, 64 c + i] = rms(o) * w * silu(z)     (0 for padded tokens)
// tensors (BF16 build): 0 QS  1 PD  2 GB  3 p (z at 128 (2 nk + nv) + 128 h)  4 ssm_norm (128)  5 meta
//                     | 6 out (128 nv, T) bf16 (sigma order, feeds the ssm_out GEMM)
// FP8 build: 0 QS  1 PD  2 GB  3 p  4 ps  5 ssm_norm  6 meta | 7 out (128 nv, T) f32 (natural order)
// params: eps, nk. index space: [nv, NC]
#include "common.h"

#ifdef FP8
#define STORE_OUT(oc, r0, r1) { v_f32_st_tnsr(oc, out, r0); oc[0] += 64; v_f32_st_tnsr(oc, out, r1); }
#else
#define STORE_OUT(oc, r0, r1) v_bf16_st_tnsr(oc, out, pack_sigma(r0, r1));
#endif

// -DSIGGATE: sigmoid output gate instead of silu
#ifdef SIGGATE
#define OGATE2(z) nl_sigm(z)
#else
#define OGATE2(z) nl_silu2(z)
#endif

#define DK 128
#define CH 64

#ifdef FP8
void main(tensor QS, tensor PD, tensor GB, tensor p, tensor ps, tensor ssm_norm, tensor meta, tensor out,
          float eps, int nk)
#define PLOAD(c, tok) (v_f32_ld_tnsr_b(c, p) * TOKEN_SCALE(ps, tok, IDX[0]))
#else
void main(tensor QS, tensor PD, tensor GB, tensor p, tensor ssm_norm, tensor meta, tensor out, float eps,
          int nk)
#define PLOAD(c, tok) v_f32_ld_tnsr_b(c, p)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    scalar_cache_reset();
    const int T = get_dim_size(out, 1);
    int n_real = meta_i32(meta, 1);
    if (n_real > T) n_real = T;
    const int nv = get_dim_size(QS, 2);
    const int zoff = (2 * nk + nv) * DK;
    DECL_IDX(IDX)
    int5 z5 = {0, 0, 0, 0, 0};
    const float64 nw0 = v_f32_ld_tnsr_b(z5, ssm_norm);
    z5[0] = 64;
    const float64 nw1 = v_f32_ld_tnsr_b(z5, ssm_norm);
    for (int h = off[0]; h < end[0]; h++) {
        for (int c = off[1]; c < end[1]; c++) {
            int5 gc = {0, 0, h, c, 0};
            const float64 GAM = nl_exp(v_f32_ld_tnsr_b(gc, GB));      // lanes = tokens
            // tokens in 4 dual groups: gamma_t broadcast from lane t
#define TOKS(DG) {                                                                          \
            const float64 Gd = BCAST_DG(GAM, DG);                                            \
            for (int e0 = 0; e0 < 16; e0 += 4) {                                             \
                float64 o0[4], o1[4], ms[4];                                                 \
                _Pragma("unroll")                                                            \
                for (int u = 0; u < 4; u++) {                                                \
                    const int i = (DG) * 16 + e0 + u;                                        \
                    const float64 gam = v_f32_shuffle_b(Gd, IDX[e0 + u], 0, Gd);             \
                    int5 ic = {0, i, h, c, 0};                                               \
                    o0[u] = v_f32_mac_b(gam, v_f32_ld_tnsr_b(ic, QS), v_f32_ld_tnsr_b(ic, PD)); \
                    ic[0] = 64;                                                              \
                    o1[u] = v_f32_mac_b(gam, v_f32_ld_tnsr_b(ic, QS), v_f32_ld_tnsr_b(ic, PD)); \
                }                                                                            \
                NL_REDUCE4_ADD(v_f32_mac_b(o0[0], o0[0], o1[0] * o1[0]), v_f32_mac_b(o0[1], o0[1], o1[1] * o1[1]), \
                               v_f32_mac_b(o0[2], o0[2], o1[2] * o1[2]), v_f32_mac_b(o0[3], o0[3], o1[3] * o1[3]), \
                               IDX, ms[0], ms[1], ms[2], ms[3]);                             \
                _Pragma("unroll")                                                            \
                for (int u = 0; u < 4; u++) {                                                \
                    const int t = c * CH + (DG) * 16 + e0 + u;                               \
                    const float64 rr = nl_rsqrt(ms[u] * (1.0f / 128.0f) + eps);              \
                    int5 zc = {zoff + h * DK, t, 0, 0, 0};                                   \
                    const float64 z0 = PLOAD(zc, t);                                         \
                    zc[0] += 64;                                                             \
                    const float64 z1 = PLOAD(zc, t);                                         \
                    const float64 rr0 = rr * (t < n_real ? 1.0f : 0.0f); /* 0/1 factor */      \
                    const float64 r0 = o0[u] * rr0 * nw0 * OGATE2(z0);                     \
                    const float64 r1 = o1[u] * rr0 * nw1 * OGATE2(z1);                     \
                    int5 oc = {h * DK, t, 0, 0, 0};                                          \
                    STORE_OUT(oc, r0, r1)                                                    \
                }                                                                            \
            } }
            TOKS(0) TOKS(1) TOKS(2) TOKS(3)
#undef TOKS
        }
    }
}
