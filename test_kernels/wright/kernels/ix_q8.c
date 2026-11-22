// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DSA index queries as FP8 (DeepSeek-V4.1): each 128-dim column as FP8 E4M3 in natural order (as mla_store_i8's
// index keys) with a power-of-two scale (max |x| / 240 rounded up).
// tensors: 0 x (128, W) f32 | 1 q (128, W) fp8  2 s (1, W) f32
// index space: [W]
#define POW2
#include "common.h"

void main(tensor x, tensor q, tensor s)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    DECL_IDX(IDX)
    for (int c = off[0]; c < end[0]; c++) {
        int5 xc = {0, c, 0, 0, 0};
        const float64 a = v_f32_ld_tnsr_b(xc, x);
        xc[0] = 64;
        const float64 b = v_f32_ld_tnsr_b(xc, x);
        float64 m;
        NL_REDUCE_MAX(v_f32_max_b(v_f32_abs_b(a), v_f32_abs_b(b)), IDX, m);
        const float64 sc = token_scale(m);
        const float64 inv = token_inv(sc);
        int5 qc = {0, c, 0, 0, 0};
        v_f8_st_tnsr_partial(qc, q, pack_lin(a * inv, b * inv, 0.0f, 0.0f), 127, 0);
        int5 sc0 = {0, c, 0, 0, 0};
        v_f32_st_tnsr_partial(sc0, s, sc, 0, 0);
    }
}
