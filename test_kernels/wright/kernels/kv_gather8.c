// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
// Required Notice: Copyright Aaron Botsis
// kv_gather for a cache with FP8 pages (see kv_gather.c for the bf16 cache, kv_store8.c for the page
// formats): gather K or V pages into a dense (256, C, S) bf16 slab, dequantizing FP8 pages. Slot s takes
// C / 128 pages; slab row j * 128 + t takes slot t of page j (sig 0, 1), or with sig = 2 key t (V pages
// hold key k at slot sigma_inv(k)).
// The arena is bytes, viewed as (512, PR, NB) (PR a power of two). fp8 = 1: the pages are FP8 (65 rows);
// fp8 = 0: they are bf16 (128 rows), read through the byte view.
// Rows of keys at or past the slot's klen and pages of -1 are written as zeros.
// On a page whose keys are all live, a member reads the scales of its 32 keys as one vector of the scale row
// and broadcasts each one in the registers.
// tensors: 0 arena (512, PR, NB) u8  1 pages (C / 128, S) i32 (first rows)  2 slots (8, S) i32
//        | 3 out (256, C, S) bf16
// params: sig, fp8. index space: [1, C / VG_KEYS, S] (key groups of a quarter page, slots).
#include "common.h"

#define VG_KEYS 32
#define ROW(r) (r) & pm, (r) >> psh

// slot t of the page at row `first`, as two bf16 halves (dims 0..127, 128..255)
#define LOAD(first, t, lo, hi)                                                               \
    if (fp8) {                                                                               \
        int5 sc_ = {4 * (t), ROW((first) + 64), 0, 0};                                       \
        const bf16 sv_ = (bf16)s_f32_ld_g((__global float*)gen_addr(sc_, arena));            \
        int5 ac_ = {((t) & 1) * 256, ROW((first) + ((t) >> 1)), 0, 0};                       \
        const bfloat256 f_ = convert_minifloat256_to_bfloat256(v_f8_ld_tnsr_b(ac_, arena), SW_LINEAR); \
        lo = f_.v1 * sv_;                                                                    \
        hi = f_.v2 * sv_;                                                                    \
    } else {                                                                                 \
        int5 ac_ = {0, ROW((first) + (t)), 0, 0};                                            \
        lo = __builtin_bit_cast(bfloat128, v_f8_ld_tnsr_b(ac_, arena));                      \
        ac_[0] = 256;                                                                        \
        hi = __builtin_bit_cast(bfloat128, v_f8_ld_tnsr_b(ac_, arena));                      \
    }

void main(tensor arena, tensor pages, tensor slots, tensor out, int sig, int fp8)
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
        for (int j = off[1]; j < end[1]; j++) {
            int5 pc = {j / (128 / VG_KEYS), s, 0, 0, 0};
            const int first = s_i32_ld_g(gen_addr(pc, pages));
            const int q0 = (j % (128 / VG_KEYS)) * VG_KEYS;
            const int cnt = klen - key0 - (j / (128 / VG_KEYS)) * 128;
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
            if (cnt >= 128) {  // every key of the page live
                if (fp8) {
                    // the member's 32 scales are in one vector of the scale row: sig 0, 1: slot q0 + t at lane
                    // (q0 & 32) + t of vector q0 / 64; sig 2: key q0 + t at slot s0 + 2 t, lane (s0 & 63) + 2 t of
                    // vector s0 / 64 (s0 = 2 q0 or 2 q0 - 127, so s0 & 63 is 0 or 1)
                    const int s0 = sig == 2 ? (q0 < 64 ? 2 * q0 : 2 * q0 - 127) : q0;
                    int5 sc = {(s0 >> 6) * 256, ROW(first + 64), 0, 0};
                    const float64 scl = __builtin_bit_cast(float64, v_u8_ld_tnsr_b(sc, arena));
                    if (sig == 2) {
                        // key q0 + t is row (q0 & 63) + t, half q0 >= 64; its scale lane: dual group t / 8,
                        // element (s0 & 1) + 2 (t % 8)
                        const float64 b0 = BCAST_DG(scl, 0), b1 = BCAST_DG(scl, 1);
                        const float64 b2 = BCAST_DG(scl, 2), b3 = BCAST_DG(scl, 3);
                        const int par = s0 & 1;
                        int5 ac = {(q0 >= 64) * 256, 0, 0, 0, 0};
                        #pragma unroll
                        for (int t = 0; t < VG_KEYS; t++) {
                            const float64 g = t < 8 ? b0 : t < 16 ? b1 : t < 24 ? b2 : b3;
                            const float64 b = v_f32_shuffle_b(g, (uchar256)SHUF_IDX(par + 2 * (t & 7)), 0, g);
                            float128 bb;
                            bb.v1 = b;
                            bb.v2 = b;
                            const bfloat128 sv = v_convert_f32_to_bf16_all_b(bb, SW_RHNE);
                            const int r = first + (q0 & 63) + t;
                            ac[1] = r & pm;
                            ac[2] = r >> psh;
                            const bfloat256 f = convert_minifloat256_to_bfloat256(v_f8_ld_tnsr_b(ac, arena), SW_LINEAR);
                            oc[1] = j * VG_KEYS + t;
                            oc[0] = 0;
                            v_bf16_st_tnsr(oc, out, f.v1 * sv);
                            oc[0] = 128;
                            v_bf16_st_tnsr(oc, out, f.v2 * sv);
                        }
                    } else {
                        // slot q0 + t is half t & 1 of row (q0 + t) / 2; its scale: dual group (q0 & 32) / 16 + t / 16,
                        // element t % 16
                        const int d0 = (q0 & 32) >> 4;
                        const float64 b0 = d0 == 0 ? BCAST_DG(scl, 0) : BCAST_DG(scl, 2);
                        const float64 b1 = d0 == 0 ? BCAST_DG(scl, 1) : BCAST_DG(scl, 3);
                        #pragma unroll
                        for (int t = 0; t < VG_KEYS; t++) {
                            const float64 g = t < 16 ? b0 : b1;
                            const float64 b = v_f32_shuffle_b(g, (uchar256)SHUF_IDX(t & 15), 0, g);
                            float128 bb;
                            bb.v1 = b;
                            bb.v2 = b;
                            const bfloat128 sv = v_convert_f32_to_bf16_all_b(bb, SW_RHNE);
                            const int u = q0 + t, r = first + (u >> 1);
                            int5 ac = {(u & 1) * 256, r & pm, r >> psh, 0, 0};
                            const bfloat256 f = convert_minifloat256_to_bfloat256(v_f8_ld_tnsr_b(ac, arena), SW_LINEAR);
                            oc[1] = j * VG_KEYS + t;
                            oc[0] = 0;
                            v_bf16_st_tnsr(oc, out, f.v1 * sv);
                            oc[0] = 128;
                            v_bf16_st_tnsr(oc, out, f.v2 * sv);
                        }
                    }
                } else {
                    // bf16 pages: slab row q0 + t is row first + slot, slot q0 + t (sig 0, 1) or s0 + 2 t (sig 2)
                    const int s0 = sig == 2 ? (q0 < 64 ? 2 * q0 : 2 * q0 - 127) : q0;
                    const int ds = sig == 2 ? 2 : 1;
                    #pragma unroll 8
                    for (int t = 0; t < VG_KEYS; t++) {
                        const int r = first + s0 + ds * t;
                        int5 ac = {0, r & pm, r >> psh, 0, 0};
                        const bfloat128 lo = __builtin_bit_cast(bfloat128, v_f8_ld_tnsr_b(ac, arena));
                        ac[0] = 256;
                        const bfloat128 hi = __builtin_bit_cast(bfloat128, v_f8_ld_tnsr_b(ac, arena));
                        oc[1] = j * VG_KEYS + t;
                        oc[0] = 0;
                        v_bf16_st_tnsr(oc, out, lo);
                        oc[0] = 128;
                        v_bf16_st_tnsr(oc, out, hi);
                    }
                }
                continue;
            }
            for (int t = 0; t < VG_KEYS; t++, oc[1]++) {
                const int r = q0 + t;
                const int key = sig == 1 ? ((r & 1) ? 64 + r / 2 : r / 2) : r;
                const int slot = sig == 2 ? (key < 64 ? 2 * key : 2 * key - 127) : r;
                bfloat128 lo = zero, hi = zero;
                if (key < cnt) {
                    LOAD(first, slot, lo, hi)
                }
                oc[0] = 0;
                v_bf16_st_tnsr(oc, out, lo);
                oc[0] = 128;
                v_bf16_st_tnsr(oc, out, hi);
            }
        }
    }
}
