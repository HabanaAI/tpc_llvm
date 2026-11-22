// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Residual add + RMSNorm.
//  BF16 build:  xo = x (+ y);  h = bf16(rmsnorm(xo) * w)   (h in sigma order)
//    tensors: 0 x (H,T) f32  1 y (H,T) f32  2 w (H) f32 | 3 xo (H,T) f32  4 h (H,T) bf16
//  FP8 build (-DFP8): y is an FP8-GEMM output with a per-token scale ys (y_true = y * ys[t]);
//    the normalised h is emitted as fp8 (tau order) + per-token scale for the next FP8 GEMM,
//    and as bf16 (sigma order) for the small BF16 GEMMs (beta/alpha, LM head).
//    tensors: 0 x  1 y  2 ys (1,T) f32  3 w | 4 xo  5 h8 (H,T) fp8  6 hs (1,T) f32  7 h16 (H,T) bf16
// params: eps, mode (1 = add y, 0 = x only). index space: [T]. H multiple of 256.
#include "common.h"

#ifdef FP8
void main(tensor x, tensor y, tensor ys, tensor w, tensor xo, tensor h8, tensor hs, tensor ho, float eps, int mode)
#else
void main(tensor x, tensor y, tensor w, tensor xo, tensor ho, float eps, int mode)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(x, 0);
    const float inv_h = 1.0f / (float)H;
    DECL_IDX(IDX)
    for (int t = off[0]; t < end[0]; t++) {
#ifdef FP8
        const float64 sy = mode ? TOKEN_SCALE(ys, t, IDX[0]) : (float64)0.0f;
#define YV(c) (v_f32_ld_tnsr_b(c, y) * sy)
#else
#define YV(c) v_f32_ld_tnsr_b(c, y)
#endif
        int5 c = {0, t, 0, 0, 0};
        float64 acc = 0.0f;
        for (int i = 0; i < H; i += 64) {
            c[0] = i;
            float64 v = v_f32_ld_tnsr_b(c, x);
            if (mode) v = v + YV(c);
            v_f32_st_tnsr(c, xo, v);
            acc = v_f32_mac_b(v, v, acc);
        }
        float64 ss;
        NL_REDUCE_ADD(acc, IDX, ss);
        const float64 r = nl_rsqrt(ss * inv_h + eps);
        // recompute from x (+y), not from xo
        int5 wc = {0, 0, 0, 0, 0};
#ifdef FP8
        float64 amax = 0.0f;
#endif
        for (int i = 0; i < H; i += 128) {
            c[0] = i;
            float64 a = v_f32_ld_tnsr_b(c, x);
            if (mode) a = a + YV(c);
            c[0] = i + 64;
            float64 b = v_f32_ld_tnsr_b(c, x);
            if (mode) b = b + YV(c);
            wc[0] = i;      a = a * r * v_f32_ld_tnsr_b(wc, w);
            wc[0] = i + 64; b = b * r * v_f32_ld_tnsr_b(wc, w);
            c[0] = i;
            v_bf16_st_tnsr(c, ho, pack_sigma(a, b));
#ifdef FP8
            amax = v_f32_max_b(amax, v_f32_max_b(v_f32_abs_b(a), v_f32_abs_b(b)));
#endif
        }
#ifdef FP8
        NL_REDUCE_MAX(amax, IDX, amax);
        const float64 sc = v_f32_max_b(amax, 1e-30f) * (1.0f / FP8_MAX);
        const float64 inv = nl_recip(sc);
        for (int i = 0; i < H; i += 256) {
            float64 q[4];
            #pragma unroll
            for (int j = 0; j < 4; j++) {
                c[0] = i + 64 * j;
                float64 v = v_f32_ld_tnsr_b(c, x);
                if (mode) v = v + YV(c);
                wc[0] = i + 64 * j;
                q[j] = v * r * v_f32_ld_tnsr_b(wc, w) * inv;
            }
            c[0] = i;
            v_f8_st_tnsr(c, h8, pack_tau(q[0], q[1], q[2], q[3]));
        }
        STORE_TOKEN_SCALE(hs, t, sc)
#endif
#undef YV
    }
}
