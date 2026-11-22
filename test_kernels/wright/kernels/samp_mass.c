// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Sampling, pass 4 of 5 (mass): one member per (vocabulary chunk, row) sums the tempered weights
// of its kept tokens (key >= theta), from samp_scan's per-token keys and weights rescaled to the
// row maximum: exp((m_c - m) / T) * sum wt. 0 for greedy rows.
// tensors: 0 wkey (Vp, B) i32  1 wt (Vp, B) f32  2 stats (see samp_scan)  3 fin_i (see
//          samp_full)  4 sel_f (see samp_select) | 5 cmass (1, NC, B) f32
// index space: [NC, B]
#include "samp.h"

void main(tensor wkey, tensor wt, tensor stats, tensor fin_i, tensor sel_f, tensor cmass)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int nvec = get_dim_size(wkey, 0) / 64;
    const int NC = get_dim_size(cmass, 1);
    const int per = (nvec + NC - 1) / NC;
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int b = off[1]; b < end[1]; b++) {
        int5 pc = {0, b, 0, 0, 0};
        const int theta = s_i32_ld_g(gen_addr(pc, fin_i));
        const float m = s_f32_ld_g(gen_addr(pc, sel_f));
        pc[0] = 1; const int sample = s_i32_ld_g(gen_addr(pc, fin_i));
        pc[0] = 3; const float invT = s_f32_ld_g(gen_addr(pc, sel_f));
        for (int c = off[0]; c < end[0]; c++) {
            float64 s = 0.0f;
            if (sample) {
                const int v0 = c * per, v1 = v0 + per < nvec ? v0 + per : nvec;
                int5 xc = {0, b, 0, 0, 0};
                float64 s1 = 0.0f;
                int v = v0;
                for (; v + 1 < v1; v += 2) {
                    xc[0] = v * 64;
                    s += v_f32_sel_geq_i32_b(v_i32_ld_tnsr_b(xc, wkey), (int64)theta, v_f32_ld_tnsr_b(xc, wt), 0.0f);
                    xc[0] += 64;
                    s1 += v_f32_sel_geq_i32_b(v_i32_ld_tnsr_b(xc, wkey), (int64)theta, v_f32_ld_tnsr_b(xc, wt), 0.0f);
                }
                if (v < v1) {
                    xc[0] = v * 64;
                    s += v_f32_sel_geq_i32_b(v_i32_ld_tnsr_b(xc, wkey), (int64)theta, v_f32_ld_tnsr_b(xc, wt), 0.0f);
                }
                s += s1;
                NL_REDUCE_ADD(s, IDX, s)
                int5 sc = {0, c, b, 0, 0};
                const float mc = s_f32_ld_g(gen_addr(sc, stats));
                s = s * weight((float64)((mc - m) * invT));
            }
            int5 oc = {0, c, b, 0, 0};
            v_f32_st_tnsr(oc, cmass, s);
        }
    }
}
