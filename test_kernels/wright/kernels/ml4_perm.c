// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Mistral-Large-4: swap the two outer axes, y[:, b, a] = x[:, a, b] (head-major attention outputs -> token-major
// columns for the o-projection GEMV). tensors: 0 x (D, A, B) f32 | 1 y (D, B, A) f32. index space: [B, A].
#include "common.h"

void main(tensor x, tensor y)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int D = get_dim_size(x, 0);
    for (int a = off[1]; a < end[1]; a++)
        for (int b = off[0]; b < end[0]; b++)
            for (int i = 0; i < D; i += 64) {
                int5 xc = {i, a, b, 0, 0}, yc = {i, b, a, 0, 0};
                v_f32_st_tnsr(yc, y, v_f32_ld_tnsr_b(xc, x));
            }
}
