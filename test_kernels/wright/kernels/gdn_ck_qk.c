// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Chunked gated delta rule for prefill, stage 1a: q / k of one shared q/k head kh per member.
// Qn / Kn are per q/k head, dims (128, 64, nk, NC); the graph replicates them to the value heads
// (h = kh + r nk).
//   causal conv1d(k=4)+SiLU, l2-norm, q *= 1/sqrt(128); padded tokens (>= n_real): 0.
// Outputs, row-major per (h, c) block of 64 tokens x 128 dims, dims (128, 64, nv, NC): Qn, Kn;
// conv state rows of the q/k channels (from the chunk holding the last real token).
// With C = conv_dim = (2 nk + nv) * 128:
// tensors: 0 p (C + ..., T) f32 [q | k | v | ...]  1 conv_w (C,4)  2 cst_in (C,3)  3 meta [pos0, n_real]
//        | 4 Qn  5 Kn  6 cst_qk (2 nk 128, 3): conv state rows of the q/k channels
//        (FP8 build: 0 p  1 ps  2 conv_w  3 cst_in  4 meta | 5 Qn  6 Kn  7 cst_qk)
// params: eps, nk. index space: [nk, NC]
#include "common.h"

#define DK 128
#define CH 64

// conv inputs of the current channel pair for tokens t0-3 .. t0+63
__local__ float64 XA[CH + 3];
__local__ float64 XB[CH + 3];

#ifdef FP8
void main(tensor p, tensor ps, tensor conv_w, tensor cst_in, tensor meta, tensor Qn, tensor Kn,
          tensor cst_qk, float eps, int nk)
#define PLOAD(c, tok) (v_f32_ld_tnsr_b(c, p) * TOKEN_SCALE(ps, tok, IDXL[0]))
#else
void main(tensor p, tensor conv_w, tensor cst_in, tensor meta, tensor Qn, tensor Kn, tensor cst_qk,
          float eps, int nk)
#define PLOAD(c, tok) v_f32_ld_tnsr_b(c, p)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    scalar_cache_reset();
    const int T = get_dim_size(p, 1);
    int n_real = meta_i32(meta, 1);
    if (n_real > T) n_real = T;
    const float qscale = 0.08838834764831845f;   // 1/sqrt(128)
    uchar256 IDXL[16];
    #pragma unroll
    for (int e = 0; e < 16; e++) IDXL[e] = SHUF_IDX(e);

    for (int kh = off[0]; kh < end[0]; kh++) {
        for (int c = off[1]; c < end[1]; c++) {
            const int t0 = c * CH;
            for (int pr = 0; pr < 2; pr++) {             // 0: q, 1: k
                const int ca = pr * nk * DK + kh * DK, cb = ca + 64;
                float64 wa[4], wb[4];
                #pragma unroll
                for (int k = 0; k < 4; k++) {
                    int5 wc = {ca, k, 0, 0, 0};
                    wa[k] = v_f32_ld_tnsr_b(wc, conv_w);
                    wc[0] = cb;
                    wb[k] = v_f32_ld_tnsr_b(wc, conv_w);
                }
                // the 3 inputs before the chunk (conv state for the first chunk), then the chunk itself
                #pragma unroll
                for (int i = 0; i < 3; i++) {
                    const int tt = t0 - 3 + i;
                    int5 xc = {ca, tt, 0, 0, 0}, stc = {ca, tt + 3, 0, 0, 0};
                    XA[i] = tt >= 0 ? PLOAD(xc, tt) : v_f32_ld_tnsr_b(stc, cst_in);
                    xc[0] = cb; stc[0] = cb;
                    XB[i] = tt >= 0 ? PLOAD(xc, tt) : v_f32_ld_tnsr_b(stc, cst_in);
                }
                #pragma unroll 4
                for (int i = 3; i < CH + 3; i++) {
                    const int tt = t0 - 3 + i;
                    int5 xc = {ca, tt, 0, 0, 0};
                    XA[i] = PLOAD(xc, tt);
                    xc[0] = cb;
                    XB[i] = PLOAD(xc, tt);
                }
                const float sc = pr == 0 ? qscale : 1.0f;
                // 4 tokens per iteration
                for (int i0 = 0; i0 < CH; i0 += 4) {
                    float64 ya[4], yb[4];
                    #pragma unroll
                    for (int u = 0; u < 4; u++) {
                        const int i = i0 + u;
                        const float rf = t0 + i < n_real ? 1.0f : 0.0f;   // 0/1 factor
                        const float64 a = v_f32_mac_b(wa[3], XA[i + 3], v_f32_mac_b(wa[2], XA[i + 2], v_f32_mac_b(wa[1], XA[i + 1], wa[0] * XA[i])));
                        const float64 b = v_f32_mac_b(wb[3], XB[i + 3], v_f32_mac_b(wb[2], XB[i + 2], v_f32_mac_b(wb[1], XB[i + 1], wb[0] * XB[i])));
                        ya[u] = nl_silu2(a) * rf;
                        yb[u] = nl_silu2(b) * rf;
                    }
                    float64 ss[4];
                    NL_REDUCE4_ADD(v_f32_mac_b(ya[0], ya[0], yb[0] * yb[0]), v_f32_mac_b(ya[1], ya[1], yb[1] * yb[1]),
                                   v_f32_mac_b(ya[2], ya[2], yb[2] * yb[2]), v_f32_mac_b(ya[3], ya[3], yb[3] * yb[3]),
                                   IDXL, ss[0], ss[1], ss[2], ss[3]);
                    #pragma unroll
                    for (int u = 0; u < 4; u++) {
                        const float64 inv = nl_rsqrt(v_f32_max_b(ss[u], 1e-12f)) * sc;
                        const float64 o0 = ya[u] * inv, o1 = yb[u] * inv;
                        int5 oc = {0, i0 + u, kh, c, 0};
                        if (pr == 0) { v_f32_st_tnsr(oc, Qn, o0); oc[0] = 64; v_f32_st_tnsr(oc, Qn, o1); }
                        else         { v_f32_st_tnsr(oc, Kn, o0); oc[0] = 64; v_f32_st_tnsr(oc, Kn, o1); }
                    }
                }
                // conv state = last 3 real inputs, from the chunk holding the last real token
                if (t0 <= n_real - 1 && n_real - 1 < t0 + CH) {
                    #pragma unroll
                    for (int s2 = 0; s2 < 3; s2++) {
                        const int tt = n_real - 3 + s2;
                        int5 oc2 = {ca, s2, 0, 0, 0};
                        v_f32_st_tnsr(oc2, cst_qk, XA[tt - t0 + 3]);
                        oc2[0] = cb;
                        v_f32_st_tnsr(oc2, cst_qk, XB[tt - t0 + 3]);
                    }
                }
            }
        }
    }
}
