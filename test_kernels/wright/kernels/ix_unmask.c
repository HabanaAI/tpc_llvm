// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// The ranks' gathered DSA bits (ix_mask) as mla_slot_pm's additive mask: per column and key, 0 when
// kept, else -3e38, for the 2048-key blocks the column can see.
// tensors: 0 bits (K / 32, T) i32  1 vis (T) i32 | 2 mask (K, T) f32
// index space: [T]
#include "common.h"

#define NINF (-3.0e38f)

void main(tensor bits, tensor vis, tensor mask)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const uint64 sink = as_uint64((float64)NINF);
    scalar_cache_reset();
    for (int c = off[0]; c < end[0]; c++) {
        int5 vc = {c, 0, 0, 0, 0};
        const int lim = s_i32_ld_g(gen_addr(vc, vis));
        for (int b = 0; 2048 * b < lim; b++) {
            int5 wc = {64 * b, c, 0, 0, 0};
            const uint64 word = as_uint64(v_i32_ld_tnsr_b(wc, bits));
            for (int j = 0; j < 32; j += 2) {
                int5 ka = {2048 * b + 64 * j, c, 0, 0, 0}, kb = {2048 * b + 64 * (j + 1), c, 0, 0, 0};
                const uint64 a = word & (uint64)(1u << j), d = word & (uint64)(1u << (j + 1));
                v_f32_st_tnsr(ka, mask, as_float64(v_u32_sel_eq_u32_b(a, (uint64)0, sink, (uint64)0)));
                v_f32_st_tnsr(kb, mask, as_float64(v_u32_sel_eq_u32_b(d, (uint64)0, sink, (uint64)0)));
            }
        }
    }
}
