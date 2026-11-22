// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Sum of the ranks' partials after an all-gather:
// out[c, t] = sum_r in[c, t, r]. tensors: 0 in (C, T, NR) f32 | 1 out (C, T) f32. index space: [C/128, T]
#include "common.h"

void main(tensor in, tensor out)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int NR = get_dim_size(in, 2);
    for (int t = off[1]; t < end[1]; t++) {
        for (int blk = off[0]; blk < end[0]; blk++) {
            int5 c = {blk * 128, t, 0, 0, 0};
            float64 a = v_f32_ld_tnsr_b(c, in);
            c[0] += 64;
            float64 b = v_f32_ld_tnsr_b(c, in);
            for (int r = 1; r < NR; r++) {
                c[2] = r;
                b += v_f32_ld_tnsr_b(c, in);
                c[0] -= 64;
                a += v_f32_ld_tnsr_b(c, in);
                c[0] += 64;
            }
            c[2] = 0;
            v_f32_st_tnsr(c, out, b);
            c[0] -= 64;
            v_f32_st_tnsr(c, out, a);
        }
    }
}
