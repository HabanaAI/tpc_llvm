// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// GLM mHC collapse + the sub-layer's RMSNorm, one member per token:
//   c = sum_s pre_s X_s  (pre from coef; mode 1: pre_s = 1/4, the final HC head mean)
//   h = c / rms(c) * w  ->  h16 (bf16, sigma order: the module GEMMs' operand) and h32 (f32)
// tensors: 0 X (4H, T) f32  1 coef (T, 24) f32  2 w (H) f32 | 3 h16 (H, T) bf16  4 h32 (H, T) f32
// params: eps, mode. index space: [1, T]
// Builds mhc_pre8 (Q8 = 1) / mhc_pre8x (Q8 = 2) also write h's per-token FP8 quantization for the FP8 GEMMs
// (s_t = max|h_t| / 240): q (H, T) fp8 in tau order (pack_tau) + qs (1, T); Q8 = 2 also qp (H, 1, R T) fp8 in pair
// order (pack_nat128), one copy per route slot (the routed experts' batch_gemm rows, as ds_xq). H % 256 == 0.
// tensors (Q8): ... | 3 h16  4 h32  5 q  6 qs [7 qp]; params (Q8 = 2): eps, mode, R
#include "common.h"

#define NS 4
#define MAXH 8192

__local__ float64 CV_[MAXH / 64];

#if Q8 == 2
void main(tensor X, tensor coef, tensor w, tensor h16, tensor h32, tensor q, tensor qs, tensor qp, float eps, int mode, int R)
#elif Q8 == 1
void main(tensor X, tensor coef, tensor w, tensor h16, tensor h32, tensor q, tensor qs, float eps, int mode)
#else
void main(tensor X, tensor coef, tensor w, tensor h16, tensor h32, float eps, int mode)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(h32, 0);
    const float inv_h = 1.0f / (float)H;
    scalar_cache_reset();
    DECL_IDX(IDX)
    for (int t = off[1]; t < end[1]; t++) {
        float64 pre[NS];
        #pragma unroll
        for (int s = 0; s < NS; s++)
            pre[s] = mode == 1 ? (float64)0.25f : TOKEN_COEF(coef, t, s);
        float64 acc = 0.0f, acc2 = 0.0f;
        for (int i = 0; i < H; i += 128) {
            float64 xa[NS], xb[NS];
            #pragma unroll
            for (int s = 0; s < NS; s++) {
                int5 c = {s * H + i, t, 0, 0, 0};
                xa[s] = v_f32_ld_tnsr_b(c, X);
                c[0] += 64;
                xb[s] = v_f32_ld_tnsr_b(c, X);
            }
            float64 ca = xa[0] * pre[0], cb = xb[0] * pre[0];
            #pragma unroll
            for (int s = 1; s < NS; s++) { ca = v_f32_mac_b(xa[s], pre[s], ca); cb = v_f32_mac_b(xb[s], pre[s], cb); }
            CV_[i >> 6] = ca;
            CV_[(i >> 6) + 1] = cb;
            acc = v_f32_mac_b(ca, ca, acc);
            acc2 = v_f32_mac_b(cb, cb, acc2);
        }
        float64 ss;
        NL_REDUCE_ADD(acc + acc2, IDX, ss);
        const float64 r = nl_rsqrt(ss * inv_h + eps);
#ifdef Q8
        float64 am = 0.0f;
#endif
        for (int i = 0; i < H; i += 128) {
            int5 wc = {i, 0, 0, 0, 0};
            const float64 wa = v_f32_ld_tnsr_b(wc, w);
            wc[0] += 64;
            const float64 wb = v_f32_ld_tnsr_b(wc, w);
            const float64 a = CV_[i >> 6] * r * wa, b = CV_[(i >> 6) + 1] * r * wb;
            int5 c = {i, t, 0, 0, 0};
            v_f32_st_tnsr(c, h32, a);
            v_bf16_st_tnsr(c, h16, pack_sigma(a, b));
            c[0] += 64;
            v_f32_st_tnsr(c, h32, b);
#ifdef Q8
            CV_[i >> 6] = a;
            CV_[(i >> 6) + 1] = b;
            am = v_f32_max_b(am, v_f32_max_b(v_f32_abs_b(a), v_f32_abs_b(b)));
#endif
        }
#ifdef Q8
        float64 amax;
        NL_REDUCE_MAX(am, IDX, amax);
        const float64 sc = token_scale(amax);
        const float64 inv = token_inv(sc);
        for (int i = 0; i < H; i += 256) {
            int5 c = {i, t, 0, 0, 0};
            v_f8_st_tnsr(c, q, pack_tau(CV_[i >> 6] * inv, CV_[(i >> 6) + 1] * inv, CV_[(i >> 6) + 2] * inv,
                                        CV_[(i >> 6) + 3] * inv));
#if Q8 == 2
            for (int h = 0; h < 2; h++) {
                const minifloat256 pp = pack_nat128(CV_[(i >> 6) + 2 * h] * inv, CV_[(i >> 6) + 2 * h + 1] * inv);
                for (int k = 0; k < R; k++) {
                    int5 pc = {i + 128 * h, 0, t * R + k, 0, 0};
                    v_f8_st_tnsr_partial(pc, qp, pp, 127, 0);
                }
            }
#endif
        }
        STORE_TOKEN_SCALE(qs, t, sc)
#endif
    }
}
