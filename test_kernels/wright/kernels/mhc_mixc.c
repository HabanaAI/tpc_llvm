// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// mhc_mix for a few tokens with K in C chunks, so that more members share the reads: member (g, c, tt) sums rows
// 4 g .. 4 g + 3 of the projection (rows 0..23, the ones the coefficients use) for tokens 4 tt .. 4 tt + 3 over
// chunk c of K, and the members of g = 0 also each token's sum of squares over the chunk. mhc_coefp adds the
// chunks up. A member reads 4 rows and 4 tokens of K / C elements, where mhc_mix reads all K. The loops stay
// rolled.
// tensors: 0 fn (K, 32) f32 (row j contiguous)  1 X (K, T) f32 | 2 mixp (T, 24, C) f32  3 ssp (T, C) f32
// index space: [6, C, ceil(T / 4)]; K / C a multiple of 64.
#include "common.h"

void main(tensor fn, tensor X, tensor mixp, tensor ssp)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int K = get_dim_size(X, 0), T = get_dim_size(X, 1), C = get_dim_size(mixp, 2);
    const int Kc = K / C;
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    for (int g = off[0]; g < end[0]; g++)
        for (int c = off[1]; c < end[1]; c++)
            for (int tt = off[2]; tt < end[2]; tt++) {
                const int t0 = 4 * tt, nt = T - t0 < 4 ? T - t0 : 4;
                float64 acc[4][4], q[4];
                #pragma unroll
                for (int i = 0; i < 4; i++) { q[i] = 0.0f; for (int u = 0; u < 4; u++) acc[i][u] = 0.0f; }
                #pragma unroll 1
                for (int k = c * Kc; k < (c + 1) * Kc; k += 64) {
                    float64 f[4];
                    #pragma unroll
                    for (int i = 0; i < 4; i++) { int5 wc = {k, 4 * g + i, 0, 0, 0}; f[i] = v_f32_ld_tnsr_b(wc, fn); }
                    #pragma unroll
                    for (int u = 0; u < 4; u++) {
                        int5 xc = {k, t0 + u, 0, 0, 0};             // (tokens past T read zeros)
                        const float64 xv = v_f32_ld_tnsr_b(xc, X);
                        #pragma unroll
                        for (int i = 0; i < 4; i++) acc[i][u] = v_f32_mac_b(f[i], xv, acc[i][u]);
                        if (g == 0) q[u] = v_f32_mac_b(xv, xv, q[u]);
                    }
                }
                #pragma unroll 1
                for (int i = 0; i < 4; i++) {
                    float64 out = 0.0f;
                    #pragma unroll 1
                    for (int u = 0; u < 4; u++) {
                        float64 sum;
                        NL_REDUCE_ADD(acc[i][u], IDX, sum);
                        out = v_f32_sel_eq_f32_b(lanef, (float)u, sum, out);
                    }
                    int5 oc = {t0, 4 * g + i, c, 0, 0};
                    v_f32_st_tnsr_partial(oc, mixp, out, nt - 1, 0);
                }
                if (g == 0) {
                    float64 out = 0.0f;
                    #pragma unroll
                    for (int u = 0; u < 4; u++) {
                        float64 ss;
                        NL_REDUCE_ADD(q[u], IDX, ss);
                        out = v_f32_sel_eq_f32_b(lanef, (float)u, ss, out);
                    }
                    int5 sc = {t0, c, 0, 0, 0};
                    v_f32_st_tnsr_partial(sc, ssp, out, nt - 1, 0);
                }
            }
}
