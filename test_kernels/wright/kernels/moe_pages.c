// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Prefill MoE, pass 2: 2^psh-row pages per local expert (ceil(count / 2^psh)), allocated in expert order:
// po[e] = first page of expert e, pe[p] = expert of page p (-1: unused). Pages beyond P are not
// allocated (their pairs overflow to the per-token kernels).
// tensors: 0 cntm (E, NW) i32 | 1 po (E,) i32  2 pe (P,) i32. params: psh. index space: [1]
#include "common.h"

#define MAXE 512

void main(tensor cntm, tensor po, tensor pe, int psh)
{
    const int E = get_dim_size(cntm, 0), NW = get_dim_size(cntm, 1), P = get_dim_size(pe, 0);
    scalar_cache_reset();
    // totals per expert
    int tot[MAXE];
    for (int e = 0; e < E; e++) tot[e] = 0;
    for (int j = 0; j < NW; j++)
        for (int e = 0; e < E; e++) {
            int5 c = {e, j, 0, 0, 0};
            tot[e] += s_i32_ld_g(gen_addr(c, cntm));
        }
    int next = 0;
    for (int e = 0; e < E; e++) {
        int5 oc = {e, 0, 0, 0, 0};
        s_i32_st_g(gen_addr(oc, po), next);
        const int pg = (tot[e] + (1 << psh) - 1) >> psh;
        for (int q = 0; q < pg && next < P; q++, next++) {
            int5 pc = {next, 0, 0, 0, 0};
            s_i32_st_g(gen_addr(pc, pe), e);
        }
    }
    for (int p = next; p < P; p++) {
        int5 pc = {p, 0, 0, 0, 0};
        s_i32_st_g(gen_addr(pc, pe), -1);
    }
}
