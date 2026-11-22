// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DFlash drafter: residual add + RMSNorm in natural channel order.
//   xo = x (+ y * ys[t]);  hn = rmsnorm(xo) * w (f32, natural);  h16 = bf16(hn) in sigma order
//   (the operand of the bf16 GEMMs: LM head, selector projection)
// tensors: 0 x (H,T) f32  1 y (H,T) f32  2 ys (1,T) f32  3 w (H) f32
//        | 4 xo (H,T) f32  5 hn (H,T) f32  6 h16 (H,T) bf16
// params: eps, mode (1 = add y * ys, 0 = x only). index space: [T]. H multiple of 128.
#include "common.h"
void main(tensor x, tensor y, tensor ys, tensor w, tensor xo, tensor hn, tensor ho, float eps, int mode)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(x, 0);
    const float inv_h = 1.0f / (float)H;
    DECL_IDX(IDX)
    for (int t = off[0]; t < end[0]; t++) {
        const float64 sy = mode ? TOKEN_SCALE(ys, t, IDX[0]) : (float64)0.0f;
        int5 c = {0, t, 0, 0, 0};
        float64 acc = 0.0f;
        for (int i = 0; i < H; i += 64) {
            c[0] = i;
            float64 v = v_f32_ld_tnsr_b(c, x);
            if (mode) v = v_f32_mac_b(v_f32_ld_tnsr_b(c, y), sy, v);
            v_f32_st_tnsr(c, xo, v);
            acc = v_f32_mac_b(v, v, acc);
        }
        float64 ss;
        NL_REDUCE_ADD(acc, IDX, ss);
        const float64 r = nl_rsqrt(ss * inv_h + eps);
        // recompute from x (+ y), not from xo
        int5 wc = {0, 0, 0, 0, 0};
        for (int i = 0; i < H; i += 128) {
            c[0] = i;
            float64 a = v_f32_ld_tnsr_b(c, x);
            if (mode) a = v_f32_mac_b(v_f32_ld_tnsr_b(c, y), sy, a);
            c[0] = i + 64;
            float64 b = v_f32_ld_tnsr_b(c, x);
            if (mode) b = v_f32_mac_b(v_f32_ld_tnsr_b(c, y), sy, b);
            wc[0] = i;      a = a * r * v_f32_ld_tnsr_b(wc, w);
            wc[0] = i + 64; b = b * r * v_f32_ld_tnsr_b(wc, w);
            c[0] = i;
            v_f32_st_tnsr(c, hn, a);
            v_bf16_st_tnsr(c, ho, pack_sigma(a, b));
            c[0] = i + 64;
            v_f32_st_tnsr(c, hn, b);
        }
    }
}
