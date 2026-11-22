// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 Engram residual: each stream s of a token takes the shared value, gated by how well the stream
// matches its key:
//   r = rsqrt(mean h_s^2 + eps) rsqrt(mean key_s^2 + eps),  dot = sum(h_s w_s key_s) r / sqrt(H)
//   gate = sigmoid(sign(dot) sqrt(max(|dot|, 1e-6))),  out_s = h_s + gate value
// tensors: 0 X (4H, T) f32 (stream s at rows s H)  1 kv (5H, T) f32 [key_0 .. key_3 | value]  2 w (4H) f32
//          | 3 out (4H, T) f32
// GATHERED builds (eg_gate_g.c): kv is (5H / R, T, R), the ranks' slices of kv's rows as an all-gather leaves them
// (row j of token t at (j % C, t, j / C), C = 5H / R a multiple of 64), and one member takes one stream of one
// token: index space [4, T].
// params: eps. index space: [T]
#include "common.h"

#ifdef GATHERED
#define KV(j, t) {(j) % C, (t), (j) / C, 0, 0}
#else
#define KV(j, t) {(j), (t), 0, 0, 0}
#endif

void main(tensor X, tensor kv, tensor w, tensor out, float eps)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(X, 0) / 4;
    const float rh = 1.0f / (float)H;
#ifdef GATHERED
    const int C = get_dim_size(kv, 0);
    const int t0 = off[1], t1 = end[1], s0 = off[0], s1 = end[0];
#else
    const int t0 = off[0], t1 = end[0], s0 = 0, s1 = 4;
#endif
    DECL_IDX(IDX)
    for (int t = t0; t < t1; t++) {
        for (int s = s0; s < s1; s++) {
            float64 hh = 0.0f, kk = 0.0f, hk = 0.0f;
            for (int i = 0; i < H; i += 64) {
                int5 xc = {s * H + i, t, 0, 0, 0}, kc = KV(s * H + i, t);
                const float64 h = v_f32_ld_tnsr_b(xc, X);
                const float64 k = v_f32_ld_tnsr_b(kc, kv);
                int5 wc = {s * H + i, 0, 0, 0, 0};
                const float64 wv = v_f32_ld_tnsr_b(wc, w);
                hh = v_f32_mac_b(h, h, hh);
                kk = v_f32_mac_b(k, k, kk);
                hk = v_f32_mac_b(h * wv, k, hk);
            }
            float64 shh, skk, shk;
            NL_REDUCE_ADD(hh, IDX, shh);
            NL_REDUCE_ADD(kk, IDX, skk);
            NL_REDUCE_ADD(hk, IDX, shk);
            const float64 r = nl_rsqrt(shh * rh + eps) * nl_rsqrt(skk * rh + eps);
            const float64 dot = shk * r * nl_rsqrt((float64)(float)H);
            const float64 a = v_f32_max_b(v_f32_abs_b(dot), 1.0e-6f);
            const float64 root = a * nl_rsqrt(a);
            const float64 gate = nl_sigm(v_f32_sel_less_f32_b(dot, 0.0f, -root, root));
            for (int i = 0; i < H; i += 64) {
                int5 xc = {s * H + i, t, 0, 0, 0};
                int5 vc = KV(4 * H + i, t);
                v_f32_st_tnsr(xc, out, v_f32_mac_b(gate, v_f32_ld_tnsr_b(vc, kv), v_f32_ld_tnsr_b(xc, X)));
            }
        }
    }
}
