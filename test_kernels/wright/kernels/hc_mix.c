// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Hyper-connection input mix: m = mean_s sigmoid(up_s) * x_s * r_s * w_s  (NS = 4 streams), with
// r_s = 1 / rms(x_s) from the hc_combine that produced x (same norm weight w).
// tensors: 0 x (NS*H,T) f32  1 w (NS*H) f32 (1 + gamma)  2 up (NS*H,T) f32 (up GEMM output)
//          3 r (64, NS, T) f32 | 4 m16 (H,T) bf16 (sigma order, for the module's GEMMs)  5 m32 (H,T) f32
// index space: [H/128, T]
#include "common.h"

#define NS 4

void main(tensor x, tensor w, tensor up, tensor rt, tensor m16, tensor m32)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(m32, 0);
    // 2 tokens per step, stream by stream, all loads of a step first
    int t = off[1];
    for (; t + 2 <= end[1]; t += 2)
        for (int blk = off[0]; blk < end[0]; blk++) {
            const int i = blk * 128;
            float64 ma[2], mb[2];
            #pragma unroll
            for (int u = 0; u < 2; u++) { ma[u] = 0.0f; mb[u] = 0.0f; }
            #pragma unroll
            for (int s = 0; s < NS; s++) {
                int5 wc = {s * H + i, 0, 0, 0, 0};
                const float64 wa = v_f32_ld_tnsr_b(wc, w);
                wc[0] += 64;
                const float64 wb = v_f32_ld_tnsr_b(wc, w);
                float64 xa[2], xb[2], ua[2], ub[2], r[2];
                #pragma unroll
                for (int u = 0; u < 2; u++) {
                    int5 c = {s * H + i, t + u, 0, 0, 0}, rc = {0, s, t + u, 0, 0};
                    xa[u] = v_f32_ld_tnsr_b(c, x); ua[u] = v_f32_ld_tnsr_b(c, up);
                    c[0] += 64;
                    xb[u] = v_f32_ld_tnsr_b(c, x); ub[u] = v_f32_ld_tnsr_b(c, up);
                    r[u] = v_f32_ld_tnsr_b(rc, rt) * (1.0f / NS);
                }
                #pragma unroll
                for (int u = 0; u < 2; u++) {
                    ma[u] = v_f32_mac_b(v_sigmoid_f32(ua[u]) * r[u], xa[u] * wa, ma[u]);
                    mb[u] = v_f32_mac_b(v_sigmoid_f32(ub[u]) * r[u], xb[u] * wb, mb[u]);
                }
            }
            #pragma unroll
            for (int u = 0; u < 2; u++) {
                int5 oc = {i, t + u, 0, 0, 0};
                v_bf16_st_tnsr(oc, m16, pack_sigma(ma[u], mb[u]));
                v_f32_st_tnsr(oc, m32, ma[u]);
                oc[0] += 64;
                v_f32_st_tnsr(oc, m32, mb[u]);
            }
        }
    for (; t < end[1]; t++) {
        float64 r[NS];
        #pragma unroll
        for (int s = 0; s < NS; s++) {
            int5 rc = {0, s, t, 0, 0};
            r[s] = v_f32_ld_tnsr_b(rc, rt) * (1.0f / NS);
        }
        for (int blk = off[0]; blk < end[0]; blk++) {
            const int i = blk * 128;
            float64 ma = 0.0f, mb = 0.0f;
            #pragma unroll
            for (int s = 0; s < NS; s++) {
                int5 c = {s * H + i, t, 0, 0, 0}, wc = {s * H + i, 0, 0, 0, 0};
                const float64 xa = v_f32_ld_tnsr_b(c, x) * v_f32_ld_tnsr_b(wc, w);
                const float64 ua = v_f32_ld_tnsr_b(c, up);
                c[0] += 64; wc[0] += 64;
                const float64 xb = v_f32_ld_tnsr_b(c, x) * v_f32_ld_tnsr_b(wc, w);
                const float64 ub = v_f32_ld_tnsr_b(c, up);
                ma = v_f32_mac_b(nl_sigm(ua) * r[s], xa, ma);
                mb = v_f32_mac_b(nl_sigm(ub) * r[s], xb, mb);
            }
            int5 oc = {i, t, 0, 0, 0};
            v_bf16_st_tnsr(oc, m16, pack_sigma(ma, mb));
            v_f32_st_tnsr(oc, m32, ma);
            oc[0] += 64;
            v_f32_st_tnsr(oc, m32, mb);
        }
    }
}
