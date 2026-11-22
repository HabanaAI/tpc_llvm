// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// MTP input: out = sigma-packed bf16 [enorm(e) | hnorm(out_norm(x))], the (2H, T) operand of the
// MTP fc (eh_proj) GEMM. x is the main model's final residual (pre final norm), e the embedding
// of the next token. All norms: v / rms(v) * w.
// tensors: 0 x (H,T) f32  1 e (H,T) f32  2 w_out (H)  3 w_h (H)  4 w_e (H) | 5 out (2H,T) bf16
// params: eps. index space: [T]. H multiple of 128.
#include "common.h"

void main(tensor x, tensor e, tensor w_out, tensor w_h, tensor w_e, tensor out, float eps)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(x, 0);
    const float inv_h = 1.0f / (float)H;
    DECL_IDX(IDX)
    for (int t = off[0]; t < end[0]; t++) {
        int5 c = {0, t, 0, 0, 0}, wc = {0, 0, 0, 0, 0};
        float64 sx = 0.0f, se = 0.0f;
        for (int i = 0; i < H; i += 64) {
            c[0] = i;
            const float64 a = v_f32_ld_tnsr_b(c, x), b = v_f32_ld_tnsr_b(c, e);
            sx = v_f32_mac_b(a, a, sx);
            se = v_f32_mac_b(b, b, se);
        }
        float64 rx, re;
        NL_REDUCE_ADD(sx, IDX, rx);
        NL_REDUCE_ADD(se, IDX, re);
        rx = nl_rsqrt(rx * inv_h + eps);
        re = nl_rsqrt(re * inv_h + eps);
        // y = out_norm(x): its mean square for the second norm
        float64 sy = 0.0f;
        for (int i = 0; i < H; i += 64) {
            c[0] = i; wc[0] = i;
            const float64 y = v_f32_ld_tnsr_b(c, x) * rx * v_f32_ld_tnsr_b(wc, w_out);
            sy = v_f32_mac_b(y, y, sy);
        }
        float64 ry;
        NL_REDUCE_ADD(sy, IDX, ry);
        ry = nl_rsqrt(ry * inv_h + eps);
        int5 oc = {0, t, 0, 0, 0};
        for (int i = 0; i < H; i += 128) {
            c[0] = i; wc[0] = i;
            const float64 e0 = v_f32_ld_tnsr_b(c, e) * re * v_f32_ld_tnsr_b(wc, w_e);
            const float64 y0 = v_f32_ld_tnsr_b(c, x) * rx * v_f32_ld_tnsr_b(wc, w_out) * ry * v_f32_ld_tnsr_b(wc, w_h);
            c[0] = i + 64; wc[0] = i + 64;
            const float64 e1 = v_f32_ld_tnsr_b(c, e) * re * v_f32_ld_tnsr_b(wc, w_e);
            const float64 y1 = v_f32_ld_tnsr_b(c, x) * rx * v_f32_ld_tnsr_b(wc, w_out) * ry * v_f32_ld_tnsr_b(wc, w_h);
            oc[0] = i;
            v_bf16_st_tnsr(oc, out, pack_sigma(e0, e1));
            oc[0] = H + i;
            v_bf16_st_tnsr(oc, out, pack_sigma(y0, y1));
        }
    }
}
