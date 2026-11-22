// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// hc_combine (see hc_combine.c), token-major for prefill: one member per token handles all NS streams,
// so y (the module output, summed over NR rank slices) is read once per token instead of once per
// stream, the inject gates are computed once, and a step issues all its loads before its stores.
// tensors / params as hc_combine. index space: [1, T].
#include "common.h"

#define NS 4
#define INJ 320
#define MAXH 2560

__local__ float64 XV_[NS * MAXH / 64];   // the token's updated residual (pass 2 reads it back)

void main(tensor x, tensor y, tensor lo, tensor w, tensor xo, tensor xn, tensor rt, float eps, int mode)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(x, 0) / NS, NR = get_dim_size(y, 2);
    const float inv_h = 1.0f / (float)H;
    const float fc = mode == 1 ? 1.0f : 0.0f;
    const float fx = mode == 2 ? 0.0f : 1.0f;
    const float fy = mode == 0 ? 0.0f : 1.0f;
    DECL_IDX(IDX)
    for (int t = off[1]; t < end[1]; t++) {
        int5 lc = {INJ, t, 0, 0, 0};
        const float64 lv = BCAST_DG(v_f32_ld_tnsr_b(lc, lo), 0) * (1.0f / NS);
        float64 a[NS], acc[NS];
        #pragma unroll
        for (int s = 0; s < NS; s++) {
            const float64 g = 2.0f * nl_sigm(v_f32_shuffle_b(lv, IDX[s], 0, lv));
            a[s] = v_f32_mac_b(g, fc, (float64)(fy - fc));
            acc[s] = 0.0f;
        }
        for (int i = 0; i < H; i += 128) {
            int5 yc = {i, t, 0, 0, 0};
            float64 ya = v_f32_ld_tnsr_b(yc, y);
            yc[0] += 64;
            float64 yb = v_f32_ld_tnsr_b(yc, y);
            for (int r = 1; r < NR; r++) {
                yc[2] = r;
                yb += v_f32_ld_tnsr_b(yc, y);
                yc[0] = i;
                ya += v_f32_ld_tnsr_b(yc, y);
                yc[0] = i + 64;
            }
            float64 xa[NS], xb[NS];
            #pragma unroll
            for (int s = 0; s < NS; s++) {
                int5 c = {s * H + i, t, 0, 0, 0};
                xa[s] = v_f32_ld_tnsr_b(c, x);
                c[0] += 64;
                xb[s] = v_f32_ld_tnsr_b(c, x);
            }
            #pragma unroll
            for (int s = 0; s < NS; s++) {
                const float64 va = v_f32_mac_b(xa[s], fx, ya * a[s]);
                const float64 vb = v_f32_mac_b(xb[s], fx, yb * a[s]);
                int5 c = {s * H + i, t, 0, 0, 0};
                v_f32_st_tnsr(c, xo, va);
                c[0] += 64;
                v_f32_st_tnsr(c, xo, vb);
                XV_[(s * H + i) >> 6] = va;
                XV_[((s * H + i) >> 6) + 1] = vb;
                acc[s] = v_f32_mac_b(va, va, acc[s]);
                acc[s] = v_f32_mac_b(vb, vb, acc[s]);
            }
        }
        #pragma unroll
        for (int s = 0; s < NS; s++) {
            float64 ss;
            NL_REDUCE_ADD(acc[s], IDX, ss);
            const float64 r = nl_rsqrt(ss * inv_h + eps);
            int5 rc = {0, s, t, 0, 0};
            v_f32_st_tnsr(rc, rt, r);
            for (int i = 0; i < H; i += 256) {
                float64 wv[4];
                #pragma unroll
                for (int u = 0; u < 4; u++) {
                    int5 wc = {s * H + i + 64 * u, 0, 0, 0, 0};
                    wv[u] = v_f32_ld_tnsr_b(wc, w);
                }
                const int b = (s * H + i) >> 6;
                int5 c = {s * H + i, t, 0, 0, 0};
                v_bf16_st_tnsr(c, xn, pack_sigma(XV_[b] * r * wv[0], XV_[b + 1] * r * wv[1]));
                c[0] += 128;
                v_bf16_st_tnsr(c, xn, pack_sigma(XV_[b + 2] * r * wv[2], XV_[b + 3] * r * wv[3]));
            }
        }
    }
}
