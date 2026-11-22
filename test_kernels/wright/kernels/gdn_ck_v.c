// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Chunked gated delta rule for prefill, stage 1b: per (value head h, chunk c):
//   gates beta = sigmoid(b), g = ssm_a * softplus(a + dt_bias) (log decay <= 0), G = cumsum(g)
//   inside the chunk; v = conv1d(k=4)+SiLU of the head's v channels. Padded tokens: beta = g = v = 0.
// Outputs: GB (64, 2, nv, NC) lane vectors [G | beta]; Vn (128, 64, nv, NC) raw v (row-major per
// block); cst_v (128 nv, 3): conv state rows of the v channels.
// b / a come token-major, baT (T, 2 nv) = [b | a], so the 64 tokens of a chunk load as one vector.
// tensors: 0 p (C + ..., T) [q | k | v | ...]  1 baT  2 conv_w  3 cst_in  4 ssm_a  5 dt_bias
//          6 meta | 7 GB  8 Vn  9 cst_v
// FP8 build: 0 p  1 ps  2 baT  3 conv_w ... (as above, shifted by one)
// params: eps, nk. index space: [nv, NC]
#include "common.h"

#define DK 128
#define CH 64

__local__ float64 XA[CH + 3];
__local__ float64 XB[CH + 3];

#ifdef FP8
void main(tensor p, tensor ps, tensor baT, tensor conv_w, tensor cst_in, tensor ssm_a, tensor dt_bias,
          tensor meta, tensor GB, tensor Vn, tensor cst_v, float eps, int nk)
#define PLOAD(c, tok) (v_f32_ld_tnsr_b(c, p) * TOKEN_SCALE(ps, tok, IDXL[0]))
#else
void main(tensor p, tensor baT, tensor conv_w, tensor cst_in, tensor ssm_a, tensor dt_bias, tensor meta,
          tensor GB, tensor Vn, tensor cst_v, float eps, int nk)
#define PLOAD(c, tok) v_f32_ld_tnsr_b(c, p)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    scalar_cache_reset();
    const int T = get_dim_size(p, 1);
    int n_real = meta_i32(meta, 1);
    if (n_real > T) n_real = T;
    uchar256 IDXL[16];
    #pragma unroll
    for (int e = 0; e < 16; e++) IDXL[e] = SHUF_IDX(e);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    const int nv = get_dim_size(GB, 2);
    const int voff = 2 * nk * DK;

    for (int h = off[0]; h < end[0]; h++) {
        int5 hc = {h, 0, 0, 0, 0};
        const float64 A0 = BCAST_DG(v_f32_ld_tnsr_b(hc, ssm_a), 0);
        const float64 D0 = BCAST_DG(v_f32_ld_tnsr_b(hc, dt_bias), 0);
        const float64 Ah = v_f32_shuffle_b(A0, IDXL[0], 0, A0);
        const float64 DTh = v_f32_shuffle_b(D0, IDXL[0], 0, D0);
        const int ca = voff + h * DK, cb = ca + 64;
        float64 wa[4], wb[4];
        #pragma unroll
        for (int k = 0; k < 4; k++) {
            int5 wc = {ca, k, 0, 0, 0};
            wa[k] = v_f32_ld_tnsr_b(wc, conv_w);
            wc[0] = cb;
            wb[k] = v_f32_ld_tnsr_b(wc, conv_w);
        }
        for (int c = off[1]; c < end[1]; c++) {
            const int t0 = c * CH;
            // gates for all 64 tokens at once (lane i = token t0 + i)
            int5 bc = {t0, h, 0, 0, 0};
            const float64 bvec = v_f32_ld_tnsr_b(bc, baT);
            bc[1] = nv + h;
            const float64 avec = v_f32_ld_tnsr_b(bc, baT);
            const float64 realv = v_f32_sel_less_f32_b(lanef, (float)(n_real - t0) - 0.5f, 1.0f, 0.0f);
            const float64 lgv = Ah * nl_softplus(avec + DTh) * realv;     // log decay per token
            const float64 bvv = nl_sigmoid(bvec) * realv;
            float64 Gacc = 0.0f, gv = 0.0f;
#define CUMSUM(DG) {                                                                        \
            const float64 Ld = BCAST_DG(lgv, DG);                                            \
            for (int e = 0; e < 16; e++) {                                                   \
                Gacc = Gacc + v_f32_shuffle_b(Ld, IDXL[e], 0, Ld);                           \
                gv = v_f32_sel_less_f32_b(lanef, (float)((DG) * 16 + e) - 0.5f, gv, Gacc);   \
            } }
            CUMSUM(0) CUMSUM(1) CUMSUM(2) CUMSUM(3)
#undef CUMSUM
            int5 gbc = {0, 0, h, c, 0};
            v_f32_st_tnsr(gbc, GB, gv);
            gbc[1] = 1;
            v_f32_st_tnsr(gbc, GB, bvv);
            // v: conv + SiLU
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
            #pragma unroll 1
            for (int i0 = 0; i0 < CH; i0 += 4) {                  // 4 tokens per step
                float64 ya[4], yb[4];
                #pragma unroll
                for (int u = 0; u < 4; u++) {
                    const int i = i0 + u;
                    ya[u] = v_f32_mac_b(wa[3], XA[i + 3], v_f32_mac_b(wa[2], XA[i + 2], v_f32_mac_b(wa[1], XA[i + 1], wa[0] * XA[i])));
                    yb[u] = v_f32_mac_b(wb[3], XB[i + 3], v_f32_mac_b(wb[2], XB[i + 2], v_f32_mac_b(wb[1], XB[i + 1], wb[0] * XB[i])));
                }
                #pragma unroll
                for (int u = 0; u < 4; u++) {
                    // padded tokens -> 0 by a 0/1 factor
                    const float rf = t0 + i0 + u < n_real ? 1.0f : 0.0f;
                    int5 oc = {0, i0 + u, h, c, 0};
                    v_f32_st_tnsr(oc, Vn, nl_silu2(ya[u]) * rf);
                    oc[0] = 64;
                    v_f32_st_tnsr(oc, Vn, nl_silu2(yb[u]) * rf);
                }
            }
            if (t0 <= n_real - 1 && n_real - 1 < t0 + CH) {
                #pragma unroll
                for (int s2 = 0; s2 < 3; s2++) {
                    const int tt = n_real - 3 + s2;
                    int5 oc2 = {h * DK, s2, 0, 0, 0};
                    v_f32_st_tnsr(oc2, cst_v, XA[tt - t0 + 3]);
                    oc2[0] = h * DK + 64;
                    v_f32_st_tnsr(oc2, cst_v, XB[tt - t0 + 3]);
                }
            }
        }
    }
}
