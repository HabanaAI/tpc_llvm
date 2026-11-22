// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Slot attention: reset the accumulators of attn_slot_acc before a chunk's first slot launch
// (output 0, max NINF, sum 0).
// tensors: 0 acc_o (256, W, G) f32  1 acc_m (W, G) f32  2 acc_l (W, G) f32 | 3 sink (64, 1) i32
// index space: [ceil(W / 64), G]; member (0, 0) writes the sink.
#include "common.h"

#define NINF (-3.0e38f)

void main(tensor acc_o, tensor acc_m, tensor acc_l, tensor sink)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int W = get_dim_size(acc_o, 1);
    const float64 zero = 0.0f, ninf = NINF;
    for (int g = off[1]; g < end[1]; g++) {
        for (int cb = off[0]; cb < end[0]; cb++) {
            int5 ac = {cb * 64, g, 0, 0, 0};
            v_f32_st_tnsr(ac, acc_m, ninf);
            v_f32_st_tnsr(ac, acc_l, zero);
            const int c1 = cb * 64 + 64 < W ? cb * 64 + 64 : W;
            int5 oc = {0, cb * 64, g, 0, 0};
            for (int c = cb * 64; c < c1; c++, oc[1]++) {
                #pragma unroll
                for (int d = 0; d < 256; d += 64) {
                    oc[0] = d;
                    v_f32_st_tnsr(oc, acc_o, zero);
                }
            }
        }
    }
    if (off[0] == 0 && off[1] == 0) {
        int5 zc = {0, 0, 0, 0, 0};
        v_i32_st_tnsr(zc, sink, (int64)1);
    }
}
