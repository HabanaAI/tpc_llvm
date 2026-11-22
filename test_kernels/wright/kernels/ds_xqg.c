// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Prefill pages: each pooled row r takes token gidx[r]'s per-token FP8 quantization (pair order, as ds_xq):
// q[:, r] = fp8(x[:, t] / s_t), sx[r] = s_t; rows of no token (gidx >= T) are zero, scale 0: ds_xact skips them.
// tensors: 0 x (H, T) f32  1 gidx (S,) i32 | 2 q (H, S) fp8  3 sx (1, S) f32.  index space: [S / 4]
#include "common.h"

void main(tensor x, tensor gidx, tensor q, tensor sx)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int H = get_dim_size(x, 0), T = get_dim_size(x, 1), S = get_dim_size(gidx, 0);
    DECL_IDX(IDX)
    scalar_cache_reset();
    for (int b = off[0]; b < end[0]; b++)
        for (int r = 4 * b; r < 4 * b + 4 && r < S; r++) {
            int5 gc = {r, 0, 0, 0, 0};
            const int t = s_i32_ld_g(gen_addr(gc, gidx));
            int5 qc = {0, r, 0, 0, 0};
            if (t < 0 || t >= T) {
                for (int i = 0; i < H; i += 128) { qc[0] = i; v_f8_st_tnsr_partial(qc, q, (minifloat256)0, 127, 0); }
                STORE_TOKEN_SCALE(sx, r, (float64)0.0f)
                continue;
            }
            int5 c = {0, t, 0, 0, 0};
            float64 a0 = 0.0f, a1 = 0.0f;
            for (int i = 0; i < H; i += 128) {
                c[0] = i;      a0 = v_f32_max_b(a0, v_f32_abs_b(v_f32_ld_tnsr_b(c, x)));
                c[0] = i + 64; a1 = v_f32_max_b(a1, v_f32_abs_b(v_f32_ld_tnsr_b(c, x)));
            }
            float64 amax;
            NL_REDUCE_MAX(v_f32_max_b(a0, a1), IDX, amax);
            const float64 sc = v_f32_max_b(amax, 1e-30f) * (1.0f / FP8_MAX);
            const float64 inv = nl_recip(sc);
            for (int i = 0; i < H; i += 128) {
                c[0] = i;       const float64 va = v_f32_ld_tnsr_b(c, x) * inv;
                c[0] = i + 64;  const float64 vb = v_f32_ld_tnsr_b(c, x) * inv;
                qc[0] = i;
                v_f8_st_tnsr_partial(qc, q, pack_nat128(va, vb), 127, 0);
            }
            STORE_TOKEN_SCALE(sx, r, sc)
        }
}
