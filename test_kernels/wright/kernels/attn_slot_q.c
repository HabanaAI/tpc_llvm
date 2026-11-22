// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Slot attention, queries: copy each slot's M query columns out of the chunk's per-group Q.
// Q is (256, W*G) bf16, group g's columns at g*W (column = h % hpg + hpg*t within a group, d in
// sigma order, as attn_prep writes them); slot s takes columns qcol0 .. qcol0 + M - 1 of its group.
// With tq > 0 the output is token-major per head instead: out column t + tq*h holds
// slot column h + hpg*t (hpg = M / tq; tq is a multiple of 64).
// slots (8, S) i32 per slot: [group, qcol0, qpos0, key0, klen, ...] (see attn_slot_p); slots
// of group -1 are padding and are skipped (their scores are masked out by klen = 0).
// tensors: 0 q (256, W*G) bf16  1 slots (8, S) i32 | 2 out (256, M, S) bf16
// params: G, tq. index space: [1, ceil(M / 64), S] (64 columns of one slot per member)
#include "common.h"

void main(tensor q, tensor slots, tensor out, int G, int tq)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int M = get_dim_size(out, 1);
    const int W = get_dim_size(q, 1) / G;
    int hpg = 0;
    for (int m = 0; tq > 0 && m < M; m += tq)
        hpg++;
    scalar_cache_reset();
    for (int s = off[2]; s < end[2]; s++) {
        int5 gc = {0, s, 0, 0, 0};
        const int g = s_i32_ld_g(gen_addr(gc, slots));
        if (g < 0)
            continue;
        gc[0] = 1;
        const int base = g * W + s_i32_ld_g(gen_addr(gc, slots));
        for (int cb = off[1]; cb < end[1]; cb++) {
            const int m1 = cb * 64 + 64 < M ? cb * 64 + 64 : M;
            int5 qc = {0, base + cb * 64, 0, 0, 0}, oc = {0, cb * 64, s, 0, 0};
            if (tq > 0) {  // the block's head h and first token t (blocks never span heads)
                int h = 0, t = cb * 64;
                while (t >= tq) { t -= tq; h++; }
                for (int m = cb * 64; m < m1; m++, t++) {
                    qc[1] = base + h + hpg * t;
                    qc[0] = 0;
                    const bfloat128 lo = v_bf16_ld_tnsr_b(qc, q);
                    qc[0] = 128;
                    const bfloat128 hi = v_bf16_ld_tnsr_b(qc, q);
                    oc[0] = 0;
                    v_bf16_st_tnsr(oc, out, lo);
                    oc[0] = 128;
                    v_bf16_st_tnsr(oc, out, hi);
                    oc[1]++;
                }
                continue;
            }
            #pragma unroll 4
            for (int m = cb * 64; m < m1; m++) {
                qc[0] = 0;
                const bfloat128 lo = v_bf16_ld_tnsr_b(qc, q);
                qc[0] = 128;
                const bfloat128 hi = v_bf16_ld_tnsr_b(qc, q);
                oc[0] = 0;
                v_bf16_st_tnsr(oc, out, lo);
                oc[0] = 128;
                v_bf16_st_tnsr(oc, out, hi);
                qc[1]++;
                oc[1]++;
            }
        }
    }
}
