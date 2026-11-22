// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Top-k over a vocabulary, stage 2: per column, the K largest of the 16 K logits of the chunks dfl_tk1 chose
// (keys), unordered: top_v[k, t] / top_i[k, t] for k < K.
// tensors: 0 x (V, T) f32  1 keys (K, T) i32 | 2 top_v (K, T) f32  3 top_i (K, T) i32
// K <= 16. index space: [T]
#include "common.h"
#define NINF (-3.0e38f)
void main(tensor x, tensor keys, tensor top_v, tensor top_i)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int V = get_dim_size(x, 0), K = get_dim_size(keys, 0);
    const int64 lane = (int64)read_lane_id_4b_b();
    const float64 lanef = v_convert_i32_to_f32_b(lane, SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int t = off[0]; t < end[0]; t++) {
        // lanes >= K hold +inf so that they are never the minimum replaced
        float64 tv = v_f32_sel_less_f32_b(lanef, (float)K, NINF, 3.0e38f);
        int64 ti = 0;
        for (int k = 0; k < K; k++) {
            int5 kc = {k, t, 0, 0, 0};
            const int key = s_i32_ld_g(gen_addr(kc, keys));
            const int base = 1024 * (key >> 6) + (key & 63);
            for (int j = 0; j < 16; j++) {
                const int i = base + 64 * j;
                if (i >= V) break;
                int5 xc = {i, t, 0, 0, 0};
                const float v = s_f32_ld_g(gen_addr(xc, x));
                // replace the smallest kept value (the lowest such lane) when v exceeds it
                float64 lo;
                NL_REDUCE_(tv, IDX, lo, v_f32_min_b);
                float64 at;
                NL_REDUCE_(v_f32_sel_eq_f32_b(tv, lo, lanef, 3.0e38f), IDX, at, v_f32_min_b);
                const float64 vv = v;
                const float64 hit = v_f32_sel_grt_f32_b(vv, lo, v_f32_sel_eq_f32_b(lanef, at, 1.0f, 0.0f), 0.0f);
                tv = v_f32_sel_eq_f32_b(hit, 1.0f, vv, tv);
                ti = v_i32_sel_eq_f32_b(hit, 1.0f, (int64)i, ti);
            }
        }
        int5 oc = {0, t, 0, 0, 0};
        v_f32_st_tnsr_partial(oc, top_v, tv, K - 1, 0);
        v_i32_st_tnsr_partial(oc, top_i, ti, K - 1, 0);
    }
}
