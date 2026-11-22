// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Mistral-Large-4 MoE output + residual: out[:, t] = x[:, t] + sh[:, t] + sum_k wts[t, k] * y[:, t K + k]
// tensors: 0 y (H, K T) f32 (the routed down GEMV's slots)  1 wts (64, T) f32  2 x (H, T) f32  3 sh (H, T) f32
//          | 4 out (H, T) f32
// params: K. index space: [H/128, T].
#include "common.h"

void main(tensor y, tensor wts, tensor x, tensor sh, tensor out, int K)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    scalar_cache_reset();
    for (int t = off[1]; t < end[1]; t++) {
        for (int blk = off[0]; blk < end[0]; blk++) {
            int5 c = {blk * 128, t, 0, 0, 0};
            float64 a = v_f32_ld_tnsr_b(c, x) + v_f32_ld_tnsr_b(c, sh);
            c[0] += 64;
            float64 b = v_f32_ld_tnsr_b(c, x) + v_f32_ld_tnsr_b(c, sh);
            for (int k = 0; k < K; k++) {
                int5 wc = {k, t, 0, 0, 0};
                const float wk = s_f32_ld_g(gen_addr(wc, wts));
                int5 yc = {blk * 128, t * K + k, 0, 0, 0};
                a = v_f32_mac_b(v_f32_ld_tnsr_b(yc, y), wk, a);
                yc[0] += 64;
                b = v_f32_mac_b(v_f32_ld_tnsr_b(yc, y), wk, b);
            }
            v_f32_st_tnsr(c, out, b);
            c[0] -= 64;
            v_f32_st_tnsr(c, out, a);
        }
    }
}
