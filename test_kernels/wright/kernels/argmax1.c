// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Argmax stage 1: each member scans a contiguous range of the logits and writes
// [max value, first index of max] (as f32) into lanes 0/1 of its partial row.
// tensors: 0 logits (V, B) f32 | 1 part (64, NB, B) f32.  index space: [NB, B] (column b of
// the logits is reduced into part (., ., b))
#include "common.h"

void main(tensor lg, tensor part)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int V = get_dim_size(lg, 0);
    const int NB = get_dim_size(part, 1);
    const int nvec = (V + 63) / 64;
    const int per = (nvec + NB - 1) / NB;
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    for (int col = off[1]; col < end[1]; col++) {
        for (int b = off[0]; b < end[0]; b++) {
            const int v0 = b * per * 64;
            int v1 = (b + 1) * per * 64;
            if (v1 > V) v1 = V;
            float64 best = -3.0e38f, bidx = 3.0e38f;
            int5 c = {0, col, 0, 0, 0};
            for (int i = v0; i < v1; i += 64) {
                c[0] = i;
                float64 x = v_f32_ld_tnsr_b(c, lg);
                const float64 ix = lanef + (float)i;
                x = v_f32_sel_less_f32_b(ix, (float)v1, x, -3.0e38f);
                // strictly greater keeps the first occurrence per lane
                bidx = v_f32_sel_grt_f32_b(x, best, ix, bidx);
                best = v_f32_max_b(best, x);
            }
            float64 m, nmi;
            NL_REDUCE_MAX(best, IDX, m);
            const float64 cand = v_f32_sel_eq_f32_b(best, m, -bidx, -3.0e38f);
            NL_REDUCE_MAX(cand, IDX, nmi);   // min index == -max(-index)
            const float64 mi = -nmi;
            float64 o = v_f32_sel_eq_f32_b(lanef, 0.0f, m, mi);
            int5 oc = {0, b, col, 0, 0};
            v_f32_st_tnsr(oc, part, o);
        }
    }
}
