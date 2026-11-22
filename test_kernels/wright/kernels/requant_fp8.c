// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Block-FP8 weight -> per-row FP8 (load time).
// Input bytes are e4m3 weights in tau order within each 256-column block (byte s of a block holds
// source column 64*(s%4) + s/4). The output keeps the byte positions.
// tensors: 0 w (K, M) fp8   1 bs (K/128, M/128) f32 block scales (w = e4m3 * bs)
//        | 2 out (K, M) fp8 (w = e4m3 * rs[m])   3 rs (1, M) f32
// index space: [M]
#include "common.h"

void main(tensor w, tensor bs, tensor out, tensor rs)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int K = get_dim_size(w, 0);
    DECL_IDX(IDX)
    for (int m = off[0]; m < end[0]; m++) {
        const int mb = m >> 7;
        float64 amax = 0.0f;
        for (int k = 0; k < K; k += 256) {
            int5 c = {k, m, 0, 0, 0};
            const float256 f = convert_minifloat256_to_float256(v_f8_ld_tnsr_b(c, w), 0);
            int5 sc = {k >> 7, mb, 0, 0, 0};
            const float64 sv = BCAST_DG(v_f32_ld_tnsr_b(sc, bs), 0);
            const float64 s0 = v_f32_shuffle_b(sv, IDX[0], 0, sv);
            const float64 s1 = v_f32_shuffle_b(sv, IDX[1], 0, sv);
            amax = v_f32_max_b(amax, v_f32_abs_b(f.v1 * s0));
            amax = v_f32_max_b(amax, v_f32_abs_b(f.v2 * s0));
            amax = v_f32_max_b(amax, v_f32_abs_b(f.v3 * s1));
            amax = v_f32_max_b(amax, v_f32_abs_b(f.v4 * s1));
        }
        NL_REDUCE_MAX(amax, IDX, amax);
        const float64 r = v_f32_max_b(amax, 1e-30f) * (1.0f / 240.0f);     // row scale
        const float64 inv = nl_recip(r);
        for (int k = 0; k < K; k += 256) {
            int5 c = {k, m, 0, 0, 0};
            const float256 f = convert_minifloat256_to_float256(v_f8_ld_tnsr_b(c, w), 0);
            int5 sc = {k >> 7, mb, 0, 0, 0};
            const float64 sv = BCAST_DG(v_f32_ld_tnsr_b(sc, bs), 0);
            const float64 s0 = v_f32_shuffle_b(sv, IDX[0], 0, sv) * inv;
            const float64 s1 = v_f32_shuffle_b(sv, IDX[1], 0, sv) * inv;
            v_f8_st_tnsr(c, out, pack_tau(f.v1 * s0, f.v2 * s0, f.v3 * s1, f.v4 * s1));
        }
        int5 rc = {0, m, 0, 0, 0};
        v_f32_st_tnsr(rc, rs, r);
    }
}
