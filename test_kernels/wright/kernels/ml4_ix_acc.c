// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Mistral-Large-4 DSA indexer scores: acc[j, t] = sum_h w[h, t] * relu(scale * S[j, h T + t]) (S: the head-major
// q . k GEMM over [cached | own] index keys; w: weights_proj(h) * n_heads^-0.5).
// tensors: 0 S (Lk, NH * T) f32  1 w (NH, T) f32 | 2 acc (Lk, T) f32
// params: scale, T. index space: [ceil(Lk / 64), T].
#include "common.h"

void main(tensor S, tensor w, tensor acc, float scale, int T)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int NH = get_dim_size(w, 0);
    scalar_cache_reset();
    for (int t = off[1]; t < end[1]; t++)
        for (int b = off[0]; b < end[0]; b++) {
            float64 a0 = 0.0f, a1 = 0.0f;
            for (int h = 0; h < NH; h += 2) {
                int5 wc = {h, t, 0, 0, 0};
                const float w0 = s_f32_ld_g(gen_addr(wc, w));
                wc[0] = h + 1;
                const float w1 = s_f32_ld_g(gen_addr(wc, w));
                int5 sc = {64 * b, h * T + t, 0, 0, 0};
                a0 = v_f32_mac_b(v_f32_max_b(v_f32_ld_tnsr_b(sc, S) * scale, 0.0f), w0, a0);
                sc[1] = (h + 1) * T + t;
                a1 = v_f32_mac_b(v_f32_max_b(v_f32_ld_tnsr_b(sc, S) * scale, 0.0f), w1, a1);
            }
            int5 oc = {64 * b, t, 0, 0, 0};
            v_f32_st_tnsr(oc, acc, a0 + a1);
        }
}
