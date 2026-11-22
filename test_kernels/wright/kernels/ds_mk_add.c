// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 DSpark Markov step: each row's logits at its block's column i, in vocabulary order, plus the
// Markov bias: z[r V_r + v, b] = L[v, b block + i, r] + bias[r V_r + v, b] (L: the all-gathered logits of R
// ranks' vocabulary slices of V_r each).
// tensors: 0 L (V_r, T, R) f32  1 bias (V, B) f32 | 2 z (V, B) f32
// params: i, block. index space: [ceil(V_r / 64), B, R]
#include "common.h"

void main(tensor L, tensor bias, tensor z, int i, int block)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int Vr = get_dim_size(L, 0);
    for (int r = off[2]; r < end[2]; r++)
        for (int b = off[1]; b < end[1]; b++)
            for (int j = off[0]; j < end[0]; j++) {
                const int v = 64 * j;
                int5 lc = {v, b * block + i, r, 0, 0};
                int5 zc = {r * Vr + v, b, 0, 0, 0};
                const float64 x = v_f32_ld_tnsr_b(lc, L) + v_f32_ld_tnsr_b(zc, bias);
                if (v + 64 <= Vr)
                    v_f32_st_tnsr(zc, z, x);
                else  // the slice's last vector: lanes past V_r are the next rank's
                    v_f32_st_tnsr_partial(zc, z, x, Vr - v - 1, 0);
            }
}
