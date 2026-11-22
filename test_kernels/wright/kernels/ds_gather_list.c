// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// mla_gather_list for DeepSeek-V4.1 (D = 512, no separate rotary key; see mla_gather_list.c): each column's
// listed compressed latents into a dense bf16 slab, the dequantized FP8 latent of key k of slot s being position
// list[k, s] of column s's sequence (block row blocks[p >> 7, s] plus the latent offset, row p & 127). Keys at or
// past the slot's count (slots[4, s]) are zeros (their reads use position 0). A member takes 16 keys: their table
// reads first, then their latent reads and stores. A key's scale comes from a vector read of its
// page's scale row, broadcast from the key's lane.
// tensors: 0 arena (512, PR, NB) u8  1 list (C, T) i32 (positions)  2 blocks (P, T) i32  3 loff (64,) i32
//          [latent offset, ...]  4 slots (8, T) i32 (ix_list)  5 after (unread) | 6 out (512, C, T) bf16
// index space: [1, C / 16, T]
#include "common.h"

#define KEYS 16
#define ROW(r) (r) & pm, (r) >> psh

void main(tensor arena, tensor list, tensor blocks, tensor loff, tensor slots, tensor after, tensor out)
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
    int5 lc = {0, 0, 0, 0, 0};
    const int lat_off = s_i32_ld_g(gen_addr(lc, loff));
    for (int s = off[2]; s < end[2]; s++) {
        int5 kc = {4, s, 0, 0, 0};
        const int cnt = s_i32_ld_g(gen_addr(kc, slots));
        for (int j = off[1]; j < end[1]; j++) {
            // every table read of the member's keys first, then their latent reads and stores as one block
            const int k0 = j * KEYS;
            int p[KEYS], lat[KEYS];
            #pragma unroll
            for (int u = 0; u < KEYS; u++) {
                int5 pc = {k0 + u, s, 0, 0, 0};
                p[u] = s_i32_ld_g(gen_addr(pc, list));
                p[u] = k0 + u < cnt ? p[u] : 0;
            }
            #pragma unroll
            for (int u = 0; u < KEYS; u++) {
                int5 bc = {p[u] >> 7, s, 0, 0, 0};
                lat[u] = s_i32_ld_g(gen_addr(bc, blocks)) + lat_off;
            }
            #pragma unroll
            for (int u = 0; u < KEYS; u++) {
                const int r = p[u] & 127;
                int5 sc = {256 * (r >> 6), ROW(lat[u] + 128), 0, 0};
                const float64 sr = __builtin_bit_cast(float64, v_u8_ld_tnsr_b(sc, arena));
                int5 ac = {0, ROW(lat[u] + r), 0, 0};
                const minifloat256 a = v_f8_ld_tnsr_b(ac, arena);
                ac[0] = 256;
                const minifloat256 b = v_f8_ld_tnsr_b(ac, arena);
                const bfloat256 f = convert_minifloat256_to_bfloat256(a, SW_LINEAR);
                const bfloat256 g = convert_minifloat256_to_bfloat256(b, SW_LINEAR);
                const int live = k0 + u < cnt;
                const int e = r & 15, dg = (r >> 4) & 3;
                const int64 cv = (int64)((0x80 | ((e >> 3) << 5) | (e & 7)) * 0x01010101);
                float64 bs = v_f32_shuffle_b(sr, __builtin_bit_cast(uchar256, cv), 0, sr);
                bs = dg == 0 ? BCAST_DG(bs, 0) : dg == 1 ? BCAST_DG(bs, 1) : dg == 2 ? BCAST_DG(bs, 2) : BCAST_DG(bs, 3);
                float128 b2;
                b2.v1 = bs;
                b2.v2 = bs;
                const bfloat128 su = v_convert_f32_to_bf16_all_b(b2, SW_RHNE);
                int5 oc = {0, k0 + u, s, 0, 0};
                v_bf16_st_tnsr(oc, out, live ? f.v1 * su : zero);
                oc[0] = 128;
                v_bf16_st_tnsr(oc, out, live ? f.v2 * su : zero);
                oc[0] = 256;
                v_bf16_st_tnsr(oc, out, live ? g.v1 * su : zero);
                oc[0] = 384;
                v_bf16_st_tnsr(oc, out, live ? g.v2 * su : zero);
            }
        }
    }
}
