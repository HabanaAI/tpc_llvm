// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// mHC site projection of one token in K chunks, for mhc_coef1: partial dots mixp[j, c] = fn_j . X over chunk c
// of the K = 4H positions, four rows per member (each X vector loaded once for the four), and the chunk's sum of
// squares of X, ssp[c] (row group 0).
// tensors: 0 fn (K, 32) f32 (row j contiguous)  1 X (K, 1) f32 | 2 mixp (32, C) f32  3 ssp (C) f32
// index space: [8, C]; K a multiple of 256 C.
#include "common.h"

void main(tensor fn, tensor X, tensor mixp, tensor ssp)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int K = get_dim_size(X, 0), C = get_dim_size(mixp, 1), chunk = K / C;
    DECL_IDX(IDX)
    for (int rg = off[0]; rg < end[0]; rg++)
        for (int c = off[1]; c < end[1]; c++) {
            float64 a[4][2], q0 = 0.0f, q1 = 0.0f;
            #pragma unroll
            for (int i = 0; i < 4; i++) { a[i][0] = 0.0f; a[i][1] = 0.0f; }
            for (int k = c * chunk; k < (c + 1) * chunk; k += 128) {
                int5 xc = {k, 0, 0, 0, 0};
                const float64 x0 = v_f32_ld_tnsr_b(xc, X);
                xc[0] = k + 64;
                const float64 x1 = v_f32_ld_tnsr_b(xc, X);
                #pragma unroll
                for (int i = 0; i < 4; i++) {
                    int5 wc = {k, 4 * rg + i, 0, 0, 0};
                    a[i][0] = v_f32_mac_b(v_f32_ld_tnsr_b(wc, fn), x0, a[i][0]);
                    wc[0] = k + 64;
                    a[i][1] = v_f32_mac_b(v_f32_ld_tnsr_b(wc, fn), x1, a[i][1]);
                }
                if (rg == 0) {
                    q0 = v_f32_mac_b(x0, x0, q0);
                    q1 = v_f32_mac_b(x1, x1, q1);
                }
            }
            #pragma unroll
            for (int i = 0; i < 4; i++) {
                float64 sum;
                NL_REDUCE_ADD(a[i][0] + a[i][1], IDX, sum);
                int5 oc = {4 * rg + i, c, 0, 0, 0};
                v_f32_st_tnsr_partial(oc, mixp, sum, 0, 0);
            }
            if (rg == 0) {
                float64 ss;
                NL_REDUCE_ADD(q0 + q1, IDX, ss);
                int5 sc = {c, 0, 0, 0, 0};
                v_f32_st_tnsr_partial(sc, ssp, ss, 0, 0);
            }
        }
}
