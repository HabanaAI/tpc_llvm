// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// MoE expert down projection, routing-weighted sum over the local slots, plus the shared expert:
//   y = sum_{k: expert local} w_k * W3[e_k] h_k  +  addsh * sigmoid(lg[E_ALL]) * sh
// W3 is block FP8 stored input-major (see moe_up.c): W[e][i][ob] = tau-packed outputs
// 256 ob .. 256 ob + 255 at input i (v1..v4 natural); S[e][ib][ob][half][lane] = block scale of
// (output block 2 ob + half, input block ib), replicated over 64 lanes.
// tensors: 0 h (128, NB, K, T) f32  1 W (256, 128, NO, F/128, E) fp8  2 S (64, 2, NO, F/128, E) f32
//          3 ids (64,T) i32  4 wts (64,T) f32  5 sh (H,T) f32 (shared expert output)
//          6 lg (>= E_ALL+1, T) f32 (row E_ALL = shared-expert gate logit)
//        | 7 y (H,T) f32
// params: lo (first local expert), K, addsh (1 on exactly one rank), E_ALL. index space: [1, T]
#include "common.h"

void main(tensor h, tensor W, tensor S, tensor ids, tensor wts, tensor sh, tensor lg, tensor y,
          int lo, int K, int addsh, int E_ALL)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int F = 128 * get_dim_size(W, 3), NO = get_dim_size(W, 2);
    const int E = get_dim_size(W, 4);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    const int T = get_dim_size(y, 1);
    for (int ti = off[1]; ti < end[1]; ti++) {
        const int t = tok_spread(ti, T);
        int5 tc = {0, t, 0, 0, 0};
        const float64 wv = v_f32_ld_tnsr_b(tc, wts);
        int5 gc = {E_ALL, t, 0, 0, 0};
        const float64 gv = BCAST_DG(v_f32_ld_tnsr_b(gc, lg), 0);
        const float64 gate = nl_sigm(v_f32_shuffle_b(gv, IDX[0], 0, gv)) * (float)addsh;
        for (int ob = 0; ob < NO; ob++) {             // all output blocks per member (ids read once)
            float64 a[4];
            #pragma unroll
            for (int j = 0; j < 4; j++) {
                int5 c = {256 * ob + 64 * j, t, 0, 0, 0};
                a[j] = v_f32_ld_tnsr_b(c, sh) * gate;
            }
            for (int k = 0; k < K; k++) {
                int5 ic = {k, t, 0, 0, 0};
                const int e = s_i32_ld_g(gen_addr(ic, ids)) - lo;
                if (e < 0 || e >= E)
                    continue;
                float64 wk;                              // routing weight of slot k (lane k)
                NL_REDUCE_ADD(v_f32_sel_eq_f32_b(lanef, (float)k, wv, 0.0f), IDX, wk);
                for (int ib = 0; ib < F; ib += 128) {
                    float64 p0 = 0.0f, p1 = 0.0f, p2 = 0.0f, p3 = 0.0f;
                    for (int i0 = ib; i0 < ib + 128; i0 += 64) {
                        int5 hc = {i0 & 127, i0 >> 7, k, t, 0};
                        const float64 hv = v_f32_ld_tnsr_b(hc, h);
                        int5 wc = {0, 0, ob, i0 >> 7, e};
#define DG_STEP(DG)                                                                          \
                        {                                                                    \
                            const float64 hd = BCAST_DG(hv, DG);                             \
                            _Pragma("unroll")                                                \
                            for (int l = 0; l < 16; l++) {                                   \
                                const float64 hb = v_f32_shuffle_b(hd, IDX[l], 0, hd);       \
                                wc[1] = (i0 & 127) + 16 * (DG) + l;                                  \
                                const float256 f = convert_minifloat256_to_float256(v_f8_ld_tnsr_b(wc, W), 0); \
                                p0 = v_f32_mac_b(f.v1, hb, p0);                              \
                                p1 = v_f32_mac_b(f.v2, hb, p1);                              \
                                p2 = v_f32_mac_b(f.v3, hb, p2);                              \
                                p3 = v_f32_mac_b(f.v4, hb, p3);                              \
                            }                                                                \
                        }
                        DG_STEP(0) DG_STEP(1) DG_STEP(2) DG_STEP(3)
#undef DG_STEP
                    }
                    int5 sc = {0, 0, ob, ib >> 7, e};
                    const float64 s0 = v_f32_ld_tnsr_b(sc, S) * wk;
                    sc[1] = 1;
                    const float64 s1 = v_f32_ld_tnsr_b(sc, S) * wk;
                    a[0] = v_f32_mac_b(p0, s0, a[0]);
                    a[1] = v_f32_mac_b(p1, s0, a[1]);
                    a[2] = v_f32_mac_b(p2, s1, a[2]);
                    a[3] = v_f32_mac_b(p3, s1, a[3]);
                }
            }
            #pragma unroll
            for (int j = 0; j < 4; j++) {
                int5 c = {256 * ob + 64 * j, t, 0, 0, 0};
                v_f32_st_tnsr(c, y, a[j]);
            }
        }
    }
}
