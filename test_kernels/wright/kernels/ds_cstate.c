// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 compressor, ratio 2: a token that leaves its group open is kept in the pending state for the next
// step, as [kv | score]. Column c goes to state row rows[c] (-1: none). ``after`` orders this store behind the
// pool that reads the previous state.
// tensors: 0 kv (512, T) f32  1 score (512, T) f32  2 rows (T) i32  3 state (1024, R) f32  4 after (unread)
//          | 5 sink (64, 1) i32
// index space: [T]
#include "common.h"

void main(tensor kv, tensor score, tensor rows, tensor state, tensor after, tensor sink)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    scalar_cache_reset();
    for (int c = off[0]; c < end[0]; c++) {
        int5 rc = {c, 0, 0, 0, 0};
        const int r = s_i32_ld_g(gen_addr(rc, rows));
        if (r < 0)
            continue;
        for (int i = 0; i < 512; i += 64) {
            int5 ic = {i, c, 0, 0, 0}, oc = {i, r, 0, 0, 0};
            v_f32_st_tnsr(oc, state, v_f32_ld_tnsr_b(ic, kv));
            oc[0] = 512 + i;
            v_f32_st_tnsr(oc, state, v_f32_ld_tnsr_b(ic, score));
        }
    }
    if (off[0] == 0) {
        int5 sc = {0, 0, 0, 0, 0};
        v_i32_st_tnsr(sc, sink, (int64)0);
    }
}
