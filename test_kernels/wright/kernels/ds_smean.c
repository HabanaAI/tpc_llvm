// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 DSpark: the mean of the 4 mHC streams of each column, y[i, t] = (X[i] + X[H + i] + X[2H + i]
// + X[3H + i]) / 4 at t (the main model's hidden state that the drafter reads).
// tensors: 0 X (4H, T) f32 | 1 y (H, T) f32
// index space: [ceil(H / 64), T]
#include "common.h"

void main(tensor X, tensor y)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(y, 0);
    for (int t = off[1]; t < end[1]; t++)
        for (int b = off[0]; b < end[0]; b++) {
            int5 c = {64 * b, t, 0, 0, 0};
            float64 s = v_f32_ld_tnsr_b(c, X);
            c[0] += H;
            s += v_f32_ld_tnsr_b(c, X);
            c[0] += H;
            s += v_f32_ld_tnsr_b(c, X);
            c[0] += H;
            s += v_f32_ld_tnsr_b(c, X);
            c[0] = 64 * b;
            v_f32_st_tnsr(c, y, s * 0.25f);
        }
}
