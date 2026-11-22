// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Prefill pages: each pooled row r takes token gidx[r]'s FP8 row and scale from the per-token quantization
// (ds_xq_lin with one copy per token): out[:, r] = q[:, t], sx[r] = s[t]. Rows of no token (gidx outside
// [0, T)) get scale 0 (ds_xact skips them) and their FP8 row is not written.
// tensors: 0 q (H, T) fp8  1 s (1, T) f32  2 gidx (S,) i32 | 3 out (H, S) fp8  4 sx (1, S) f32
// index space: [S / 4]
#include "common.h"

void main(tensor q, tensor s, tensor gidx, tensor out, tensor sx)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(q, 0), T = get_dim_size(q, 1), S = get_dim_size(gidx, 0);
    scalar_cache_reset();
    for (int b = off[0]; b < end[0]; b++) {
        // four rows at once; rows of no token are not stored
        int tk[4];
        bool ok[4];
        #pragma unroll
        for (int k = 0; k < 4; k++) {
            const int r = 4 * b + k;
            int5 gc = {r < S ? r : S - 1, 0, 0, 0, 0};
            const int t = s_i32_ld_g(gen_addr(gc, gidx));
            ok[k] = r < S && t >= 0 && t < T;
            tk[k] = ok[k] ? t : 0;
            int5 rc = {0, r, 0, 0, 0}, tc = {0, tk[k], 0, 0, 0};
            if (r < S)
                v_f32_st_tnsr(rc, sx, ok[k] ? v_f32_ld_tnsr_b(tc, s) : (float64)0.0f);
        }
        for (int i = 0; i < H; i += 256) {
            uchar256 v[4];
            #pragma unroll
            for (int k = 0; k < 4; k++) {
                int5 tc = {i, tk[k], 0, 0, 0};
                v[k] = v_u8_ld_tnsr_b(tc, q);
            }
            #pragma unroll
            for (int k = 0; k < 4; k++) {
                int5 rc = {i, 4 * b + k, 0, 0, 0};
                v_u8_st_tnsr(rc, out, v[k], 0, ok[k], 0);
            }
        }
    }
}
