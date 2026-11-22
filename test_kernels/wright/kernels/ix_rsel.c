// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DSA top-k threshold by radix, the choosing half of a pass (DeepSeek-V4.1; after ix_rcnt): with the counts
// summed over the column's members, lo + i 2^sh for the largest i (0 .. 15) whose count is at least KS. After
// the passes for sh = 28, 24, .. 0 from lo = 0, lo is the KS-th largest key, as ix_thr's. Columns that see at
// most KS entries get 1 (every key kept). The result is in every lane.
// tensors: 0 part (64, M, T) i32  1 vis (T) i32  2 state (64, T) i32 | 3 out (64, T) i32
// params: sh, KS. index space: [T]
#include "common.h"

void main(tensor part, tensor vis, tensor state, tensor out, int sh, int KS)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int M = get_dim_size(part, 1);
    const int64 lane = (int64)read_lane_id_4b_b();
    const int64 step = (int64)(1u << sh);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int c = off[0]; c < end[0]; c++) {
        int5 vc = {c, 0, 0, 0, 0};
        const int lim = s_i32_ld_g(gen_addr(vc, vis));
        int5 tc = {0, c, 0, 0, 0};
        int64 lo = v_i32_ld_tnsr_b(tc, state);
        if (lim > KS) {
            int64 tot = 0;
            for (int m = 0; m < M; m++) {
                int5 pc = {0, m, c, 0, 0};
                tot += v_i32_ld_tnsr_b(pc, part);
            }
            // counts fall with i: the lanes 1 .. 15 at or above KS are 1 .. i
            const int64 up = v_i32_sel_less_i32_b(tot, (int64)KS, (int64)0, step);
            int64 add;
            NL_REDUCE_I32(v_i32_sel_eq_i32_b(lane, (int64)0, (int64)0, up), IDX, add, v_i32_add_b);
            lo = lo + add;
        } else {
            lo = 1;
        }
        v_i32_st_tnsr(tc, out, lo);
    }
}
