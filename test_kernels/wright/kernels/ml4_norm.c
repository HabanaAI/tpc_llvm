// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Mistral-Large-4: RMSNorm or LayerNorm (with bias) over the first n rows of each column, f32 -> f32 (natural order,
// the lq GEMVs' input):  mode 0: y = x * rsqrt(mean x^2 + eps) * w;  mode 1: y = (x - mean) * rsqrt(var + eps) * w + b
// tensors: 0 x (>= n, T) f32  1 w (n) f32  2 b (n) f32 (mode 1; any tensor otherwise) | 3 y (n, T) f32
// params: eps, mode. index space: [T]. n multiple of 64.
#include "common.h"

void main(tensor x, tensor w, tensor b, tensor y, float eps, int mode)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int n = get_dim_size(y, 0);
    const float inv_n = 1.0f / (float)n;
    DECL_IDX(IDX)
    for (int t = off[0]; t < end[0]; t++) {
        float64 mean = 0.0f;
        if (mode == 1) {
            float64 s0 = 0.0f, s1 = 0.0f;
            for (int i = 0; i < n; i += 128) {
                int5 c = {i, t, 0, 0, 0};
                s0 += v_f32_ld_tnsr_b(c, x);
                c[0] = i + 64;
                s1 += i + 64 < n ? v_f32_ld_tnsr_b(c, x) : 0.0f;
            }
            float64 tot;
            NL_REDUCE_ADD(s0 + s1, IDX, tot);
            mean = tot * inv_n;
        }
        float64 q0 = 0.0f, q1 = 0.0f;
        for (int i = 0; i < n; i += 128) {
            int5 c = {i, t, 0, 0, 0};
            const float64 a = v_f32_ld_tnsr_b(c, x) - mean;
            q0 = v_f32_mac_b(a, a, q0);
            if (i + 64 < n) {
                c[0] = i + 64;
                const float64 d = v_f32_ld_tnsr_b(c, x) - mean;
                q1 = v_f32_mac_b(d, d, q1);
            }
        }
        float64 qt;
        NL_REDUCE_ADD(q0 + q1, IDX, qt);
        const float64 r = nl_rsqrt(qt * inv_n + eps);
        for (int i = 0; i < n; i += 64) {
            int5 c = {i, t, 0, 0, 0}, wc = {i, 0, 0, 0, 0};
            float64 v = (v_f32_ld_tnsr_b(c, x) - mean) * r * v_f32_ld_tnsr_b(wc, w);
            if (mode == 1)
                v += v_f32_ld_tnsr_b(wc, b);
            v_f32_st_tnsr(c, y, v);
        }
    }
}
