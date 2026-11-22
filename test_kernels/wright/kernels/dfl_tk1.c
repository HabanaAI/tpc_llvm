// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Top-k over a vocabulary, stage 1 (dfl_tk2 completes it): per column, the K chunks with the largest maxima.
// Chunk (g, l) of column t holds the 16 logits g * 1024 + 64 j + l (j = 0 .. 15); the K largest logits of a
// column lie in its K chunks with the largest maxima. keys[k, t] = 64 g + l of the k-th of them (equal
// maxima: the lowest key first).
// tensors: 0 x (V, T) f32 | 1 keys (K, T) i32
// K <= 16, V <= 262144. index space: [T]
#include "common.h"
#define NINF (-3.0e38f)
#define GMAX 256
__local__ float64 CM[GMAX];  // a column's chunk maxima, the chosen ones set to -inf
void main(tensor x, tensor keys)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int V = get_dim_size(x, 0), K = get_dim_size(keys, 0);
    const int G = (V + 1023) / 1024;
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    for (int t = off[0]; t < end[0]; t++) {
        int5 c = {0, t, 0, 0, 0};
        for (int g = 0; g < G; g++) {
            float64 m = NINF;
            const int base = 1024 * g;
            if (base + 1024 <= V) {
                #pragma unroll
                for (int j = 0; j < 16; j++) {
                    c[0] = base + 64 * j;
                    m = v_f32_max_b(m, v_f32_ld_tnsr_b(c, x));
                }
            } else {
                for (int j = 0; j < 16; j++) {
                    const int i = base + 64 * j;
                    if (i >= V) break;
                    c[0] = i;
                    m = v_f32_max_b(m, v_f32_sel_less_f32_b(lanef + (float)i, (float)V, v_f32_ld_tnsr_b(c, x), NINF));
                }
            }
            CM[g] = m;
        }
        int5 kc = {0, t, 0, 0, 0};
        for (int k = 0; k < K; k++) {
            float64 m = NINF;
            for (int g = 0; g < G; g++)
                m = v_f32_max_b(m, CM[g]);
            float64 mm;
            NL_REDUCE_MAX(m, IDX, mm);
            float64 where = 3.0e38f;  // the lowest key holding it
            for (int g = 0; g < G; g++)
                where = v_f32_min_b(where, v_f32_sel_eq_f32_b(CM[g], mm, (float)(64 * g) + lanef, 3.0e38f));
            float64 key;
            NL_REDUCE_(where, IDX, key, v_f32_min_b);
            for (int g = 0; g < G; g++)
                CM[g] = v_f32_sel_eq_f32_b((float)(64 * g) + lanef, key, NINF, CM[g]);
            kc[0] = k;
            v_i32_st_tnsr_partial(kc, keys, v_convert_f32_to_i32_b(key, SW_RHNE, 0, 1, 0), 0, 0);
        }
    }
}
