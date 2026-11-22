// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 candidate blocks, sparse scoring: the FP8 index keys of one slot's 256 candidate blocks of 8
// entries into a dense slab with their scales (as ds_ix_gather), and each slab entry's position. Slot s is column c = slots[5, s]
// and slab entries key0 .. key0 + 2047 (key0 = slots[3, s]) of the column's candidate entries: block u of the
// column's candidate list cand[u, c] (ix_list over ix_bmax's block order, cslots[4, c] blocks) holds positions
// 8 b .. 8 b + 7. Index key of position p: unit p & 127 (four per row) of the index page after latent page
// blocks[p >> 7, c] plus the owner's latent offset; its scale in the page's row 32. Blocks past the column's count read position 0; slots of group -1 are skipped.
// tensors: 0 arena (512, PR, NB) u8  1 cand (KB, T) i32  2 cslots (8, T) i32  3 blocks (P, T) i32
//          4 loff (64,) i32 [latent offset, ...]  5 slots (8, S) i32  6 after (unread)
//        | 7 out (256, 1024, S) fp8 (the (128, 2048, S) slab: entries 2 i, 2 i + 1 in row i)  8 scales (2048, S) f32
//          9 cpos (KC, T) i32 (written at key0 ..)
// params: page_rows (rows of a latent page). index space: [1, 32, S] (8 blocks per member)
#include "common.h"

#define ROW(r) (r) & pm, (r) >> psh

void main(tensor arena, tensor cand, tensor cslots, tensor blocks, tensor loff, tensor slots, tensor after,
          tensor out, tensor scales, tensor cpos, int page_rows)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    int psh = 0;
    while ((1 << psh) < PR)
        psh++;
    const int pm = PR - 1;
    const int64 lane = (int64)read_lane_id_4b_b();
    scalar_cache_reset();
    int5 lc = {0, 0, 0, 0, 0};
    const int ioff = s_i32_ld_g(gen_addr(lc, loff)) + page_rows;
    for (int s = off[2]; s < end[2]; s++) {
        int5 kc = {0, s, 0, 0, 0};
        if (s_i32_ld_g(gen_addr(kc, slots)) < 0)  // padding slot
            continue;
        kc[0] = 5;
        const int c = s_i32_ld_g(gen_addr(kc, slots));
        kc[0] = 3;
        const int key0 = s_i32_ld_g(gen_addr(kc, slots));
        int5 nc = {4, c, 0, 0, 0};
        const int n = s_i32_ld_g(gen_addr(nc, cslots));
        for (int j = off[1]; j < end[1]; j++) {
            const int u0 = (key0 >> 3) + 8 * j;  // the member's first block in the column's list
            int p0[8], r0[8];
            #pragma unroll
            for (int u = 0; u < 8; u++) {
                int5 cc = {u0 + u, c, 0, 0, 0};
                const int P = u0 + u < n ? s_i32_ld_g(gen_addr(cc, cand)) : 0;
                // ix_bmax stores block 8 j + k of a group of 64 at lane 8 k + j
                p0[u] = 8 * ((P & ~63) + 8 * (P & 7) + ((P >> 3) & 7));
            }
            #pragma unroll
            for (int u = 0; u < 8; u++) {
                int5 bc = {p0[u] >> 7, c, 0, 0, 0};
                r0[u] = s_i32_ld_g(gen_addr(bc, blocks)) + ioff;  // the index page's first row
            }
            int64 pv = 0;
            #pragma unroll
            for (int u = 0; u < 8; u++) {
                pv = v_i32_sel_eq_i32_b(lane >> 3, (int64)u, (int64)p0[u] + (lane & 7), pv);
                const int r = p0[u] & 127;  // a multiple of 8: rows r / 4, r / 4 + 1 of the page
                #pragma unroll
                for (int e = 0; e < 8; e += 2) {
                    int5 ic = {((r + e) & 2) * 128, ROW(r0[u] + ((r + e) >> 2)), 0, 0};
                    int5 oc = {0, 32 * j + 4 * u + e / 2, s, 0, 0};
                    v_f8_st_tnsr(oc, out, v_f8_ld_tnsr_b(ic, arena));
                }
                int5 sc = {256 * (r >> 6), ROW(r0[u] + 32), 0, 0};
                int5 so = {64 * j + 8 * u, s, 0, 0, 0};
                v_f32_st_tnsr_partial(so, scales, __builtin_bit_cast(float64, v_u8_ld_tnsr_b(sc, arena)), 7, r & 63);
            }
            int5 pc = {key0 + 64 * j, c, 0, 0, 0};
            v_i32_st_tnsr(pc, cpos, pv);
        }
    }
}
