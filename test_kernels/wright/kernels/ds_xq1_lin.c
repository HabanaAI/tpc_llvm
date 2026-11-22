// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Per-token FP8 quantization in natural order with power-of-two scales, one member per token: the values of
// ds_xq_lin with one copy per token (R = 1).
//   s_t = the smallest power of two with max|x_t| <= 240 s_t;  q = fp8(x_t / s_t).
// tensors: 0 x (H, T) f32 | 1 q (H, T) fp8  2 s (1, T) f32.  index space: [T].  H % 256 == 0.
#define POW2
#include "common.h"

void main(tensor x, tensor q, tensor s)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(x, 0);
    DECL_IDX(IDX)
    for (int t = off[0]; t < end[0]; t++) {
        int5 c = {0, t, 0, 0, 0};
        float64 a0 = 0.0f, a1 = 0.0f;
        #pragma unroll 4
        for (int i = 0; i < H; i += 128) {
            c[0] = i;      a0 = v_f32_max_b(a0, v_f32_abs_b(v_f32_ld_tnsr_b(c, x)));
            c[0] = i + 64; a1 = v_f32_max_b(a1, v_f32_abs_b(v_f32_ld_tnsr_b(c, x)));
        }
        float64 amax;
        NL_REDUCE_MAX(v_f32_max_b(a0, a1), IDX, amax);
        const float64 sc = token_scale(amax);
        const float64 inv = token_inv(sc);
        #pragma unroll 2
        for (int i = 0; i < H; i += 256) {
            float64 v[4];
            #pragma unroll
            for (int k = 0; k < 4; k++) {
                c[0] = i + 64 * k;
                v[k] = v_f32_ld_tnsr_b(c, x) * inv;
            }
            int5 qc = {i, t, 0, 0, 0};
            v_f8_st_tnsr(qc, q, pack_lin(v[0], v[1], v[2], v[3]));
        }
        STORE_TOKEN_SCALE(s, t, sc)
    }
}
