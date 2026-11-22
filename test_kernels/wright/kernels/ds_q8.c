// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Per-token FP8 quantization (tau order, pack_tau) of an activation for an FP8 GEMM, with optional producers:
//   mode & 1: v = x * sin[t]              (chains the previous FP8 GEMM's per-token scale)
//   mode & 2: v = x * rsqrt(mean x^2 + eps) * nw    (RMSNorm, e.g. q_norm)
//   mode & 4: v = silu(min(g, lim)) * clamp(u, +-lim), g = x[k] (* sin), u = x[K + k] (* sin)   (x: 2K rows)
// then s_t = max|v_t| / 240, q = fp8(v / s_t). Positions past K (Kp = K rounded up to 256) are zero.
// Each member writes 256 positions of one token (and reads the whole token for the max / norm).
// tensors: 0 x (K or 2K, T) f32  1 sin (1, T) f32  2 nw (K) f32 | 3 q (Kp, T) fp8  4 s (1, T) f32
//   mode & 8: one member per token covers all of K
// params: mode, eps, lim.  index space: [Kp/256, T] (mode & 8: [1, T])
// Builds with MODE fixed (ds_q8z_p2: 0, ds_q8n_p2: 2, ds_q8s_p2: 5) keep only bit 8 of the mode parameter.
#include "common.h"

static inline float64 val(tensor x, tensor nw, int k, int t, int K, int mode, float64 sc, float64 r, float lim,
                          float64 lanef)
{
    int5 c = {k, t, 0, 0, 0};
    float64 v;
    if (mode & 4) {
        const float64 g = v_f32_ld_tnsr_b(c, x) * sc;
        c[0] = k + K;
        const float64 u = v_f32_ld_tnsr_b(c, x) * sc;
        v = nl_silus(v_f32_min_b(g, lim)) * v_f32_max_b(v_f32_min_b(u, lim), -lim);
    } else {
        v = v_f32_ld_tnsr_b(c, x) * sc;
        if (mode & 2) {
            int5 wc = {k, 0, 0, 0, 0};
            v = v * r * v_f32_ld_tnsr_b(wc, nw);
        }
    }
    return v_f32_sel_less_f32_b(lanef + (float)k, (float)K, v, 0.0f);
}

void main(tensor x, tensor sin, tensor nw, tensor q, tensor s, int mode, float eps, float lim)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
#ifdef MODE
    mode = MODE | (mode & 8);  // a build for one producer
#endif
    const int K = (mode & 4) ? get_dim_size(x, 0) / 2 : get_dim_size(x, 0);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    for (int t = off[1]; t < end[1]; t++) {
        const float64 sc = (mode & 1) ? TOKEN_SCALE(sin, t, IDX[0]) : (float64)1.0f;
        float64 r = 1.0f;
        if (mode & 2) {
            float64 s4[4] = {0.0f, 0.0f, 0.0f, 0.0f};  // four accumulators
            for (int k = 0; k < K; k += 256) {
                #pragma unroll
                for (int u = 0; u < 4; u++) {
                    int5 c = {k + 64 * u, t, 0, 0, 0};
                    const float64 v = v_f32_sel_less_f32_b(lanef + (float)(k + 64 * u), (float)K,
                                                           v_f32_ld_tnsr_b(c, x) * sc, 0.0f);
                    s4[u] = v_f32_mac_b(v, v, s4[u]);
                }
            }
            float64 ss;
            NL_REDUCE_ADD((s4[0] + s4[1]) + (s4[2] + s4[3]), IDX, ss);
            r = nl_rsqrt(ss * (1.0f / (float)K) + eps);
        }
        float64 amax = 0.0f;
        if (mode & 4) {
            for (int k = 0; k < K; k += 64)
                amax = v_f32_max_b(amax, v_f32_abs_b(val(x, nw, k, t, K, mode, sc, r, lim, lanef)));
        } else {
            float64 a4[4] = {0.0f, 0.0f, 0.0f, 0.0f};
            for (int k = 0; k < K; k += 256) {
                #pragma unroll
                for (int u = 0; u < 4; u++)
                    a4[u] = v_f32_max_b(a4[u], v_f32_abs_b(val(x, nw, k + 64 * u, t, K, mode, sc, r, lim, lanef)));
            }
            amax = v_f32_max_b(v_f32_max_b(a4[0], a4[1]), v_f32_max_b(a4[2], a4[3]));
        }
        NL_REDUCE_MAX(amax, IDX, amax);
        const float64 s_ = token_scale(amax);
        const float64 inv = token_inv(s_);
        const int b0 = (mode & 8) ? 0 : off[0], b1 = (mode & 8) ? get_dim_size(q, 0) / 256 : end[0];
        for (int b = b0; b < b1; b++) {
            const int k = 256 * b;
            int5 qc = {k, t, 0, 0, 0};
            v_f8_st_tnsr(qc, q, pack_tau(val(x, nw, k, t, K, mode, sc, r, lim, lanef) * inv,
                                         val(x, nw, k + 64, t, K, mode, sc, r, lim, lanef) * inv,
                                         val(x, nw, k + 128, t, K, mode, sc, r, lim, lanef) * inv,
                                         val(x, nw, k + 192, t, K, mode, sc, r, lim, lanef) * inv));
            if (b == 0)
                STORE_TOKEN_SCALE(s, t, s_)
        }
    }
}
