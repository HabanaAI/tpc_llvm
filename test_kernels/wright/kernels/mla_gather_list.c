// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// MLA sparse gather (Mistral-Large-4): each column's DSA-listed keys into a dense bf16 slab, as
// mla_gather mode 0 (dequantized FP8 latent, then the rotary key when D = 576) but key k of slot
// s is position list[k, s] of column s's sequence: block row blocks[p >> 7, s], plus this
// layer's page offsets, plus p & 127 (see CacheGeometry.store_rows). Keys at or past the slot's
// count (slots[4, s]) are zeros.
// tensors: 0 arena (512, PR, NB) u8  1 list (C, T) i32 (positions)  2 blocks (P, T) i32 (block
//          rows of each column's sequence)  3 loff (64,) i32 [latent offset, rotary offset (-1:
//          NoPE)]  4 slots (8, T) i32 (ix_list)  5 after (unread)
//        | 6 out (D, C, T) bf16
// index space: [1, C / 32, T] (32 keys per member).
#include "common.h"

#define VG_KEYS 32
#define ROW(r) (r) & pm, (r) >> psh

void main(tensor arena, tensor list, tensor blocks, tensor loff, tensor slots, tensor after,
          tensor out)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    int psh = 0;
    while ((1 << psh) < PR)
        psh++;
    const int pm = PR - 1;
    const int D = get_dim_size(out, 0);
    const bfloat128 zero = 0.0f;
    scalar_cache_reset();
    int5 lc = {0, 0, 0, 0, 0};
    const int lat_off = s_i32_ld_g(gen_addr(lc, loff));
    lc[0] = 1;
    const int rope_off = s_i32_ld_g(gen_addr(lc, loff));
    const int rope = D > 512 && rope_off >= 0;
    for (int s = off[2]; s < end[2]; s++) {
        int5 kc = {4, s, 0, 0, 0};
        const int cnt = s_i32_ld_g(gen_addr(kc, slots));
        for (int j = off[1]; j < end[1]; j++) {
            int5 oc = {0, j * VG_KEYS, s, 0, 0};
            for (int t = 0; t < VG_KEYS; t++, oc[1]++) {
                const int k = j * VG_KEYS + t;
                bfloat128 a = zero, b = zero, c = zero, d = zero, e = zero;
                if (k < cnt) {
                    int5 pc = {k, s, 0, 0, 0};
                    const int p = s_i32_ld_g(gen_addr(pc, list));
                    int5 bc = {p >> 7, s, 0, 0, 0};
                    const int blk = s_i32_ld_g(gen_addr(bc, blocks));
                    const int r = p & 127, lat0 = blk + lat_off;
                    int5 sc = {4 * r, ROW(lat0 + 128), 0, 0};
                    const bf16 sv = (bf16)s_f32_ld_g((__global float*)gen_addr(sc, arena));
                    int5 ac = {0, ROW(lat0 + r), 0, 0};
                    const bfloat256 f = convert_minifloat256_to_bfloat256(v_f8_ld_tnsr_b(ac, arena), SW_LINEAR);
                    ac[0] = 256;
                    const bfloat256 g = convert_minifloat256_to_bfloat256(v_f8_ld_tnsr_b(ac, arena), SW_LINEAR);
                    a = f.v1 * sv;
                    b = f.v2 * sv;
                    c = g.v1 * sv;
                    d = g.v2 * sv;
                    if (rope) {
                        const int u = 4 * (blk + rope_off) + r;
                        int5 rc = {(u & 3) * 128, ROW(u >> 2), 0, 0};
                        e = __builtin_bit_cast(bfloat128, v_f8_ld_tnsr_b(rc, arena));
                    }
                }
                oc[0] = 0;
                v_bf16_st_tnsr(oc, out, a);
                oc[0] = 128;
                v_bf16_st_tnsr(oc, out, b);
                oc[0] = 256;
                v_bf16_st_tnsr(oc, out, c);
                oc[0] = 384;
                v_bf16_st_tnsr(oc, out, d);
                if (D > 512) {
                    oc[0] = 512;  // lanes past D are dropped
                    v_bf16_st_tnsr(oc, out, e);
                }
            }
        }
    }
}
