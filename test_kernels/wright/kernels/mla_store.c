// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// MLA cache store (Mistral-Large-4): a chunk's new key rows into their arena pages.
// The arena is bytes, viewed as (512, PR, NB) (PR a power of two): row r is
// (., r & (PR - 1), r >> log2 PR). Per token n:
//   - the latent (kn dims 0..511, f32) as FP8 E4M3 in row rows[0, n], in natural
//     order, with its per-token scale (max |latent| / 240 rounded up to a bf16 value, or with POW2
//     (mla_store_p2.c) to a power of two, as f32) in
//     row rows[3, n] at byte 4 rows[4, n] (a latent page is 128 token rows and one row of their 128
//     scales);
//   - the rotary key (kn dims 512..575, rope layers: D = 576) as 64 bf16 in the 128-byte unit
//     rows[1, n] (unit u = row u >> 2, bytes (u & 3) * 128: four tokens per row);
//   - the DSA index key (128 bf16) in the 256-byte unit rows[2, n] (two per row).
// -1 skips an entry (padding columns, or a layer without that part).
// tensors: 0 arena (512, PR, NB) u8  1 rows (8, T) i32  2 kn (D, T) f32  3 ix (128, T) bf16 (any
//          bf16 tensor when no row uses it) | 4 sink (64, 1) i32
// index space: [T]; member 0 writes the sink.
#include "common.h"

#define ROW(r) (r) & pm, (r) >> psh

void main(tensor arena, tensor rows, tensor kn, tensor ix, tensor sink)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    int psh = 0;
    while ((1 << psh) < PR)
        psh++;
    const int pm = PR - 1;
    const int D = get_dim_size(kn, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int n = off[0]; n < end[0]; n++) {
        int5 rc = {0, n, 0, 0, 0};
        const int lat = s_i32_ld_g(gen_addr(rc, rows));
        rc[0] = 1;
        const int rope = s_i32_ld_g(gen_addr(rc, rows));
        rc[0] = 2;
        const int idx = s_i32_ld_g(gen_addr(rc, rows));
        rc[0] = 3;
        const int srow = s_i32_ld_g(gen_addr(rc, rows));
        rc[0] = 4;
        const int sidx = s_i32_ld_g(gen_addr(rc, rows));
        if (lat >= 0) {
            float64 x[8];
            float64 amax = 0.0f;
            #pragma unroll
            for (int k = 0; k < 8; k++) {
                int5 kc = {64 * k, n, 0, 0, 0};
                x[k] = v_f32_ld_tnsr_b(kc, kn);
                amax = v_f32_max_b(amax, v_f32_abs_b(x[k]));
            }
            NL_REDUCE_MAX(amax, IDX, amax);
#ifdef POW2
            const float64 s = token_scale(amax);
            const float64 inv = token_inv(s);
#else
            const float64 s0 = v_f32_max_b(amax, 1e-30f) * (1.0f / 240.0f);
            const float64 s = as_float64((as_uint64(s0) + (uint64)0xFFFFu) & (uint64)0xFFFF0000u);
            const float64 inv = nl_recip(s);
#endif
            int5 dc = {0, ROW(lat), 0, 0};
            v_f8_st_tnsr(dc, arena, pack_lin(x[0] * inv, x[1] * inv, x[2] * inv, x[3] * inv));
            dc[0] = 256;
            v_f8_st_tnsr(dc, arena, pack_lin(x[4] * inv, x[5] * inv, x[6] * inv, x[7] * inv));
            int5 sc = {4 * sidx, ROW(srow), 0, 0};  // lane 0's 4 bytes: the scale
            v_f8_st_tnsr_partial(sc, arena, __builtin_bit_cast(minifloat256, s), 3, 0);
        }
        if (rope >= 0 && D > 512) {
            int5 kc = {512, n, 0, 0, 0};
            const float64 r = v_f32_ld_tnsr_b(kc, kn);  // 64 values
            const bfloat128 b = cvt_bf16_lin(r, r);   // lanes 0..63 natural
            int5 dc = {(rope & 3) * 128, ROW(rope >> 2), 0, 0};
            v_f8_st_tnsr_partial(dc, arena, __builtin_bit_cast(minifloat256, b), 127, 0);
        }
        if (idx >= 0) {
#ifdef IX8
            int5 ic = {0, n, 0, 0, 0};
            const float64 a = v_f32_ld_tnsr_b(ic, ix);
            ic[0] = 64;
            const float64 b = v_f32_ld_tnsr_b(ic, ix);
            float64 m;
            NL_REDUCE_MAX(v_f32_max_b(v_f32_abs_b(a), v_f32_abs_b(b)), IDX, m);
            const float64 s = token_scale(m);
            const float64 inv = token_inv(s);
            int5 dc = {(idx & 3) * 128, ROW(idx >> 2), 0, 0};
            v_f8_st_tnsr_partial(dc, arena, pack_lin(a * inv, b * inv, 0.0f, 0.0f), 127, 0);
            rc[0] = 5;
            const int irow = s_i32_ld_g(gen_addr(rc, rows));
            rc[0] = 6;
            const int ie = s_i32_ld_g(gen_addr(rc, rows));
            int5 sc = {4 * ie, ROW(irow), 0, 0};
            v_f8_st_tnsr_partial(sc, arena, __builtin_bit_cast(minifloat256, s), 3, 0);
#else
            int5 ic = {0, n, 0, 0, 0};
            const bfloat128 r = v_bf16_ld_tnsr_b(ic, ix);
            int5 dc = {(idx & 1) * 256, ROW(idx >> 1), 0, 0};
            v_f8_st_tnsr(dc, arena, __builtin_bit_cast(minifloat256, r));
#endif
        }
    }
    if (off[0] == 0) {
        int5 zc = {0, 0, 0, 0, 0};
        v_i32_st_tnsr(zc, sink, (int64)1);
    }
}
