// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Prompt scoring: raw logprobs of chosen (row, token) pairs, logits[token, row] - lse[row], with
// lse from samp_select (sel_f lane 1). Pairs with row < 0 are padding (0).
// tensors: 0 logits (V, R) f32  1 sel_f (16, R) f32  2 pairs (2, N) i32 [row, token]
//        | 3 out (N) f32
// index space: [ceil(N / 64)] (64 pairs per member)
#include "common.h"

void main(tensor logits, tensor sel_f, tensor pairs, tensor out)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int N = get_dim_size(pairs, 1);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    scalar_cache_reset();
    for (int blk = off[0]; blk < end[0]; blk++) {
        float64 v = 0.0f;
        for (int l = 0; l < 64 && blk * 64 + l < N; l++) {
            int5 pc = {0, blk * 64 + l, 0, 0, 0};
            const int row = s_i32_ld_g(gen_addr(pc, pairs));
            pc[0] = 1;
            const int token = s_i32_ld_g(gen_addr(pc, pairs));
            if (row < 0)
                continue;
            int5 lc = {token, row, 0, 0, 0}, sc = {1, row, 0, 0, 0};
            const float x = s_f32_ld_g(gen_addr(lc, logits)) - s_f32_ld_g(gen_addr(sc, sel_f));
            v = v_f32_sel_eq_f32_b(lanef, (float)l, x, v);
        }
        int5 oc = {blk * 64, 0, 0, 0, 0};
        v_f32_st_tnsr(oc, out, v);
    }
}
