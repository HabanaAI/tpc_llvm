// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Prefill MoE, pass 3: rows for the pairs. Pooled rows r = PS p + i (page p, row i; PS = 2^psh). Token block j
// continues each expert's running row from po[e] * PS + (pairs of earlier blocks), so the rows of an
// expert are in token order. gidx[r] = token (unused rows: T, the zero column); bm[k, t] = r, or -1
// when the expert is not local or its pages were not allocated (overflow), and for every route of a padding column. Unused rows are filled by member e % NW
// (padding of expert e's last page) and by all members (unallocated pages, split by rows).
// tensors: 0 ids (64,T) i32  1 meta i32  2 cntm (E, NW) i32  3 po (E,) i32  4 pe (P,) i32
//        | 5 gidx (PS P,) i32  6 bm (64, T) i32
// params: lo, K, psh. index space: [NW]
#include "common.h"

#define NW 24
#define MAXE 512

// gidx[a, b) = v: aligned vector stores in the middle, scalar ends
static void fill_rows(tensor gidx, int a, int b, int v)
{
    int a64 = (a + 63) & ~63, b64 = b & ~63;
    if (a64 > b64) { a64 = b; b64 = b; }
    for (int r = a; r < a64; r++) {
        int5 gc = {r, 0, 0, 0, 0};
        s_i32_st_g(gen_addr(gc, gidx), v);
    }
    for (int r = a64; r < b64; r += 64) {
        int5 gc = {r, 0, 0, 0, 0};
        v_i32_st_tnsr(gc, gidx, (int64)v);
    }
    for (int r = b64 > a64 ? b64 : a64; r < b; r++) {
        int5 gc = {r, 0, 0, 0, 0};
        s_i32_st_g(gen_addr(gc, gidx), v);
    }
}

void main(tensor ids, tensor meta, tensor cntm, tensor po, tensor pe, tensor gidx, tensor bm, int lo, int K, int psh)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int T = get_dim_size(ids, 1), E = get_dim_size(cntm, 0), P = get_dim_size(pe, 0);
    const int PS = 1 << psh, R = PS * P;
    scalar_cache_reset();
    const int n_real = meta_i32(meta, 1);
    // per-expert state
    int cur[MAXE], lim[MAXE], tot[MAXE];
    for (int j = off[0]; j < end[0]; j++) {
        for (int e = 0; e < E; e++) { tot[e] = 0; cur[e] = 0; }
        for (int jj = 0; jj < NW; jj++)
            for (int e = 0; e < E; e++) {
                int5 c = {e, jj, 0, 0, 0};
                const int v = s_i32_ld_g(gen_addr(c, cntm));
                tot[e] += v;
                cur[e] += jj < j ? v : 0;
            }
        int used = 0;                                   // pages in use (= first unallocated page)
        for (int e = 0; e < E; e++) {
            int5 oc = {e, 0, 0, 0, 0};
            const int p0 = s_i32_ld_g(gen_addr(oc, po));
            int last = p0 + ((tot[e] + PS - 1) >> psh);         // one past expert e's last allocated page
            last = last < P ? last : P;
            used = last > used ? last : used;
            cur[e] += PS * p0;
            lim[e] = PS * last;                         // rows of expert e: [PS p0, lim)
            if (e % NW == j)                            // padding rows of expert e's last page
                fill_rows(gidx, PS * p0 + tot[e], PS * last, T);
        }
        // unallocated pages: rows [PS used, R) split over the members
        const int r0 = PS * used + (R - PS * used) * j / NW, r1 = PS * used + (R - PS * used) * (j + 1) / NW;
        fill_rows(gidx, r0, r1, T);
        const int t0 = T * j / NW;
        int t1 = T * (j + 1) / NW;
        t1 = t1 < n_real ? t1 : n_real;
        const int64 lane = (int64)read_lane_id_4b_b();
        // padding columns route nowhere
        for (int t = t0 > t1 ? t0 : t1; t < T * (j + 1) / NW; t++) {
            int5 bc = {0, t, 0, 0, 0};
            v_i32_st_tnsr(bc, bm, (int64)-1);
        }
        for (int t = t0; t < t1; t++) {
            int64 row = -1;                             // bm row of token t, one vector store (slots in lanes)
            for (int k = 0; k < K; k++) {
                int5 ic = {k, t, 0, 0, 0};
                const int e = s_i32_ld_g(gen_addr(ic, ids)) - lo;
                if (e < 0 || e >= E)
                    continue;
                const int r = cur[e]++;
                if (r < lim[e]) {
                    int5 gc = {r, 0, 0, 0, 0};
                    s_i32_st_g(gen_addr(gc, gidx), t);
                    row = v_i32_sel_eq_i32_b(lane, (int64)k, (int64)r, row);
                }
            }
            int5 bc = {0, t, 0, 0, 0};
            v_i32_st_tnsr(bc, bm, row);
        }
    }
}
