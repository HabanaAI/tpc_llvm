// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
// Required Notice: Copyright Aaron Botsis
// kv_store for a cache whose K pages, V pages or both are FP8 (see kv_store.c for the bf16 cache).
// The arena is bytes, viewed as (512, PR, NB) (PR a power of two): row r is (., r & (PR - 1), r >> log2 PR).
// A page holds 128 key slots: K key p at slot p % 128, V key p at slot sigma_inv(p % 128).
//   - a bf16 page is 128 rows, slot t in row t (as kv_store.c);
//   - an FP8 page is 65 rows: slot t's 256 values as FP8 E4M3 (natural order) in bytes (t & 1) * 256 of
//     row t / 2, and in row 64 the 128 slots' f32 scales (bytes 4 t). A key's scale is max |x| / 240
//     rounded up to a bf16 value, so kv_gather8 multiplies by it exactly in bf16.
// rows (8, N) i32: per column n, K groups 0..3 then V groups 0..3: for a bf16 page the key's arena row
// (page row + slot), for an FP8 page its 256-byte unit (2 * page row + slot); -1 skips it.
// pos (N) i32: column n's position (its slot in the page). V staging rows are sigma_inv-permuted within
// 128-token blocks (attn_prep), so column n's V row is read at sigma_inv(n).
// tensors: 0 arena (512, PR, NB) u8  1 rows (8, N) i32  2 pos (N) i32  3..6 k0..k3 (256, T) bf16
//          7..10 v0..v3 (256, Tv) bf16 | 11 sink (64, 1) i32
// params: fp8 (bit 0: K pages FP8, bit 1: V pages FP8). index space: [N]; member 0 writes the sink.
#include "common.h"

#define ROW(r) (r) & pm, (r) >> psh

#define STORE(src, srow, slot, f8)                                                           \
    { int5 sc_ = {0, (srow), 0, 0, 0};                                                       \
      const bfloat128 lo_ = v_bf16_ld_tnsr_b(sc_, src);                                      \
      sc_[0] = 128;                                                                          \
      const bfloat128 hi_ = v_bf16_ld_tnsr_b(sc_, src);                                      \
      if (f8) { STORE8(dst, (slot), lo_, hi_) } else { STORE16(dst, lo_, hi_) } }

#define STORE16(dst, lo, hi)                                                                 \
    { int5 dc_ = {0, ROW(dst), 0, 0};                                                        \
      v_f8_st_tnsr(dc_, arena, __builtin_bit_cast(minifloat256, (lo)));                      \
      dc_[0] = 256;                                                                          \
      v_f8_st_tnsr(dc_, arena, __builtin_bit_cast(minifloat256, (hi))); }

#define CLIP8(x) v_f32_max_b(v_f32_min_b((x) * inv_, FP8_MAX), -FP8_MAX)

#define STORE8(unit, slot, lo, hi)                                                           \
    { const float128 a_ = convert_bfloat128_to_float128((lo), SW_LINEAR);                    \
      const float128 b_ = convert_bfloat128_to_float128((hi), SW_LINEAR);                    \
      float64 amax_ = v_f32_max_b(v_f32_max_b(v_f32_abs_b(a_.v1), v_f32_abs_b(a_.v2)),       \
                                  v_f32_max_b(v_f32_abs_b(b_.v1), v_f32_abs_b(b_.v2)));      \
      NL_REDUCE_MAX(amax_, IDX, amax_);                                                      \
      const float64 s0_ = v_f32_max_b(amax_, 1e-30f) * (1.0f / FP8_MAX);                    \
      const float64 s_ = as_float64((as_uint64(s0_) + (uint64)0xFFFFu) & (uint64)0xFFFF0000u); \
      const float64 inv_ = nl_recip(s_);                                                     \
      int5 dc_ = {((unit) & 1) * 256, ROW((unit) >> 1), 0, 0};                               \
      v_f8_st_tnsr(dc_, arena, pack_lin(CLIP8(a_.v1), CLIP8(a_.v2), CLIP8(b_.v1), CLIP8(b_.v2))); \
      int5 sc8_ = {4 * (slot), ROW((((unit) - (slot)) >> 1) + 64), 0, 0};                    \
      v_f8_st_tnsr_partial(sc8_, arena, __builtin_bit_cast(minifloat256, s_), 3, 0); }

void main(tensor arena, tensor rows, tensor pos, tensor k0, tensor k1, tensor k2, tensor k3, tensor v0,
          tensor v1, tensor v2, tensor v3, tensor sink, int fp8)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int PR = get_dim_size(arena, 1);
    int psh = 0;
    while ((1 << psh) < PR)
        psh++;
    const int pm = PR - 1;
    const int k8 = fp8 & 1, v8 = (fp8 >> 1) & 1;
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int n = off[0]; n < end[0]; n++) {
        const int vn = sigma_inv(n);
        int5 pc = {n, 0, 0, 0, 0};
        const int r = s_i32_ld_g(gen_addr(pc, pos)) & 127;
        const int vr = sigma_inv(r);
        for (int j = 0; j < 8; j++) {
            int5 rc = {j, n, 0, 0, 0};
            const int dst = s_i32_ld_g(gen_addr(rc, rows));
            if (dst < 0)
                continue;
            switch (j) {
            case 0: STORE(k0, n, r, k8) break;
            case 1: STORE(k1, n, r, k8) break;
            case 2: STORE(k2, n, r, k8) break;
            case 3: STORE(k3, n, r, k8) break;
            case 4: STORE(v0, vn, vr, v8) break;
            case 5: STORE(v1, vn, vr, v8) break;
            case 6: STORE(v2, vn, vr, v8) break;
            default: STORE(v3, vn, vr, v8)
            }
        }
    }
    if (off[0] == 0) {
        int5 zc = {0, 0, 0, 0, 0};
        v_i32_st_tnsr(zc, sink, (int64)1);
    }
}
