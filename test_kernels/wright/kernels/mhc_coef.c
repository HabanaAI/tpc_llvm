// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// GLM mHC coefficients (Manifold-constrained hyper-connections), vectorized across 64 tokens:
//   mix = (X . fn^T) * r   (r = 1/rms of the token's flattened 4 streams: flat is RMS-normalized first)
//   pre_s  = sigmoid(mix_s scale0 + base_s) + eps              s = 0..3
//   post_s = 2 sigmoid(mix_{4+s} scale1 + base_{4+s})
//   comb   = Sinkhorn(softmax_rows(mix_{8..23} scale2 + base_{8..23}) + eps), `iters` alternations
//            (column normalization first, then iters-1 x (rows, columns)), eps in every denominator
// tensors: 0 mixT (T, 32) f32 (token-major GEMM output, rows 0..23 used)  1 rt (64, T) f32 (r per
//          token, all lanes)  2 base (24) f32  3 scale (3) f32 | 4 coef (T, 24) f32 [pre | post | comb]
// params: iters, eps. index space: [ceil(T / 64)]
#include "common.h"

void main(tensor mixT, tensor rt, tensor base, tensor scl, tensor coef, int iters, float eps)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int T = get_dim_size(coef, 0);
    scalar_cache_reset();
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    float b[24], sc[3];
    for (int m = 0; m < 24; m++) { int5 c = {m, 0, 0, 0, 0}; b[m] = s_f32_ld_g(gen_addr(c, base)); }
    for (int m = 0; m < 3; m++) { int5 c = {m, 0, 0, 0, 0}; sc[m] = s_f32_ld_g(gen_addr(c, scl)); }
    for (int blk = off[0]; blk < end[0]; blk++) {
        const int t0 = 64 * blk;
        // r of the 64 tokens: lane k <- lane k of token t0 + k's replicated row
        float64 r = 0.0f;
        for (int k = 0; k < 64 && t0 + k < T; k++) {
            int5 c = {0, t0 + k, 0, 0, 0};
            r = v_f32_sel_eq_f32_b(lanef, (float)k, v_f32_ld_tnsr_b(c, rt), r);
        }
        float64 v[24];
        #pragma unroll
        for (int m = 0; m < 24; m++) {
            int5 c = {t0, m, 0, 0, 0};
            v[m] = v_f32_ld_tnsr_b(c, mixT) * r;
        }
        #pragma unroll
        for (int s = 0; s < 4; s++) {
            v[s] = nl_sigm(v_f32_mac_b(v[s], sc[0], (float64)b[s])) + eps;
            v[4 + s] = 2.0f * nl_sigm(v_f32_mac_b(v[4 + s], sc[1], (float64)b[4 + s]));
        }
        // comb: softmax over each row i (entries 8 + 4 i + j)
        float64 cm[16];
        #pragma unroll
        for (int i = 0; i < 4; i++) {
            float64 l[4];
            #pragma unroll
            for (int j = 0; j < 4; j++) l[j] = v_f32_mac_b(v[8 + 4 * i + j], sc[2], (float64)b[8 + 4 * i + j]);
            const float64 mx = v_f32_max_b(v_f32_max_b(l[0], l[1]), v_f32_max_b(l[2], l[3]));
            float64 e[4], sum = 0.0f;
            #pragma unroll
            for (int j = 0; j < 4; j++) { e[j] = nl_exp(l[j] - mx); sum += e[j]; }
            const float64 inv = nl_recip(sum);
            #pragma unroll
            for (int j = 0; j < 4; j++) cm[4 * i + j] = e[j] * inv + eps;
        }
#define COLS()                                                                                   \
        _Pragma("unroll") for (int j = 0; j < 4; j++) {                                          \
            const float64 inv = nl_recip(cm[j] + cm[4 + j] + cm[8 + j] + cm[12 + j] + eps);      \
            _Pragma("unroll") for (int i = 0; i < 4; i++) cm[4 * i + j] *= inv;                   \
        }
#define ROWS()                                                                                   \
        _Pragma("unroll") for (int i = 0; i < 4; i++) {                                          \
            const float64 inv = nl_recip(cm[4 * i] + cm[4 * i + 1] + cm[4 * i + 2] + cm[4 * i + 3] + eps); \
            _Pragma("unroll") for (int j = 0; j < 4; j++) cm[4 * i + j] *= inv;                   \
        }
        COLS()
        for (int it = 1; it < iters; it++) {
            ROWS()
            COLS()
        }
        #pragma unroll
        for (int m = 0; m < 8; m++) { int5 c = {t0, m, 0, 0, 0}; v_f32_st_tnsr(c, coef, v[m]); }
        #pragma unroll
        for (int m = 0; m < 16; m++) { int5 c = {t0, 8 + m, 0, 0, 0}; v_f32_st_tnsr(c, coef, cm[m]); }
    }
}
