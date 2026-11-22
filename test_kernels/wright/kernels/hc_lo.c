// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Hyper-connection low-rank activation: out = bf16(silu(lo / NS)) in sigma order (feeds the up GEMM,
// whose K rows >= 320 are zero, so the inject rows 320..323 of lo contribute nothing).
// tensors: 0 lo (C,T) f32 | 1 out (C,T) bf16.  index space: [C/128, T]
#include "common.h"

void main(tensor lo, tensor out)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    for (int t = off[1]; t < end[1]; t++) {
        for (int blk = off[0]; blk < end[0]; blk++) {
            int5 c = {blk * 128, t, 0, 0, 0};
            const float64 a = v_f32_ld_tnsr_b(c, lo) * 0.25f;
            c[0] += 64;
            const float64 b = v_f32_ld_tnsr_b(c, lo) * 0.25f;
            c[0] -= 64;
            v_bf16_st_tnsr(c, out, pack_sigma(nl_silus(a), nl_silus(b)));
        }
    }
}
