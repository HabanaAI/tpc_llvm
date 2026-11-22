// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// MLA slot queries (Mistral-Large-4): copy each slot's M query columns out of the chunk's Q.
// Q is (D, W) bf16 (one KV group: the latent is shared by every head), column = h + hpg * t;
// slot s takes columns qcol0 .. qcol0 + M - 1. Slots of group -1 are padding and are skipped
// (their scores are masked out by klen = 0).
// tensors: 0 q (D, W) bf16  1 slots (8, S) i32 | 2 out (D, M, S) bf16
// index space: [1, ceil(M / 64), S] (64 columns of one slot per member)
#include "common.h"

void main(tensor q, tensor slots, tensor out)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int D = get_dim_size(q, 0), M = get_dim_size(out, 1);
    scalar_cache_reset();
    for (int s = off[2]; s < end[2]; s++) {
        int5 gc = {0, s, 0, 0, 0};
        if (s_i32_ld_g(gen_addr(gc, slots)) < 0)
            continue;
        gc[0] = 1;
        const int base = s_i32_ld_g(gen_addr(gc, slots));
        for (int cb = off[1]; cb < end[1]; cb++) {
            const int m1 = cb * 64 + 64 < M ? cb * 64 + 64 : M;
            int5 qc = {0, base + cb * 64, 0, 0, 0}, oc = {0, cb * 64, s, 0, 0};
            for (int m = cb * 64; m < m1; m++, qc[1]++, oc[1]++) {
                for (int d = 0; d < D; d += 128) {  // a partial last vector is clipped
                    qc[0] = d;
                    oc[0] = d;
                    v_bf16_st_tnsr(oc, out, v_bf16_ld_tnsr_b(qc, q));
                }
            }
        }
    }
}
