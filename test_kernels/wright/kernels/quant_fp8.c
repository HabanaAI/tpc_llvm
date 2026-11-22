// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Per-token dynamic FP8 quantization of an f32 activation (natural order) for an FP8 GEMM:
//   s = max|x_t| / 240;  q = fp8(x / s) in tau order.
// tensors: 0 x (K,T) f32 | 1 q (K,T) fp8  2 s (1,T) f32.  index space: [T]. K multiple of 256.
#include "common.h"

void main(tensor x, tensor q, tensor s)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int K = get_dim_size(x, 0);
    DECL_IDX(IDX)
    for (int t = off[0]; t < end[0]; t++) {
        int5 c = {0, t, 0, 0, 0};
        float64 amax = 0.0f;
        for (int i = 0; i < K; i += 64) {
            c[0] = i;
            amax = v_f32_max_b(amax, v_f32_abs_b(v_f32_ld_tnsr_b(c, x)));
        }
        NL_REDUCE_MAX(amax, IDX, amax);
        const float64 sc = v_f32_max_b(amax, 1e-30f) * (1.0f / FP8_MAX);
        const float64 inv = nl_recip(sc);
        for (int i = 0; i < K; i += 256) {
            c[0] = i;       const float64 a = v_f32_ld_tnsr_b(c, x) * inv;
            c[0] = i + 64;  const float64 b = v_f32_ld_tnsr_b(c, x) * inv;
            c[0] = i + 128; const float64 d = v_f32_ld_tnsr_b(c, x) * inv;
            c[0] = i + 192; const float64 e = v_f32_ld_tnsr_b(c, x) * inv;
            c[0] = i;
            v_f8_st_tnsr(c, q, pack_tau(a, b, d, e));
        }
        STORE_TOKEN_SCALE(s, t, sc)
    }
}
