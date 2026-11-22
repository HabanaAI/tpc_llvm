// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DSA index query slots with FP8 queries (DeepSeek-V4.1): copy each slot's M query columns qcol0 .. qcol0 + M - 1
// (qcol0 = slots[1, s]) of the FP8 queries (128 bytes a column) and their scales. Slots of group -1 are skipped.
// tensors: 0 q (128, W) fp8  1 qs (W) f32  2 slots (8, S) i32 | 3 out (128, M, S) fp8  4 outs (M, S) f32
// index space: [1, ceil(M / 64), S] (64 columns of one slot per member)
#include "common.h"

void main(tensor q, tensor qs, tensor slots, tensor out, tensor outs)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int M = get_dim_size(out, 1);
    scalar_cache_reset();
    for (int s = off[2]; s < end[2]; s++) {
        int5 fc = {0, s, 0, 0, 0};
        if (s_i32_ld_g(gen_addr(fc, slots)) < 0)
            continue;
        fc[0] = 1;
        const int qcol0 = s_i32_ld_g(gen_addr(fc, slots));
        for (int j = off[1]; j < end[1]; j++) {
            const int m0 = 64 * j, n = M - m0 < 64 ? M - m0 : 64;
            for (int m = 0; m < n; m++) {
                int5 ic = {0, qcol0 + m0 + m, 0, 0, 0};
                int5 oc = {0, m0 + m, s, 0, 0};
                v_f8_st_tnsr_partial(oc, out, v_f8_ld_tnsr_b(ic, q), 127, 0);
            }
            int5 sc = {qcol0 + m0, 0, 0, 0, 0};
            int5 so = {m0, s, 0, 0, 0};
            v_f32_st_tnsr_partial(so, outs, v_f32_ld_tnsr_b(sc, qs), n - 1, 0);
        }
    }
}
