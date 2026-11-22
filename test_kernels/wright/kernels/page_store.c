// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Store 512-byte rows into the cache arena. sink is a small i32 token output.
// Rows of -1 are skipped.
// tensors: 0 arena (128, PR, NB) i32  1 rows (N,) i32  2 vals (128, N) i32 | 3 sink (64, 1) i32
// index space: [N]
// DEP build: a trailing input tensor dep, unread.
//   tensors: 0 arena  1 rows  2 vals  3 dep | 4 sink
#include "common.h"

#ifdef DEP
void main(tensor arena, tensor rows, tensor vals, tensor dep, tensor sink)
#else
void main(tensor arena, tensor rows, tensor vals, tensor sink)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    scalar_cache_reset();
    for (int n = off[0]; n < end[0]; n++) {
        int5 rc = {n, 0, 0, 0, 0};
        const int r = s_i32_ld_g(gen_addr(rc, rows));
        if (r < 0) continue;
        int5 vc = {0, n, 0, 0, 0};
        int5 ac = {0, r % PR, r / PR, 0, 0};
        v_i32_st_tnsr(ac, arena, v_i32_ld_tnsr_b(vc, vals));
        vc[0] = 64;
        ac[0] = 64;
        v_i32_st_tnsr(ac, arena, v_i32_ld_tnsr_b(vc, vals));
    }
    if (off[0] == 0) {
        int5 sc = {0, 0, 0, 0, 0};
        v_i32_st_tnsr(sc, sink, (int64)1);
    }
}
