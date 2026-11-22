// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Decode MoE down projection over the same dynamic work list idea as moe_up_w: blocks (pair, input
// block b, output block ob), pair-major then b; member j of NW takes a contiguous share. Per (pair, b):
// h = silu(sum_ib gate) * (sum_ib up) for the 128 inputs (moe_up_w partials), broadcast into VLM;
// per block 128 weight loads -> yk[:, b + NB k, t] = w_k * W3[e][ob, b] h (256 outputs).
// Only local pairs' columns are written; moe_sum_w adds the local ones.
// tensors: 0 hp (256, NB, NI, K, T) f32  1 W (256, 128, NO, NB, E) fp8  2 S (64, 2, NO, NB, E) f32
//          3 ids (64,T) i32  4 wts (64,T) f32 | 5 yk (H, NB*K, T) f32
// params: lo, K. index space: [NW]
#include "common.h"

#define NW 48
#define MAXP 50                 // (token, slot) pairs: T K <= 50 (MTP-4 verify: 5 x 8 GLM, 5 x 10 Flash-Next)

__local__ float64 HB[128];

#ifdef LIM
void main(tensor hp, tensor W, tensor S, tensor ids, tensor wts, tensor yk, int lo, int K, float lim)
#else
void main(tensor hp, tensor W, tensor S, tensor ids, tensor wts, tensor yk, int lo, int K)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int E = get_dim_size(W, 4), NO = get_dim_size(W, 2), NB = get_dim_size(W, 3);
    const int NI = get_dim_size(hp, 2), T = get_dim_size(hp, 4);
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    int pt[MAXP], pk[MAXP], pe[MAXP];
    int n = 0;
    for (int t = 0; t < T; t++)
        for (int k = 0; k < K; k++) {
            int5 ic = {k, t, 0, 0, 0};
            const int e = s_i32_ld_g(gen_addr(ic, ids)) - lo;
            if (e >= 0 && e < E && n < MAXP) { pt[n] = t; pk[n] = k; pe[n] = e; n++; }
        }
    const int per = NB * NO, total = n * per;
    for (int w = off[0]; w < end[0]; w++) {
        const int i0 = total * w / NW, i1 = total * (w + 1) / NW;
        // (p, a, c) = coordinates of item i0 (a outer block, c inner block), walked by counters below
        int p = 0, rem = i0;
        while (rem >= per) { rem -= per; p++; }
        int a = 0;
        while (rem >= NO) { rem -= NO; a++; }
        int c = rem;
        int built = -1;
        float64 wk = 0.0f;
        for (int it = i0; it < i1; it++) {
            const int b = a, ob = c;
            const int t = pt[p], k = pk[p], e = pe[p];
            const int key = p * NB + b;
            if (key != built) {
                built = key;
                int5 tc = {0, t, 0, 0, 0};
                NL_REDUCE_ADD(v_f32_sel_eq_f32_b(lanef, (float)k, v_f32_ld_tnsr_b(tc, wts), 0.0f), IDX, wk);
                float64 g0 = 0.0f, g1 = 0.0f, u0 = 0.0f, u1 = 0.0f;
                for (int ib = 0; ib < NI; ib++) {
                    int5 c = {0, b, ib, k, t};
                    g0 += v_f32_ld_tnsr_b(c, hp); c[0] = 64;
                    g1 += v_f32_ld_tnsr_b(c, hp); c[0] = 128;
                    u0 += v_f32_ld_tnsr_b(c, hp); c[0] = 192;
                    u1 += v_f32_ld_tnsr_b(c, hp);
                }
                const float64 h0 = nl_silus(CLAMP_G(g0)) * CLAMP_U(u0), h1 = nl_silus(CLAMP_G(g1)) * CLAMP_U(u1);
#define DG_STEP(v, base, DG) { const float64 d = BCAST_DG(v, DG); _Pragma("unroll") for (int l = 0; l < 16; l++) \
                                   HB[(base) + 16 * (DG) + l] = v_f32_shuffle_b(d, IDX[l], 0, d); }
                DG_STEP(h0, 0, 0) DG_STEP(h0, 0, 1) DG_STEP(h0, 0, 2) DG_STEP(h0, 0, 3)
                DG_STEP(h1, 64, 0) DG_STEP(h1, 64, 1) DG_STEP(h1, 64, 2) DG_STEP(h1, 64, 3)
#undef DG_STEP
            }
            float64 p0 = 0.0f, p1 = 0.0f, p2 = 0.0f, p3 = 0.0f;
            int5 wc = {0, 0, ob, b, e};
            #pragma unroll 8
            for (int i = 0; i < 128; i++) {
                wc[1] = i;
                const float64 hv = HB[i];
                const float256 f = convert_minifloat256_to_float256(v_f8_ld_tnsr_b(wc, W), 0);
                p0 = v_f32_mac_b(f.v1, hv, p0);
                p1 = v_f32_mac_b(f.v2, hv, p1);
                p2 = v_f32_mac_b(f.v3, hv, p2);
                p3 = v_f32_mac_b(f.v4, hv, p3);
            }
            int5 sc = {0, 0, ob, b, e};
            const float64 s0 = v_f32_ld_tnsr_b(sc, S) * wk;
            sc[1] = 1;
            const float64 s1 = v_f32_ld_tnsr_b(sc, S) * wk;
            int5 oc = {256 * ob, b + NB * k, t, 0, 0};
            v_f32_st_tnsr(oc, yk, p0 * s0); oc[0] += 64; v_f32_st_tnsr(oc, yk, p1 * s0);
            oc[0] += 64; v_f32_st_tnsr(oc, yk, p2 * s1); oc[0] += 64; v_f32_st_tnsr(oc, yk, p3 * s1);
            if (++c == NO) { c = 0; if (++a == NB) { a = 0; p++; } }
        }
    }
}
