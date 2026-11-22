// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Load recurrent state from the cache arena: sequence b's R rows of 512 bytes start at row
// bases[b] (one contiguous run) and land in out (., ., b). A base of
// -1 (a fresh sequence) loads zeros.
// The arena is viewed as (128, PR, NB) f32: row r is (., r % PR, r / PR).
// tensors: 0 arena (128, PR, NB) f32  1 bases (B,) i32 | 2 out (128, R, B) f32
// index space: [ceil(R / 16), B] (16 rows per member, the last one stops at R)
#include "common.h"

void main(tensor arena, tensor bases, tensor out)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    const int R = get_dim_size(out, 1);
    const float64 zero = 0.0f;
    scalar_cache_reset();
    for (int b = off[1]; b < end[1]; b++) {
        int5 bc = {b, 0, 0, 0, 0};
        const int base = s_i32_ld_g(gen_addr(bc, bases));
        for (int blk = off[0]; blk < end[0]; blk++) {
            int5 oc = {0, blk * 16, b, 0, 0};
            const int n = (R - blk * 16 < 16) ? R - blk * 16 : 16;
            if (base < 0) {
                for (int i = 0; i < n; i++, oc[1]++) {
                    oc[0] = 0;
                    v_f32_st_tnsr(oc, out, zero);
                    oc[0] = 64;
                    v_f32_st_tnsr(oc, out, zero);
                }
                continue;
            }
            const int row = base + blk * 16;
            int5 ac = {0, row % PR, row / PR, 0, 0};
            #pragma unroll 4
            for (int i = 0; i < n; i++) {
                ac[0] = 0;
                const float64 lo = v_f32_ld_tnsr_b(ac, arena);
                ac[0] = 64;
                const float64 hi = v_f32_ld_tnsr_b(ac, arena);
                oc[0] = 0;
                v_f32_st_tnsr(oc, out, lo);
                oc[0] = 64;
                v_f32_st_tnsr(oc, out, hi);
                oc[1]++;
                if (++ac[1] == PR) {
                    ac[1] = 0;
                    ac[2]++;
                }
            }
        }
    }
}
