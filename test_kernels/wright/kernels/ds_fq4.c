// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// DeepSeek-V4.1 FP4 rounding in place of a cache store (values, not codes): each group of G consecutive elements of a
// column gets a scale s from its largest magnitude a, and each element becomes s * e2m1(x / s), the nearest of
// 0, .5, 1, 1.5, 2, 3, 4, 6 (ties to the even code), clamped to +-6.
//   mode 0: G = 32, s = 2^ceil(log2(max(a, 6 2^-126) / 6))  (index keys and queries)
//   mode 1: G = 16, s = OCP E4M3 of max(a, 6 2^-9) / 6 (nearest even, <= 448)  (compressed KV)
// tensors: 0 x (D, T) f32 | 1 y (D, T) f32. params: mode. index space: [D / 64, T]
#include "common.h"

// s * e2m1(x / s) by comparing |x| with the midpoints times s (exact: both have few significant bits)
static inline float64 e2m1s(float64 v, float64 s)
{
    const float64 a = v_f32_abs_b(v);
    float64 q = 6.0f;
    q = v_f32_sel_leq_f32_b(a, 5.0f * s, 4.0f, q);
    q = v_f32_sel_less_f32_b(a, 3.5f * s, 3.0f, q);
    q = v_f32_sel_leq_f32_b(a, 2.5f * s, 2.0f, q);
    q = v_f32_sel_less_f32_b(a, 1.75f * s, 1.5f, q);
    q = v_f32_sel_leq_f32_b(a, 1.25f * s, 1.0f, q);
    q = v_f32_sel_less_f32_b(a, 0.75f * s, 0.5f, q);
    q = v_f32_sel_leq_f32_b(a, 0.25f * s, 0.0f, q);
    return v_f32_sel_less_f32_b(v, 0.0f, -q, q) * s;
}

// round x > 0 to 3 mantissa bits, nearest even, exponent >= -6 (OCP E4M3 without its NaN), at most 448
static inline float64 e4m3(float64 x)
{
    const int64 bits = as_int64(x);
    int64 e = ((bits >> 23) & 255) - 127;
    e = v_i32_max_b(e, (int64)-6);
    const float64 quantum = as_float64((e - 3 + 127) << 23);
    const float64 q = v_convert_i32_to_f32_b(v_convert_f32_to_i32_b(x * as_float64((127 - e + 3) << 23), SW_RHNE, 0, 1, 0),
                                             SW_RHNE, 0, 1, 0);
    return v_f32_min_b(q * quantum, 448.0f);
}

void main(tensor x, tensor y, int mode)
{
    const int5 off = get_index_space_offset();
    const int5 end = off + get_index_space_size();
    DECL_IDX(IDX)
    for (int t = off[1]; t < end[1]; t++) {
        for (int b = off[0]; b < end[0]; b++) {
            int5 c = {64 * b, t, 0, 0, 0};
            const float64 v = v_f32_ld_tnsr_b(c, x);
            const float64 a = v_f32_abs_b(v);
            float64 r0 = v_f32_shuffle_b(a, IDX[0], 0, a), r1 = v_f32_shuffle_b(a, IDX[1], 0, a);
            float64 r2 = v_f32_shuffle_b(a, IDX[2], 0, a), r3 = v_f32_shuffle_b(a, IDX[3], 0, a);
            #pragma unroll
            for (int e = 4; e < 16; e += 4) {
                r0 = v_f32_max_b(r0, v_f32_shuffle_b(a, IDX[e], 0, a));
                r1 = v_f32_max_b(r1, v_f32_shuffle_b(a, IDX[e + 1], 0, a));
                r2 = v_f32_max_b(r2, v_f32_shuffle_b(a, IDX[e + 2], 0, a));
                r3 = v_f32_max_b(r3, v_f32_shuffle_b(a, IDX[e + 3], 0, a));
            }
            float64 m = v_f32_max_b(v_f32_max_b(r0, r1), v_f32_max_b(r2, r3));  // max over a 16-lane group
            float64 s;
            if (mode == 0) {
                m = v_f32_max_b(m, v_f32_mov_dual_group_all_b(m, 0xFFFFFFFF, 1, 0, 3, 2, WR_ALL, m));  // 32 lanes
                m = v_f32_max_b(m, 6.0f * 1.1754944e-38f);
                const float64 r = m * (1.0f / 6.0f);
                s = as_float64((as_int64(r) + 0x007FFFFF) & 0x7F800000);
                s = v_f32_sel_grt_f32_b(m, s * 6.0f, s * 2.0f, s);
                s = v_f32_sel_leq_f32_b(m, s * 3.0f, s * 0.5f, s);
            } else {
                m = v_f32_max_b(m, 6.0f * 0.001953125f);
                s = e4m3(m * (1.0f / 6.0f));
            }
            v_f32_st_tnsr(c, y, e2m1s(v, s));
        }
    }
}
