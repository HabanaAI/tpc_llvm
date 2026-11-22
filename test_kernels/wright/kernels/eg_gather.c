// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 Engram lookup, one rank's share: each token's J hashed rows of a row-sharded table, dequantized.
// Row r of the table is 256 FP8 (E4M3 bias 7) values times rs[r]. This rank holds global rows row0 .. row0 + R - 1;
// a hashed row outside them gives zeros, so the sum over the ranks is the full lookup.
//   out[256 j + i, t] = table[i, idx[j, t] - row0] * rs[idx[j, t] - row0]
// tensors: 0 table (256, R) fp8  1 rs (R) f32  2 idx (J, T) i32 | 3 out (256 J, T) f32
// params: row0. index space: [J, T]
#include "common.h"

void main(tensor table, tensor rs, tensor idx, tensor out, int row0)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int R = get_dim_size(table, 1);
    scalar_cache_reset();
    for (int t = off[1]; t < end[1]; t++) {
        for (int j = off[0]; j < end[0]; j++) {
            int5 ic = {j, t, 0, 0, 0};
            const int r = s_i32_ld_g(gen_addr(ic, idx)) - row0;
            float64 v[4] = {0.0f, 0.0f, 0.0f, 0.0f};
            if (r >= 0 && r < R) {
                int5 sc = {r, 0, 0, 0, 0};
                const float s = s_f32_ld_g(gen_addr(sc, rs));
                int5 tc = {0, r, 0, 0, 0};
                const float256 f = convert_minifloat256_to_float256(v_f8_ld_tnsr_b(tc, table), SW_LINEAR);
                v[0] = f.v1 * s;
                v[1] = f.v2 * s;
                v[2] = f.v3 * s;
                v[3] = f.v4 * s;
            }
            #pragma unroll
            for (int q = 0; q < 4; q++) {
                int5 oc = {256 * j + 64 * q, t, 0, 0, 0};
                v_f32_st_tnsr(oc, out, v[q]);
            }
        }
    }
}
