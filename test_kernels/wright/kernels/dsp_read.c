// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DSpine code bridge, read side: the residual after a drafter layer, its RMS and the read operand.
//   raw = x + y * ys[t];  rms = sqrt(max(mean(raw^2), FLT_MIN));  nb = bf16(raw / (rms + eps)) in sigma
//   order (the operand of the read GEMM);  ws[t] = rms, 0 on the anchor columns (t % block == 0), which
//   get no write.
// tensors: 0 x (H,T) f32  1 y (H,T) f32  2 ys (1,T) f32 | 3 raw (H,T) f32  4 nb (H,T) bf16  5 ws (1,T) f32
// params: eps, block. index space: [T]. H multiple of 128.
#include "common.h"
void main(tensor x, tensor y, tensor ys, tensor raw, tensor nb, tensor ws, float eps, int block)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(x, 0);
    const float inv_h = 1.0f / (float)H;
    DECL_IDX(IDX)
    for (int t = off[0]; t < end[0]; t++) {
        const float64 sy = TOKEN_SCALE(ys, t, IDX[0]);
        int5 c = {0, t, 0, 0, 0};
        float64 acc = 0.0f;
        for (int i = 0; i < H; i += 64) {
            c[0] = i;
            const float64 v = v_f32_mac_b(v_f32_ld_tnsr_b(c, y), sy, v_f32_ld_tnsr_b(c, x));
            v_f32_st_tnsr(c, raw, v);
            acc = v_f32_mac_b(v, v, acc);
        }
        float64 ss;
        NL_REDUCE_ADD(acc, IDX, ss);
        const float64 ms = v_f32_max_b(ss * inv_h, 1.1754943508222875e-38f);
        const float64 rms = ms * nl_rsqrt(ms);
        const float64 inv = nl_recip(rms + eps);
        // recompute from x + y, not from raw
        for (int i = 0; i < H; i += 128) {
            c[0] = i;
            const float64 a = v_f32_mac_b(v_f32_ld_tnsr_b(c, y), sy, v_f32_ld_tnsr_b(c, x));
            c[0] = i + 64;
            const float64 b = v_f32_mac_b(v_f32_ld_tnsr_b(c, y), sy, v_f32_ld_tnsr_b(c, x));
            c[0] = i;
            v_bf16_st_tnsr(c, nb, pack_sigma(a * inv, b * inv));
        }
        const float64 w = (t % block == 0) ? (float64)0.0f : rms;
        STORE_TOKEN_SCALE(ws, t, w);
    }
}
