// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DSpine code bridge, the messages: pk = [query | sender | value] (3R rows) of every column's code; the
// column before t in its block sends
//   msg[:, t] = sigmoid(q[:, t] + s[:, t - 1] + bias) * v[:, t - 1]     (t % block != 0), 0 on anchor columns
// in sigma order (the operand of the write GEMM).
// tensors: 0 pk (3R,T) f32  1 bias (R) f32 | 2 msg (R,T) bf16
// params: block. index space: [T]. R multiple of 128.
#include "common.h"
void main(tensor pk, tensor bias, tensor msg, int block)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int R = get_dim_size(msg, 0);
    for (int t = off[0]; t < end[0]; t++) {
        int5 c = {0, t, 0, 0, 0};
        if (t % block == 0) {
            for (int i = 0; i < R; i += 128) {
                c[0] = i;
                v_bf16_st_tnsr(c, msg, pack_sigma((float64)0.0f, (float64)0.0f));
            }
            continue;
        }
        int5 p = {0, t - 1, 0, 0, 0};
        int5 bc = {0, 0, 0, 0, 0};
        float64 m[2];
        for (int i = 0; i < R; i += 128) {
            #pragma unroll
            for (int h = 0; h < 2; h++) {
                const int r = i + 64 * h;
                c[0] = r;
                p[0] = R + r;
                bc[0] = r;
                const float64 g = v_f32_ld_tnsr_b(c, pk) + v_f32_ld_tnsr_b(p, pk) + v_f32_ld_tnsr_b(bc, bias);
                p[0] = 2 * R + r;
                m[h] = nl_sigm(g) * v_f32_ld_tnsr_b(p, pk);
            }
            c[0] = i;
            v_bf16_st_tnsr(c, msg, pack_sigma(m[0], m[1]));
        }
    }
}
