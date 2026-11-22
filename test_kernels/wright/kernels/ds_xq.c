// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Per-token FP8 quantization of the routed experts' input, pair order (pack_nat128), one copy per route slot
// (slot = t * R + k).
//   s_t = max|x_t| / 240;  q = fp8(x_t / s_t).
// Each member takes 256 elements of one token (and reads the whole token for the max).
// tensors: 0 x (H, T) f32 | 1 q (H, 1, R*T) fp8  2 s (1, T) f32.  params: R.  index space: [H/256, T]
// LIN (ds_xq_lin.c): q in natural order.
#include "common.h"

void main(tensor x, tensor q, tensor s, int R)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(x, 0);
    DECL_IDX(IDX)
    for (int t = off[1]; t < end[1]; t++) {
        int5 c = {0, t, 0, 0, 0};
        float64 a0 = 0.0f, a1 = 0.0f;
        for (int i = 0; i < H; i += 128) {
            c[0] = i;      a0 = v_f32_max_b(a0, v_f32_abs_b(v_f32_ld_tnsr_b(c, x)));
            c[0] = i + 64; a1 = v_f32_max_b(a1, v_f32_abs_b(v_f32_ld_tnsr_b(c, x)));
        }
        float64 amax;
        NL_REDUCE_MAX(v_f32_max_b(a0, a1), IDX, amax);
        const float64 sc = token_scale(amax);
        const float64 inv = token_inv(sc);
        for (int b = off[0]; b < end[0]; b++) {
            for (int i = 256 * b; i < 256 * b + 256; i += 128) {
                c[0] = i;       const float64 va = v_f32_ld_tnsr_b(c, x) * inv;
                c[0] = i + 64;  const float64 vb = v_f32_ld_tnsr_b(c, x) * inv;
#ifdef LIN
                const minifloat256 p = pack_lin(va, vb, 0.0f, 0.0f);
#else
                const minifloat256 p = pack_nat128(va, vb);
#endif
                for (int k = 0; k < R; k++) {
                    int5 qc = {i, 0, t * R + k, 0, 0};
                    v_f8_st_tnsr_partial(qc, q, p, 127, 0);
                }
            }
            if (b == 0)
                STORE_TOKEN_SCALE(s, t, sc)
        }
    }
}
