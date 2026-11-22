// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// GLM DSA indexer, per token: k = LayerNorm(k_raw) w + b (128), packed with the pooling gate logits:
//   kg[0:128] = k, kg[128:256] = gate logits (the 4:1 pool softmax adds ape[slot] later)
//   wts[h] = weights_proj(h)_h * NH^-0.5 * DI^-0.5   (the per-head score weights, relu scale folded in)
// tensors: 0 kgw (288, T) f32 [k_raw 128 | gate 128 | weights 32] (GEMM output)  1 kn_w (128) f32  2 kn_b (128) f32
//        | 3 kg (256, T) f32  4 wts (32, T) f32
// params: eps. index space: [T]
#include "common.h"

void main(tensor kgw, tensor kn_w, tensor kn_b, tensor kg, tensor wts, float eps)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    DECL_IDX(IDX)
    int5 z = {0, 0, 0, 0, 0};
    const float64 w0 = v_f32_ld_tnsr_b(z, kn_w), b0 = v_f32_ld_tnsr_b(z, kn_b);
    z[0] = 64;
    const float64 w1 = v_f32_ld_tnsr_b(z, kn_w), b1 = v_f32_ld_tnsr_b(z, kn_b);
    const float ws = 0.17677669529663687f * 0.08838834764831845f;     // 32^-0.5 * 128^-0.5
    for (int t = off[0]; t < end[0]; t++) {
        int5 c = {0, t, 0, 0, 0};
        const float64 k0 = v_f32_ld_tnsr_b(c, kgw);
        c[0] = 64;  const float64 k1 = v_f32_ld_tnsr_b(c, kgw);
        c[0] = 128; const float64 g0 = v_f32_ld_tnsr_b(c, kgw);
        c[0] = 192; const float64 g1 = v_f32_ld_tnsr_b(c, kgw);
        c[0] = 256; const float64 wv = v_f32_ld_tnsr_b(c, kgw);      // lanes 0..31 = the head weights
        float64 s, s2;
        NL_REDUCE_ADD(k0 + k1, IDX, s);
        const float64 mu = s * (1.0f / 128.0f);
        const float64 d0 = k0 - mu, d1 = k1 - mu;
        NL_REDUCE_ADD(v_f32_mac_b(d0, d0, d1 * d1), IDX, s2);
        const float64 r = nl_rsqrt(s2 * (1.0f / 128.0f) + eps);
        int5 o = {0, t, 0, 0, 0};
        v_f32_st_tnsr(o, kg, v_f32_mac_b(d0 * r, w0, b0));
        o[0] = 64;  v_f32_st_tnsr(o, kg, v_f32_mac_b(d1 * r, w1, b1));
        o[0] = 128; v_f32_st_tnsr(o, kg, g0);
        o[0] = 192; v_f32_st_tnsr(o, kg, g1);
        int5 wo = {0, t, 0, 0, 0};
        v_f32_st_tnsr(wo, wts, wv * ws);                            // the tensor holds lanes 0..31
    }
}
