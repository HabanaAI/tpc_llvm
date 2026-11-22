// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// f32 -> bf16 in sigma order per 128-row block.
// tensors: 0 x (C, T) f32 | 1 y (C, T) bf16. index space: [C/128, T]
#include "common.h"

void main(tensor x, tensor y)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    for (int t = off[1]; t < end[1]; t++)
        for (int b = off[0]; b < end[0]; b++) {
            int5 c = {128 * b, t, 0, 0, 0};
            const float64 a = v_f32_ld_tnsr_b(c, x);
            c[0] += 64;
            const float64 bb = v_f32_ld_tnsr_b(c, x);
            c[0] -= 64;
            v_bf16_st_tnsr(c, y, pack_sigma(a, bb));
        }
}
