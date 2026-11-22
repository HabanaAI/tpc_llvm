// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DSA index keys of one slot (DeepSeek-V4.1, FP8 index pages; see mla_store_i8.c): the slot's C keys from its
// index pages into a dense FP8 slab, and each key's scale. An index page holds 128 keys of 128 FP8 bytes, four
// per row from its first row, then a row of their f32 scales. Keys at or past the slot's klen (slots[4, s], from
// key0 = slots[3, s]) and pages of -1 get zeros and scale 0.
// tensors: 0 arena (512, PR, NB) u8  1 pages (C / 128, S) i32 (first row of each index page)  2 slots (8, S) i32
//          3 after (unread) | 4 out (256, C / 2, S) fp8 (the (128, C, S) slab: keys 2 i, 2 i + 1 in row i)
//          5 scales (C, S) f32
// index space: [1, C / 32, S] (32 keys of a page per member)
#include "common.h"

#define KEYS 32
#define ROW(r) (r) & pm, (r) >> psh

void main(tensor arena, tensor pages, tensor slots, tensor after, tensor out, tensor scales)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    int psh = 0;
    while ((1 << psh) < PR)
        psh++;
    const int pm = PR - 1;
    const minifloat256 zero8 = 0;
    const float64 zero = 0.0f;
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    scalar_cache_reset();
    for (int s = off[2]; s < end[2]; s++) {
        int5 kc = {3, s, 0, 0, 0};
        const int key0 = s_i32_ld_g(gen_addr(kc, slots));
        kc[0] = 4;
        const int klen = s_i32_ld_g(gen_addr(kc, slots));
        for (int j = off[1]; j < end[1]; j++) {
            const int page = j / (128 / KEYS);
            int5 pc = {page, s, 0, 0, 0};
            const int row0 = s_i32_ld_g(gen_addr(pc, pages));
            const int cnt = row0 >= 0 ? klen - key0 - page * 128 : 0;  // live keys of the page
            const int q0 = (j % (128 / KEYS)) * KEYS;
            #pragma unroll 4
            for (int i = 0; i < KEYS; i += 2) {
                // keys q0 + i, q0 + i + 1: half of row row0 + (q0 + i) / 4, one slab row
                const int key = q0 + i;
                int5 ic = {(key & 2) * 128, ROW(row0 + (key >> 2)), 0, 0};
                const minifloat256 v = row0 >= 0 ? v_f8_ld_tnsr_b(ic, arena) : zero8;
                int5 oc = {0, (j * KEYS + i) / 2, s, 0, 0};
                if (key + 1 < cnt) {
                    v_f8_st_tnsr(oc, out, v);
                } else {
                    v_f8_st_tnsr_partial(oc, out, key < cnt ? v : zero8, 127, 0);
                    oc[0] = 128;
                    v_f8_st_tnsr_partial(oc, out, zero8, 127, 128);
                }
            }
            // the scales of keys q0 .. q0 + 31: lanes q0 % 64 .. of the scale row's vector q0 / 64
            int5 sc = {256 * (q0 >> 6), ROW(row0 + 32), 0, 0};
            float64 sv = row0 >= 0 ? __builtin_bit_cast(float64, v_u8_ld_tnsr_b(sc, arena)) : zero;
            sv = v_f32_sel_less_f32_b(lanef, (float)(cnt - (q0 & ~63)), sv, zero);
            int5 oc = {j * KEYS, s, 0, 0, 0};
            v_f32_st_tnsr_partial(oc, scales, sv, KEYS - 1, q0 & 63);
        }
    }
}
