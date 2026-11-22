// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Decode MoE gate/up over a dynamic work list: the (token, slot) pairs whose expert is local are
// enumerated in every member (scalar reads of the router ids, cached), their 128x128 weight blocks
// (NI input blocks x NB output blocks each, pair-major) form the work list, and member j of NW takes a
// contiguous share: equal work per TPC whatever the routing, and each TPC stays within one or two
// experts' weights.
// Per block: the 128 inputs broadcast into VLM (rebuilt when the input block changes), then 128 weight
// loads + convert + 4 MACs. Output partials as moe_up_v (summed by moe_down_w).
// tensors: 0 x (H,T) f32  1 W (256, 128, NB, NI, E) fp8  2 S (64, 2, NB, NI, E) f32  3 ids (64,T) i32
//        | 4 hp (256, NB, NI, K, T) f32 (written for local pairs only)
// params: lo, K. index space: [NW]
#include "common.h"

#define NW 48
#define MAXP 50                 // (token, slot) pairs: T K <= 50 (MTP-4 verify: 5 x 8 GLM, 5 x 10 Flash-Next)

__local__ float64 XB[128];

void main(tensor x, tensor W, tensor S, tensor ids, tensor hp, int lo, int K)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int E = get_dim_size(W, 4), NB = get_dim_size(W, 2), NI = get_dim_size(W, 3);
    const int T = get_dim_size(x, 1);
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
    const int per = NI * NB, total = n * per;
    for (int w = off[0]; w < end[0]; w++) {
        const int i0 = total * w / NW, i1 = total * (w + 1) / NW;
        // (p, a, c) = coordinates of item i0 (a outer block, c inner block), walked by counters below
        int p = 0, rem = i0;
        while (rem >= per) { rem -= per; p++; }
        int a = 0;
        while (rem >= NB) { rem -= NB; a++; }
        int c = rem;
        int built = -1;                                   // (pair, ib) of the VLM contents
        for (int it = i0; it < i1; it++) {
            const int ib = a, b = c;
            const int t = pt[p], k = pk[p], e = pe[p];
            const int key = p * NI + ib;
            if (key != built) {
                built = key;
                #pragma unroll 1
                for (int h = 0; h < 2; h++) {
                    int5 xc = {128 * ib + 64 * h, t, 0, 0, 0};
                    const float64 v = v_f32_ld_tnsr_b(xc, x);
#define DG_STEP(DG) { const float64 d = BCAST_DG(v, DG); _Pragma("unroll") for (int l = 0; l < 16; l++) \
                          XB[64 * h + 16 * (DG) + l] = v_f32_shuffle_b(d, IDX[l], 0, d); }
                    DG_STEP(0) DG_STEP(1) DG_STEP(2) DG_STEP(3)
#undef DG_STEP
                }
            }
            float64 p0 = 0.0f, p1 = 0.0f, p2 = 0.0f, p3 = 0.0f;
            int5 wc = {0, 0, b, ib, e};
            #pragma unroll 8
            for (int i = 0; i < 128; i++) {
                wc[1] = i;
                const float64 xv = XB[i];
                const float256 f = convert_minifloat256_to_float256(v_f8_ld_tnsr_b(wc, W), 0);
                p0 = v_f32_mac_b(f.v1, xv, p0);
                p1 = v_f32_mac_b(f.v2, xv, p1);
                p2 = v_f32_mac_b(f.v3, xv, p2);
                p3 = v_f32_mac_b(f.v4, xv, p3);
            }
            int5 sc = {0, 0, b, ib, e};
            const float64 sg = v_f32_ld_tnsr_b(sc, S);
            sc[1] = 1;
            const float64 su = v_f32_ld_tnsr_b(sc, S);
            int5 oc = {0, b, ib, k, t};
            v_f32_st_tnsr(oc, hp, p0 * sg);
            oc[0] = 64;  v_f32_st_tnsr(oc, hp, p1 * sg);
            oc[0] = 128; v_f32_st_tnsr(oc, hp, p2 * su);
            oc[0] = 192; v_f32_st_tnsr(oc, hp, p3 * su);
            if (++c == NB) { c = 0; if (++a == NI) { a = 0; p++; } }
        }
    }
}
