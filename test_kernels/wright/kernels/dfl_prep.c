// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DFlash drafter attention prologue (Qwen3 GQA, head_dim 128, full NeoX RoPE), laid out for the 256-wide
// slot attention kernels: KV groups are paired, pair pg = groups (2 pg, 2 pg + 1), and a pair's K (V) row holds
// [k_{2pg} | k_{2pg+1}] as two sigma-packed 128-halves. Queries are zero-extended to 256: head hd (group
// hd / 4) lives in pair pg = hd / 8 as pseudo-head ph = hd % 8, with its 128 values in half ph / 4.
//   q_hd = rope(rmsnorm_128(q_hd) * qn_w)  -> q_all column pg W + ph + HPG t (W = HPG T, HPG = 8)
//   k_g  = rope(rmsnorm_128(k_g) * kn_w)   -> kn_pg half (g & 1), row t
//   v_g                                     -> vn_pg half (g & 1), row sigma_inv(t)
// RoPE: [a | b] -> [a cos - b sin | b cos + a sin], cs row t = [cos(64) | sin(64)].
// p is an FP8-GEMM output with per-token scale ps: rows [q_0 .. q_{NH-1} | k_0 .. k_7 | v_0 .. v_7] x 128
// (WITH_Q build, NH = 32), or [k | v] (the KV build, injection of the target's features).
// tensors: 0 p (R, T) f32  1 ps (1,T) f32  2 qkn (256) f32 [q_norm | k_norm]  3 cs (128, T) f32
//        | [q_all (256, 4 W) bf16]  kn0 .. kn3 (256, T) bf16  vn0 .. vn3 (256, Tv) bf16
// params: eps. index space: [T]
#include "common.h"
#define NG 8
#define HPG 8
static inline void norm_rope128(float64 x[2], float64 w0, float64 w1, float64 cosv, float64 sinv, float eps)
{
    DECL_IDX(IDX)
    float64 acc = v_f32_mac_b(x[1], x[1], x[0] * x[0]);
    float64 ss;
    NL_REDUCE_ADD(acc, IDX, ss);
    const float64 r = nl_rsqrt(ss * (1.0f / 128.0f) + eps);
    const float64 a = x[0] * r * w0, b = x[1] * r * w1;
    x[0] = v_f32_mac_b(b, -sinv, a * cosv);
    x[1] = v_f32_mac_b(a, sinv, b * cosv);
}
static inline void load128(tensor p, int row, int t, float64 x[2], float64 s)
{
    int5 c = {row, t, 0, 0, 0};
    x[0] = v_f32_ld_tnsr_b(c, p) * s;
    c[0] += 64;
    x[1] = v_f32_ld_tnsr_b(c, p) * s;
}
#define PAIR_STORE(pg, a0, a1, a2, a3, c, x)                                                   \
    { const bfloat128 v_ = pack_sigma((x)[0], (x)[1]);                                           \
      switch (pg) { case 0: v_bf16_st_tnsr(c, a0, v_); break; case 1: v_bf16_st_tnsr(c, a1, v_); break; \
                    case 2: v_bf16_st_tnsr(c, a2, v_); break; default: v_bf16_st_tnsr(c, a3, v_); } }
#ifdef WITH_Q
void main(tensor p, tensor ps, tensor qkn, tensor cs, tensor q_all, tensor kn0, tensor kn1, tensor kn2,
          tensor kn3, tensor vn0, tensor vn1, tensor vn2, tensor vn3, float eps)
#else
void main(tensor p, tensor ps, tensor qkn, tensor cs, tensor kn0, tensor kn1, tensor kn2, tensor kn3,
          tensor vn0, tensor vn1, tensor vn2, tensor vn3, float eps)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
#ifdef WITH_Q
    const int NH = (get_dim_size(p, 0) - 2 * NG * 128) / 128;
    const int W = get_dim_size(q_all, 1) / (NG / 2);
    const int kbase = NH * 128;
#else
    const int kbase = 0;
#endif
    const int vbase = kbase + NG * 128;
    float64 qw0, qw1, kw0, kw1;
    {
        int5 wc = {0, 0, 0, 0, 0};
        qw0 = v_f32_ld_tnsr_b(wc, qkn);
        wc[0] = 64;
        qw1 = v_f32_ld_tnsr_b(wc, qkn);
        wc[0] = 128;
        kw0 = v_f32_ld_tnsr_b(wc, qkn);
        wc[0] = 192;
        kw1 = v_f32_ld_tnsr_b(wc, qkn);
    }
    DECL_IDX(IDX)
    const bfloat128 zero = 0.0f;
    for (int t = off[0]; t < end[0]; t++) {
        int5 cc = {0, t, 0, 0, 0};
        const float64 cosv = v_f32_ld_tnsr_b(cc, cs);
        cc[0] = 64;
        const float64 sinv = v_f32_ld_tnsr_b(cc, cs);
        const float64 sc = TOKEN_SCALE(ps, t, IDX[0]);
        float64 x[2];
#ifdef WITH_Q
        for (int hd = 0; hd < NH; hd++) {
            load128(p, 128 * hd, t, x, sc);
            norm_rope128(x, qw0, qw1, cosv, sinv, eps);
            const int pg = hd / HPG, ph = hd % HPG, e = (ph / 4) & 1;
            int5 qc = {128 * e, pg * W + ph + HPG * t, 0, 0, 0};
            v_bf16_st_tnsr(qc, q_all, pack_sigma(x[0], x[1]));
            qc[0] = 128 * (1 - e);
            v_bf16_st_tnsr(qc, q_all, zero);
        }
#endif
        const int vslot = sigma_inv(t);
        for (int g = 0; g < NG; g++) {
            const int pg = g >> 1, e = g & 1;
            load128(p, kbase + 128 * g, t, x, sc);
            norm_rope128(x, kw0, kw1, cosv, sinv, eps);
            int5 kc = {128 * e, t, 0, 0, 0};
            PAIR_STORE(pg, kn0, kn1, kn2, kn3, kc, x)
            load128(p, vbase + 128 * g, t, x, sc);
            int5 vc = {128 * e, vslot, 0, 0, 0};
            PAIR_STORE(pg, vn0, vn1, vn2, vn3, vc, x)
        }
    }
}
