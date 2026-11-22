// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// A prefill chunk's DSA selection for this rank's columns as bits (Mistral-Large-4):
// bit j of lane i of a column's word vector b says that key
// 2048 b + 64 j + i is visible (below vis[c]) and kept (mkey(score) >= thr[c]: ix_thr). ix_unmask
// expands them.
// tensors: 0 score (K, T) f32  1 vis (T) i32  2 thr (64, T) i32 | 3 bits (K / 32, T) i32
// index space: [K / 2048, T]
#include "common.h"

static inline uint64 mkey(float64 v)
{
    const uint64 b = as_uint64(v);
    return as_uint64(v_f32_sel_less_f32_b(v, 0.0f, as_float64(~b), as_float64(b | (uint64)0x80000000u)));
}

void main(tensor score, tensor vis, tensor thr, tensor bits)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const float64 lanef = v_convert_i32_to_f32_b((int64)read_lane_id_4b_b(), SW_RHNE, 0, 1, 0);
    scalar_cache_reset();
    for (int c = off[1]; c < end[1]; c++) {
        int5 vc = {c, 0, 0, 0, 0};
        const int lim = s_i32_ld_g(gen_addr(vc, vis));
        const float limf = (float)lim;
        int5 tc = {0, c, 0, 0, 0};
        const uint64 th = __builtin_bit_cast(uint64, v_i32_ld_tnsr_b(tc, thr));
        for (int b = off[0]; b < end[0]; b++) {
            uint64 word = 0;
            const int key0 = 2048 * b;
            if (key0 < lim) {
                for (int j = 0; j < 32; j++) {
                    int5 kc = {key0 + 64 * j, c, 0, 0, 0};
                    const float64 key = lanef + (float)(key0 + 64 * j);
                    const uint64 bit = v_u32_sel_less_u32_b(mkey(v_f32_ld_tnsr_b(kc, score)), th, (uint64)0, (uint64)(1u << j));
                    word |= as_uint64(v_f32_sel_less_f32_b(key, limf, as_float64(bit), as_float64((uint64)0)));
                }
            }
            int5 oc = {64 * b, c, 0, 0, 0};
            v_i32_st_tnsr(oc, bits, as_int64(word));
        }
    }
}
