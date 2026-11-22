// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// mHC site projection on the TPC: mixT[t, j] = fn_j . X_t.
// Member 0 also writes rt[:, t] = 1 / rms(X_t) (all lanes; eps as mhc_post), the site's r.
// tensors: 0 fn (K, 32) f32 (row j contiguous)  1 X (K, T) f32 | 2 mixT (T, 32) f32  3 rt (64, T) f32
// params: eps.  index space: T <= 2: [32] (one row per member); else [8, ceil(T/4)] (4 rows x 4 tokens per member).
#include "common.h"

void main(tensor fn, tensor X, tensor mixT, tensor rt, float eps)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int K = get_dim_size(X, 0), T = get_dim_size(X, 1);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
#ifndef ROWS_ONLY
    if (T > 2) {
        for (int rt4 = off[0]; rt4 < end[0]; rt4++)
            for (int tt = off[1]; tt < end[1]; tt++) {
                const int t0 = 4 * tt, nt = T - t0 < 4 ? T - t0 : 4;
                float64 acc[4][4], q[4];
                #pragma unroll
                for (int i = 0; i < 4; i++) { q[i] = 0.0f; for (int u = 0; u < 4; u++) acc[i][u] = 0.0f; }
                #pragma unroll 4
                for (int k = 0; k < K; k += 64) {
                    float64 f[4];
                    #pragma unroll
                    for (int i = 0; i < 4; i++) { int5 wc = {k, 4 * rt4 + i, 0, 0, 0}; f[i] = v_f32_ld_tnsr_b(wc, fn); }
                    #pragma unroll
                    for (int u = 0; u < 4; u++) {
                        int5 xc = {k, t0 + u, 0, 0, 0};             // (tokens past T read zeros)
                        const float64 xv = v_f32_ld_tnsr_b(xc, X);
                        #pragma unroll
                        for (int i = 0; i < 4; i++) acc[i][u] = v_f32_mac_b(f[i], xv, acc[i][u]);
                        if (rt4 == 0) q[u] = v_f32_mac_b(xv, xv, q[u]);
                    }
                }
                #pragma unroll
                for (int i = 0; i < 4; i++) {
                    float64 out = 0.0f;
                    #pragma unroll
                    for (int u = 0; u < 4; u++) {
                        float64 sum;
                        NL_REDUCE_ADD(acc[i][u], IDX, sum);
                        out = v_f32_sel_eq_f32_b(lanef, (float)u, sum, out);
                    }
                    int5 oc = {t0, 4 * rt4 + i, 0, 0, 0};
                    v_f32_st_tnsr_partial(oc, mixT, out, nt - 1, 0);
                }
                if (rt4 == 0)
                    for (int u = 0; u < nt; u++) {
                        float64 ss;
                        NL_REDUCE_ADD(q[u], IDX, ss);
                        int5 rc = {0, t0 + u, 0, 0, 0};
                        v_f32_st_tnsr(rc, rt, nl_rsqrt(ss * (1.0f / (float)K) + eps));
                    }
            }
        return;
    }
#endif
    for (int j = off[0]; j < end[0]; j++)
        for (int tb = 0; tb < T; tb += 64) {               // results of 64 tokens per output vector
            float64 out = 0.0f;
            for (int t = tb; t < tb + 64 && t < T; t++) {
                float64 a0 = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
                int5 wc = {0, j, 0, 0, 0}, xc = {0, t, 0, 0, 0};
                if (j == 0) {                               // 1 / rms of the token's streams
                    float64 q0 = 0.0f, q1 = 0.0f;
                    for (int k = 0; k < K; k += 128) {
                        xc[0] = k;      const float64 u = v_f32_ld_tnsr_b(xc, X); q0 = v_f32_mac_b(u, u, q0);
                        xc[0] = k + 64; const float64 v = v_f32_ld_tnsr_b(xc, X); q1 = v_f32_mac_b(v, v, q1);
                    }
                    float64 ss;
                    NL_REDUCE_ADD(q0 + q1, IDX, ss);
                    int5 rc = {0, t, 0, 0, 0};
                    v_f32_st_tnsr(rc, rt, nl_rsqrt(ss * (1.0f / (float)K) + eps));
                }
                #pragma unroll 2
                for (int k = 0; k < K; k += 256) {
                    wc[0] = k;       xc[0] = k;       a0 = v_f32_mac_b(v_f32_ld_tnsr_b(wc, fn), v_f32_ld_tnsr_b(xc, X), a0);
                    wc[0] = k + 64;  xc[0] = k + 64;  a1 = v_f32_mac_b(v_f32_ld_tnsr_b(wc, fn), v_f32_ld_tnsr_b(xc, X), a1);
                    wc[0] = k + 128; xc[0] = k + 128; a2 = v_f32_mac_b(v_f32_ld_tnsr_b(wc, fn), v_f32_ld_tnsr_b(xc, X), a2);
                    wc[0] = k + 192; xc[0] = k + 192; a3 = v_f32_mac_b(v_f32_ld_tnsr_b(wc, fn), v_f32_ld_tnsr_b(xc, X), a3);
                }
                float64 sum;
                NL_REDUCE_ADD((a0 + a1) + (a2 + a3), IDX, sum);
                out = v_f32_sel_eq_f32_b(lanef, (float)(t - tb), sum, out);
            }
            int5 oc = {tb, j, 0, 0, 0};
            v_f32_st_tnsr(oc, mixT, out);
        }
}
