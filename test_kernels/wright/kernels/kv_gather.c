// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Gather token-major K or V pages of the cache arena into a dense (256, C, S) bf16 slab: slot s
// takes C / 128 pages, and row t of page j lands at (., j * 128 + t, s).
// The arena is viewed as (256, PR, NB) bf16 rows of 512 bytes; a page is 128 rows, one per key
// (V pages hold their keys in sigma_inv order: sig = 1;
// sig = 2 un-permutes them, so slab row j * 128 + k holds key k).
// The table holds each page's first row, a multiple of 128; PR is a multiple of 128 too.
// Rows of keys at or past the slot's klen and pages of -1 are written as zeros.
// slots (8, S) i32 per slot: [group, qcol0, qpos0, key0, klen, ...] (see attn_slot_p).
// tensors: 0 arena (256, PR, NB) bf16  1 pages (C / 128, S) i32  2 slots | 3 out (256, C, S) bf16
// params: sig. index space: [1, C / VG_KEYS, S] (key groups of a quarter page, slots; a member
// copies whole 512-byte rows).
#include "common.h"

#define VG_KEYS 32

// -DDEP (kv_gather_d.c): one more input ``dep``, unread: a data edge from the node that wrote the pages this
// launch reads (the K/V store of the same graph), which orders the two.
#ifdef DEP
void main(tensor arena, tensor pages, tensor slots, tensor dep, tensor out, int sig)
#else
void main(tensor arena, tensor pages, tensor slots, tensor out, int sig)
#endif
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    const bfloat128 zero = 0.0f;
    scalar_cache_reset();
    for (int s = off[2]; s < end[2]; s++) {
        int5 kc = {3, s, 0, 0, 0};
        const int key0 = s_i32_ld_g(gen_addr(kc, slots));
        kc[0] = 4;
        const int klen = s_i32_ld_g(gen_addr(kc, slots));
        for (int j = off[1]; j < end[1]; j++) {
            int5 pc = {j / (128 / VG_KEYS), s, 0, 0, 0};
            const int first = s_i32_ld_g(gen_addr(pc, pages));
            const int r0 = first % PR + (j % (128 / VG_KEYS)) * VG_KEYS, nb = first / PR;
            const int q0 = (j % (128 / VG_KEYS)) * VG_KEYS;    // first page row of the member
            const int cnt = klen - key0 - (j / (128 / VG_KEYS)) * 128;  // live keys of the page
            int5 oc = {0, j * VG_KEYS, s, 0, 0};
            if (first < 0 || cnt <= 0) {
                for (int t = 0; t < VG_KEYS; t++, oc[1]++) {
                    oc[0] = 0;
                    v_bf16_st_tnsr(oc, out, zero);
                    oc[0] = 128;
                    v_bf16_st_tnsr(oc, out, zero);
                }
                continue;
            }
            int5 ac = {0, r0, nb, 0, 0};
            if (sig == 2) {  // natural key order: output row q0 + t <- stored row sigma_inv(q0 + t)
                for (int t = 0; t < VG_KEYS; t++, oc[1]++) {
                    const int key = q0 + t;
                    bfloat128 lo = zero, hi = zero;
                    if (key < cnt) {
                        ac[1] = first % PR + (key < 64 ? 2 * key : 2 * key - 127);
                        ac[0] = 0;
                        lo = v_bf16_ld_tnsr_b(ac, arena);
                        ac[0] = 128;
                        hi = v_bf16_ld_tnsr_b(ac, arena);
                    }
                    oc[0] = 0;
                    v_bf16_st_tnsr(oc, out, lo);
                    oc[0] = 128;
                    v_bf16_st_tnsr(oc, out, hi);
                }
                continue;
            }
            if (cnt < 128) {  // a sequence's newest page: rows of keys >= cnt are stale
                for (int t = 0; t < VG_KEYS; t++, ac[1]++, oc[1]++) {
                    const int r = q0 + t;
                    const int key = sig ? ((r & 1) ? 64 + r / 2 : r / 2) : r;
                    bfloat128 lo = zero, hi = zero;
                    if (key < cnt) {
                        ac[0] = 0;
                        lo = v_bf16_ld_tnsr_b(ac, arena);
                        ac[0] = 128;
                        hi = v_bf16_ld_tnsr_b(ac, arena);
                    }
                    oc[0] = 0;
                    v_bf16_st_tnsr(oc, out, lo);
                    oc[0] = 128;
                    v_bf16_st_tnsr(oc, out, hi);
                }
                continue;
            }
            #pragma unroll 8
            for (int t = 0; t < VG_KEYS; t++) {
                ac[0] = 0;
                const bfloat128 lo = v_bf16_ld_tnsr_b(ac, arena);
                ac[0] = 128;
                const bfloat128 hi = v_bf16_ld_tnsr_b(ac, arena);
                oc[0] = 0;
                v_bf16_st_tnsr(oc, out, lo);
                oc[0] = 128;
                v_bf16_st_tnsr(oc, out, hi);
                oc[1]++;
                ac[1]++;
            }
        }
    }
}
