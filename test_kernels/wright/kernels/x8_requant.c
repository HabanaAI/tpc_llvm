// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Routed-expert weights, load time: block FP8 (fp8_inc, block scales) -> per-output-row FP8,
// input-major (M lanes per input row, as x8_copy's slots). Output row m gets
// c[m] = max_k |w[m, k] bs[m / 128, k / 128]| / 240 and w'[m, k] = fp8(w[m, k] bs / c[m]), natural lane order.
// K's order within a 128-row block is pair order, so kb = k / 128 still names the scale.
// tensors: 0 w (M, K, E) fp8  1 bs (M/128, K/128, E) f32 | 2 out (M, K, E) fp8  3 c (M, E) f32
// index space: [M/256, E]. M % 256 == 0, K % 128 == 0.
#include "common.h"

void main(tensor w, tensor bs, tensor out, tensor c)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    const int K = get_dim_size(w, 1);
    scalar_cache_reset();
    for (int e = off[1]; e < end[1]; e++)
        for (int m = off[0]; m < end[0]; m++) {
            float64 a0 = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;
            for (int kb = 0; kb < K >> 7; kb++) {
                int5 sc = {2 * m, kb, e, 0, 0};
                const float s0 = s_f32_ld_g(gen_addr(sc, bs));
                sc[0]++;
                const float s1 = s_f32_ld_g(gen_addr(sc, bs));
                int5 wc = {256 * m, 128 * kb, e, 0, 0};
                #pragma unroll 4
                for (int k = 0; k < 128; k++) {
                    const float256 f = convert_minifloat256_to_float256(v_f8_ld_tnsr_b(wc, w), SW_LINEAR);
                    a0 = v_f32_max_b(a0, v_f32_abs_b(f.v1 * s0));
                    a1 = v_f32_max_b(a1, v_f32_abs_b(f.v2 * s0));
                    a2 = v_f32_max_b(a2, v_f32_abs_b(f.v3 * s1));
                    a3 = v_f32_max_b(a3, v_f32_abs_b(f.v4 * s1));
                    wc[1]++;
                }
            }
            const float64 r0 = v_f32_max_b(a0, 1e-30f) * (1.0f / FP8_MAX), r1 = v_f32_max_b(a1, 1e-30f) * (1.0f / FP8_MAX);
            const float64 r2 = v_f32_max_b(a2, 1e-30f) * (1.0f / FP8_MAX), r3 = v_f32_max_b(a3, 1e-30f) * (1.0f / FP8_MAX);
            const float64 i0 = nl_recip(r0), i1 = nl_recip(r1), i2 = nl_recip(r2), i3 = nl_recip(r3);
            for (int kb = 0; kb < K >> 7; kb++) {
                int5 sc = {2 * m, kb, e, 0, 0};
                const float s0 = s_f32_ld_g(gen_addr(sc, bs));
                sc[0]++;
                const float s1 = s_f32_ld_g(gen_addr(sc, bs));
                int5 wc = {256 * m, 128 * kb, e, 0, 0};
                #pragma unroll 4
                for (int k = 0; k < 128; k++) {
                    const float256 f = convert_minifloat256_to_float256(v_f8_ld_tnsr_b(wc, w), SW_LINEAR);
                    v_f8_st_tnsr(wc, out, pack_lin(f.v1 * s0 * i0, f.v2 * s0 * i1, f.v3 * s1 * i2, f.v4 * s1 * i3));
                    wc[1]++;
                }
            }
            int5 cc = {256 * m, e, 0, 0, 0};
            v_f32_st_tnsr(cc, c, r0);
            cc[0] += 64;
            v_f32_st_tnsr(cc, c, r1);
            cc[0] += 64;
            v_f32_st_tnsr(cc, c, r2);
            cc[0] += 64;
            v_f32_st_tnsr(cc, c, r3);
        }
}
