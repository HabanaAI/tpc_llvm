// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 DSpark: each column's argmax over its first V rows of x (ties: the lowest index), as i32 in
// every lane of out[:, c] and, at row c of idx, in lane 0.
// tensors: 0 x (V', T) f32 | 1 out (64, T) i32  2 idx (T) i32
// params: V. index space: [T]
#include "common.h"

void main(tensor x, tensor out, tensor idx, int V)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int64 lane = (int64)read_lane_id_4b_b();
    const float64 lanef = v_convert_i32_to_f32_b(lane, SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    for (int c = off[0]; c < end[0]; c++) {
        float64 best = -3.0e38f;
        float64 at = 0.0f;  // f32 indices: exact below 2^24
        for (int i = 0; i < V; i += 64) {
            int5 xc = {i, c, 0, 0, 0};
            float64 v = v_f32_ld_tnsr_b(xc, x);
            const float64 pos = lanef + (float)i;
            v = v_f32_sel_less_f32_b(pos, (float)V, v, -3.0e38f);
            at = v_f32_sel_grt_f32_b(v, best, pos, at);  // strictly greater: the first index of a lane wins
            best = v_f32_max_b(best, v);
        }
        float64 m;
        NL_REDUCE_MAX(best, IDX, m);
        float64 first;
        NL_REDUCE_(v_f32_sel_eq_f32_b(best, m, at, 3.0e38f), IDX, first, v_f32_min_b);
        const int64 r = v_convert_f32_to_i32_b(first, SW_RHNE, 0, 1, 0);
        int5 oc = {0, c, 0, 0, 0};
        v_i32_st_tnsr(oc, out, r);
        int5 ic = {c, 0, 0, 0, 0};
        v_i32_st_tnsr_partial(ic, idx, r, 0, 0);
    }
}
