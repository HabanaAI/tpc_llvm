// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Hyper-connection residual update + the next module's grouped RMSNorm (Qwen3.8-Flash-Next), one member
// per (stream s, token t). The residual holds NS = 4 streams of H: x (NS*H, T) f32, stream s = rows s*H ..
//   mode 1 (combine): xo_s = x_s + y * 2*sigmoid(inj_s / NS),  inj_s = lo[320 + s]
//   mode 2 (init):    xo_s = y                       (embedding copied into every stream)
//   mode 0 (norm):    xo   = x
// then xn = bf16(rmsnorm_per_stream(xo) * w) (sigma order: feeds the down / inject GEMM) and
// r[:, s, t] = 1 / rms(xo_s) (all lanes; hc_mix reuses it).
// tensors: 0 x (NS*H,T) f32  1 y (H,T,NR) f32 (NR > 1: the ranks' all-gathered partials, summed here)  2 lo (>=384,T) f32  3 w (NS*H) f32 (1 + gamma)
//        | 4 xo (NS*H,T) f32  5 xn (NS*H,T) bf16  6 r (64, NS, T) f32
// Modes 0 / 2 still read y and lo (0/1 factors, no branches): pass finite tensors (zeros).
// params: eps, mode. index space: [NS, T]. H multiple of 128.
#include "common.h"

#define NS 4
#define INJ 320
#define MAXH 4096

__local__ float64 XV_[MAXH / 64];     // the updated residual stream of one member (pass 2 reads it back)

void main(tensor x, tensor y, tensor lo, tensor w, tensor xo, tensor xn, tensor rt, float eps, int mode)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(x, 0) / NS, NR = get_dim_size(y, 2);
    const float inv_h = 1.0f / (float)H;
    const float fc = mode == 1 ? 1.0f : 0.0f;       // 0/1 factors instead of branches
    const float fx = mode == 2 ? 0.0f : 1.0f;
    const float fy = mode == 0 ? 0.0f : 1.0f;
    DECL_IDX(IDX)
    // y summed over the NR rank slices (yc[2] = 0 on return)
#define YV() ({ float64 y_ = v_f32_ld_tnsr_b(yc, y); for (yc[2] = 1; yc[2] < NR; yc[2]++) y_ += v_f32_ld_tnsr_b(yc, y); yc[2] = 0; y_; })
    for (int t = off[1]; t < end[1]; t++) {
        int5 lc = {INJ, t, 0, 0, 0};
        const float64 lv = BCAST_DG(v_f32_ld_tnsr_b(lc, lo), 0) * (1.0f / NS);
        for (int s = off[0]; s < end[0]; s++) {
            const float64 g = 2.0f * nl_sigm(v_f32_shuffle_b(lv, IDX[s], 0, lv));
            const float64 a = v_f32_mac_b(g, fc, (float64)(fy - fc));     // combine: g, init: 1, norm: 0
#define XV(c) v_f32_mac_b(v_f32_ld_tnsr_b(c, x), fx, YV() * a)
            // 256 channels per step, all loads before the stores
            float64 acc = 0.0f, acc2 = 0.0f;
            int5 c = {s * H, t, 0, 0, 0}, yc = {0, t, 0, 0, 0};
            for (int i = 0; i < H; i += 256) {
                float64 v[4];
                #pragma unroll
                for (int u = 0; u < 4; u++) {
                    c[0] = s * H + i + 64 * u; yc[0] = i + 64 * u;
                    v[u] = XV(c);
                }
                #pragma unroll
                for (int u = 0; u < 4; u++) {
                    c[0] = s * H + i + 64 * u;
                    v_f32_st_tnsr(c, xo, v[u]);
                    XV_[(i >> 6) + u] = v[u];
                }
                acc = v_f32_mac_b(v[0], v[0], acc);
                acc2 = v_f32_mac_b(v[1], v[1], acc2);
                acc = v_f32_mac_b(v[2], v[2], acc);
                acc2 = v_f32_mac_b(v[3], v[3], acc2);
            }
            float64 ss;
            NL_REDUCE_ADD(acc + acc2, IDX, ss);
            const float64 r = nl_rsqrt(ss * inv_h + eps);
            int5 rc = {0, s, t, 0, 0};
            v_f32_st_tnsr(rc, rt, r);
            int5 wc = {0, 0, 0, 0, 0};
            for (int i = 0; i < H; i += 256) {
                float64 wv[4];
                #pragma unroll
                for (int u = 0; u < 4; u++) {
                    wc[0] = s * H + i + 64 * u;
                    wv[u] = v_f32_ld_tnsr_b(wc, w);
                }
                c[0] = s * H + i;
                v_bf16_st_tnsr(c, xn, pack_sigma(XV_[i >> 6] * r * wv[0], XV_[(i >> 6) + 1] * r * wv[1]));
                c[0] += 128;
                v_bf16_st_tnsr(c, xn, pack_sigma(XV_[(i >> 6) + 2] * r * wv[2], XV_[(i >> 6) + 3] * r * wv[3]));
            }
#undef XV
        }
    }
}
