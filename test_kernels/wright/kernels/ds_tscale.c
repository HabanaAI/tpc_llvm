// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// y * s[t]: a per-token scale (an FP8 GEMM's activation scale) on an f32 output.
// tensors: 0 y (M, T) f32  1 s (1, T) f32 | 2 out (M, T) f32.  index space: [M/128, T]
#include "common.h"

void main(tensor y, tensor s, tensor out)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    DECL_IDX(IDX)
    for (int t = off[1]; t < end[1]; t++) {
        const float64 sc = TOKEN_SCALE(s, t, IDX[0]);
        for (int b = off[0]; b < end[0]; b++) {
            int5 c = {128 * b, t, 0, 0, 0};
            v_f32_st_tnsr(c, out, v_f32_ld_tnsr_b(c, y) * sc);
            c[0] += 64;
            v_f32_st_tnsr(c, out, v_f32_ld_tnsr_b(c, y) * sc);
        }
    }
}
