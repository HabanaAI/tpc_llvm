// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Store one residual column per sequence into its STATE block's hidden slot (the H f32 values
// the MTP layer of a continuing prefill reads at its first position).
// tensors: 0 arena (128, PR, NB) f32  1 x (H, T) f32  2 table (2, R) i32 [column, first row;
//          row -1: skip] | 3 sink (64, 1) i32
// index space: [R]; member 0 writes the sink.
#include "common.h"

void main(tensor arena, tensor x, tensor table, tensor sink)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    const int H = get_dim_size(x, 0);
    scalar_cache_reset();
    for (int r = off[0]; r < end[0]; r++) {
        int5 tc = {0, r, 0, 0, 0};
        const int col = s_i32_ld_g(gen_addr(tc, table));
        tc[0] = 1;
        const int row = s_i32_ld_g(gen_addr(tc, table));
        if (row < 0)
            continue;
        int5 ac = {0, row % PR, row / PR, 0, 0};
        int5 xc = {0, col, 0, 0, 0};
        for (int i = 0; i < H; i += 128) {
            xc[0] = i;
            ac[0] = 0;
            v_f32_st_tnsr(ac, arena, v_f32_ld_tnsr_b(xc, x));
            xc[0] = i + 64;
            ac[0] = 64;
            v_f32_st_tnsr(ac, arena, v_f32_ld_tnsr_b(xc, x));
            if (++ac[1] == PR) { ac[1] = 0; ac[2]++; }
        }
    }
    if (off[0] == 0) {
        int5 zc = {0, 0, 0, 0, 0};
        v_i32_st_tnsr(zc, sink, (int64)1);
    }
}
