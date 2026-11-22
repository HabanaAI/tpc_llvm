// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Argmax stage 2: reduce the per-member [max, index] partials to the token id of each column.
// tensors: 0 part (64, NB, B) f32 | 1 tok (1, B) i32.  index space: [B]
#include "common.h"

void main(tensor part, tensor tok)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int NB = get_dim_size(part, 1);
    for (int col = off[0]; col < end[0]; col++) {
        float64 best = -3.0e38f, bidx = 3.0e38f;
        int5 c = {0, 0, col, 0, 0};
        for (int b = 0; b < NB; b++) {
            c[1] = b;
            const float64 row = v_f32_ld_tnsr_b(c, part);
            // lane 0 = value, lane 1 = index: bring the index into lane 0
            const float64 val = BCAST_DG(row, 0);
            const float64 v = v_f32_shuffle_b(val, (unsigned char)SHUF_IDX(0), 0, val);
            const float64 ix = v_f32_shuffle_b(val, (unsigned char)SHUF_IDX(1), 0, val);
            // earlier members hold smaller indices: strict > keeps the first occurrence
            bidx = v_f32_sel_grt_f32_b(v, best, ix, bidx);
            best = v_f32_max_b(best, v);
        }
        const int64 ti = v_convert_f32_to_i32_b(bidx, SW_RHNE, 0, 1, 0);
        int5 oc = {0, col, 0, 0, 0};
        v_i32_st_tnsr(oc, tok, ti);
    }
}
