// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// GLM DSA: one index slot's C pool keys into a dense bf16 slab, natural order, from the index pages gx_pool
// writes in the byte arena (two pool keys per row, 16 rows = 32 pools per 128-token block). Pool keys at or past
// the slot's klen and pages of -1 are zeros. (mla_gather mode 2 assumes 128 keys per page.)
// slots (8, S) i32 per slot: [group, qcol0, qpos0, key0, klen, qtok0, 0, 0], keys in pool units.
// tensors: 0 arena (512, PR, NB) u8  1 pages (C / 32, S) i32 (first rows)  2 slots (8, S) i32  3 after (unread)
//        | 4 out (128, C, S) bf16
// index space: [1, C / 32, S]
#include "common.h"

#define ROW(r) (r) & pm, (r) >> psh

void main(tensor arena, tensor pages, tensor slots, tensor after, tensor out)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    int psh = 0;
    while ((1 << psh) < PR)
        psh++;
    const int pm = PR - 1;
    const bfloat128 zero = 0.0f;
    scalar_cache_reset();
    for (int s = off[2]; s < end[2]; s++) {
        int5 kc = {3, s, 0, 0, 0};
        const int key0 = s_i32_ld_g(gen_addr(kc, slots));
        kc[0] = 4;
        const int klen = s_i32_ld_g(gen_addr(kc, slots));
        kc[0] = 0;
        const int live = s_i32_ld_g(gen_addr(kc, slots)) >= 0;
        for (int j = off[1]; j < end[1]; j++) {
            int5 pc = {j, s, 0, 0, 0};
            const int page = s_i32_ld_g(gen_addr(pc, pages));
            const int cnt = klen - key0 - j * 32;
            int5 oc = {0, j * 32, s, 0, 0};
            #pragma unroll 2
            for (int t = 0; t < 32; t++) {
                bfloat128 a = zero;
                if (live && page >= 0 && t < cnt) {
                    const int u = page * 2 + t;
                    int5 ic = {(u & 1) * 256, ROW(u >> 1), 0, 0};
                    a = __builtin_bit_cast(bfloat128, v_f8_ld_tnsr_b(ic, arena));
                }
                v_bf16_st_tnsr(oc, out, a);
                oc[1]++;
            }
        }
    }
}
