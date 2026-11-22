// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// MLA slot gather (Mistral-Large-4): one slot's C cached keys into a dense bf16 slab, from the
// pages mla_store writes in the byte arena (512, PR, NB) (PR a power of two). A latent page is
// 128 FP8 token rows (natural order) and one row of their f32 scales; a rotary page
// holds four tokens' 64 bf16 per row; an index page two tokens' 128 bf16 per row.
//   mode 0, K: out (D, C, S), slab row k = key k in natural order: the dequantized latent at
//              dims 0..511, then the rotary key at 512..575 when D = 576.
//   mode 2, index keys: out (128, C, S) in natural order from DSA index pages, given as
//              lat_pages.
// Keys at or past the slot's klen and pages of -1 are zeros.
// slots (8, S) i32 per slot: [group, qcol0, qpos0, key0, klen, ...] (see attn_slot_p).
// tensors: 0 arena (512, PR, NB) u8  1 lat_pages (C / 128, S) i32 (first latent row)
//          2 rope_pages (C / 128, S) i32 (first rotary row; unused when D = 512)  3 slots
//          4 after (unread)
//        | 5 out (D, C, S) bf16
// params: mode. index space: [1, C / 32, S] (32 keys per member).
#include "common.h"

#define VG_KEYS 32
#define ROW(r) (r) & pm, (r) >> psh

void main(tensor arena, tensor lat_pages, tensor rope_pages, tensor slots, tensor after, tensor out, int mode)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    int psh = 0;
    while ((1 << psh) < PR)
        psh++;
    const int pm = PR - 1;
    const int D = get_dim_size(out, 0);
    const int rope = mode == 0 && D > 512;
    const bfloat128 zero = 0.0f;
    scalar_cache_reset();
    for (int s = off[2]; s < end[2]; s++) {
        int5 kc = {3, s, 0, 0, 0};
        const int key0 = s_i32_ld_g(gen_addr(kc, slots));
        kc[0] = 4;
        const int klen = s_i32_ld_g(gen_addr(kc, slots));
        for (int j = off[1]; j < end[1]; j++) {
            const int page = j / (128 / VG_KEYS);
            int5 pc = {page, s, 0, 0, 0};
            const int lat0 = s_i32_ld_g(gen_addr(pc, lat_pages));
            const int rope0 = rope ? s_i32_ld_g(gen_addr(pc, rope_pages)) : 0;
            const int cnt = klen - key0 - page * 128;  // live keys of the page
            const int q0 = (j % (128 / VG_KEYS)) * VG_KEYS;
            int5 oc = {0, j * VG_KEYS, s, 0, 0};
            const int full = lat0 >= 0 && q0 + VG_KEYS <= cnt;  // every key of the member live
            if (mode == 2 && full) {
                // a page copy: keys q0 + 2 i, q0 + 2 i + 1 are the two halves of row u0 + i
                const int u0 = lat0 + q0 / 2;
                #pragma unroll 4
                for (int i = 0; i < VG_KEYS / 2; i++) {
                    int5 ic = {0, ROW(u0 + i), 0, 0};
                    const minifloat256 lo = v_f8_ld_tnsr_b(ic, arena);
                    ic[0] = 256;
                    const minifloat256 hi = v_f8_ld_tnsr_b(ic, arena);
                    oc[1] = j * VG_KEYS + 2 * i;
                    v_bf16_st_tnsr(oc, out, __builtin_bit_cast(bfloat128, lo));
                    oc[1]++;
                    v_bf16_st_tnsr(oc, out, __builtin_bit_cast(bfloat128, hi));
                }
                continue;
            }
            if (mode == 2) {
                for (int t = 0; t < VG_KEYS; t++, oc[1]++) {
                    const int key = q0 + t;
                    bfloat128 a = zero;
                    if (lat0 >= 0 && key < cnt) {
                        const int u = lat0 * 2 + key;
                        int5 ic = {(u & 1) * 256, ROW(u >> 1), 0, 0};
                        a = __builtin_bit_cast(bfloat128, v_f8_ld_tnsr_b(ic, arena));
                    }
                    oc[0] = 0;
                    v_bf16_st_tnsr(oc, out, a);
                }
                continue;
            }
            if (full) {  // no per-key liveness test
                #pragma unroll 4
                for (int t = 0; t < VG_KEYS; t++) {
                    const int key = q0 + t;
                    int5 sc = {4 * key, ROW(lat0 + 128), 0, 0};
                    const bf16 sv = (bf16)s_f32_ld_g((__global float*)gen_addr(sc, arena));
                    int5 ac = {0, ROW(lat0 + key), 0, 0};
                    const bfloat256 f = convert_minifloat256_to_bfloat256(v_f8_ld_tnsr_b(ac, arena), SW_LINEAR);
                    ac[0] = 256;
                    const bfloat256 g = convert_minifloat256_to_bfloat256(v_f8_ld_tnsr_b(ac, arena), SW_LINEAR);
                    oc[1] = j * VG_KEYS + t;
                    oc[0] = 0;
                    v_bf16_st_tnsr(oc, out, f.v1 * sv);
                    oc[0] = 128;
                    v_bf16_st_tnsr(oc, out, f.v2 * sv);
                    oc[0] = 256;
                    v_bf16_st_tnsr(oc, out, g.v1 * sv);
                    oc[0] = 384;
                    v_bf16_st_tnsr(oc, out, g.v2 * sv);
                    if (rope) {
                        const int u = rope0 * 4 + key;
                        int5 rc = {(u & 3) * 128, ROW(u >> 2), 0, 0};
                        oc[0] = 512;  // lanes past D are dropped
                        v_bf16_st_tnsr(oc, out, __builtin_bit_cast(bfloat128, v_f8_ld_tnsr_b(rc, arena)));
                    }
                }
                continue;
            }
            for (int t = 0; t < VG_KEYS; t++, oc[1]++) {
                const int key = q0 + t;
                bfloat128 a = zero, b = zero, c = zero, d = zero, e = zero;
                if (lat0 >= 0 && key < cnt) {
                    int5 sc = {4 * key, ROW(lat0 + 128), 0, 0};
                    const bf16 sv = (bf16)s_f32_ld_g((__global float*)gen_addr(sc, arena));
                    int5 ac = {0, ROW(lat0 + key), 0, 0};
                    const bfloat256 f = convert_minifloat256_to_bfloat256(v_f8_ld_tnsr_b(ac, arena), SW_LINEAR);
                    ac[0] = 256;
                    const bfloat256 g = convert_minifloat256_to_bfloat256(v_f8_ld_tnsr_b(ac, arena), SW_LINEAR);
                    a = f.v1 * sv;
                    b = f.v2 * sv;
                    c = g.v1 * sv;
                    d = g.v2 * sv;
                    if (rope) {
                        const int u = rope0 * 4 + key;
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
                if (rope) {
                    oc[0] = 512;  // lanes past D are dropped
                    v_bf16_st_tnsr(oc, out, e);
                }
            }
        }
    }
}
