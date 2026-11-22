// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 prefill selection: the scores of a column's listed blocks of 8 entries, packed. blist holds
// indices into ix_bmax's block maxima (block b of a group of 64 at lane 8 (b % 8) + b / 8 of the group's
// vector), as ix_list wrote them. Listed block j of column c fills entries 8 j .. 8 j + 7 of cs with its
// scores and of cpos with their positions; an index below 0 fills them with -3e38 and position 0.
// tensors: 0 score (K, T) f32  1 blist (NBL, T) i32 | 2 cs (8 NBL, T) f32  3 cpos (8 NBL, T) i32
// index space: [NBL / 64, T]
#include "common.h"

#define NINF (-3.0e38f)

void main(tensor score, tensor blist, tensor cs, tensor cpos)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int64 lane = (int64)read_lane_id_4b_b();
    scalar_cache_reset();
    for (int c = off[1]; c < end[1]; c++)
        for (int g = off[0]; g < end[0]; g++)
            for (int j = 64 * g; j < 64 * g + 64; j++) {
                int5 lc = {j, c, 0, 0, 0};
                const int L = s_i32_ld_g(gen_addr(lc, blist));
                int5 oc = {8 * j, c, 0, 0, 0};
                if (L < 0) {
                    v_f32_st_tnsr_partial(oc, cs, (float64)NINF, 7, 0);
                    v_i32_st_tnsr_partial(oc, cpos, (int64)0, 7, 0);
                    continue;
                }
                const int l = L & 63;
                const int v = 8 * (L >> 6) + (l & 7);  // the score vector holding the block
                const int first = 8 * (l >> 3);         // its first lane there
                int5 sc = {64 * v, c, 0, 0, 0};
                v_f32_st_tnsr_partial(oc, cs, v_f32_ld_tnsr_b(sc, score), 7, first);
                v_i32_st_tnsr_partial(oc, cpos, lane + (int64)(64 * v), 7, first);
            }
}
