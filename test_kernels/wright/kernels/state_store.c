// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Store recurrent state into the cache arena: vals (., ., b) goes to
// sequence b's R rows starting at row bases[b]; a base of -1 is skipped.
// tensors: 0 arena (128, PR, NB) f32  1 bases (B,) i32  2 vals (128, R, B) f32 | 3 sink (64, 1) i32
// index space: [ceil(R / 16), B] (16 rows per member, the last one stops at R); member 0 writes the sink.
#include "common.h"

void main(tensor arena, tensor bases, tensor vals, tensor sink)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    const int R = get_dim_size(vals, 1);
    scalar_cache_reset();
    for (int b = off[1]; b < end[1]; b++) {
        int5 bc = {b, 0, 0, 0, 0};
        const int base = s_i32_ld_g(gen_addr(bc, bases));
        if (base < 0)
            continue;
        for (int blk = off[0]; blk < end[0]; blk++) {
            const int row = base + blk * 16;
            int5 ac = {0, row % PR, row / PR, 0, 0};
            int5 vc = {0, blk * 16, b, 0, 0};
            const int n = (R - blk * 16 < 16) ? R - blk * 16 : 16;
            #pragma unroll 4
            for (int i = 0; i < n; i++) {
                vc[0] = 0;
                const float64 lo = v_f32_ld_tnsr_b(vc, vals);
                vc[0] = 64;
                const float64 hi = v_f32_ld_tnsr_b(vc, vals);
                ac[0] = 0;
                v_f32_st_tnsr(ac, arena, lo);
                ac[0] = 64;
                v_f32_st_tnsr(ac, arena, hi);
                vc[1]++;
                if (++ac[1] == PR) {
                    ac[1] = 0;
                    ac[2]++;
                }
            }
        }
    }
    if (off[0] == 0 && off[1] == 0) {
        int5 sc = {0, 0, 0, 0, 0};
        v_i32_st_tnsr(sc, sink, (int64)1);
    }
}
