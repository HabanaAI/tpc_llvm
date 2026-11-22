// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Prefill MoE, pass 1: per token block j (of NW), the number of (token, slot) pairs routed to each
// local expert: cntm[e, j].
// tensors: 0 ids (64,T) i32  1 meta i32 [pos0, n_real] | 2 cntm (E, NW) i32
// params: lo, K. index space: [NW]
#include "common.h"

#define NW 24
#define MAXE 512

void main(tensor ids, tensor meta, tensor cntm, int lo, int K)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int T = get_dim_size(ids, 1), E = get_dim_size(cntm, 0);
    scalar_cache_reset();
    const int n_real = meta_i32(meta, 1);
    int cnt[MAXE];
    for (int j = off[0]; j < end[0]; j++) {
        for (int e = 0; e < E; e++) cnt[e] = 0;
        const int t0 = T * j / NW;
        int t1 = T * (j + 1) / NW;
        t1 = t1 < n_real ? t1 : n_real;
        for (int t = t0; t < t1; t++)
            for (int k = 0; k < K; k++) {
                int5 ic = {k, t, 0, 0, 0};
                const int e = s_i32_ld_g(gen_addr(ic, ids)) - lo;
                if (e >= 0 && e < E) cnt[e]++;
            }
        for (int e = 0; e < E; e++) {
            int5 oc = {e, j, 0, 0, 0};
            s_i32_st_g(gen_addr(oc, cntm), cnt[e]);
        }
    }
}
