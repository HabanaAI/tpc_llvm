// SPDX-License-Identifier: Apache-2.0
#pragma once
// Shared Gaudi2 row reduction and reciprocal-square-root primitives.
static inline float64 positive_rsqrt(float64 value) {
#ifdef FLASHINFER_NORM_USE_LOOKUP_RSQRT
    return v_rsqrt_f32(value);
#else
    float64 estimate = as_float64((int64)0x5f375a86 - (as_int64(value) >> 1));
    const float64 half_input = value * 0.5f;
    estimate = estimate * (1.5f - half_input * estimate * estimate);
    estimate = estimate * (1.5f - half_input * estimate * estimate);
    estimate = estimate * (1.5f - half_input * estimate * estimate);
    // Recover the squared-estimate rounding error before the final Newton
    // correction. A one-ULP rsqrt error can cross a BF16 tie and then an FP8 bin.
    const float64 square = estimate * estimate;
    const float64 square_error = v_f32_mac_b(estimate, estimate, -square);
    float64 correction = v_f32_mac_b(-value, square, 1.0f);
    correction = v_f32_mac_b(-value, square_error, correction);
    estimate = v_f32_mac_b(estimate * 0.5f, correction, estimate);
    return v_f32_sel_eq_f32_b(value, as_float64((int64)0x7f800000), 0.0f, estimate);
#endif
}

static inline float64 row_sum(float64 value) {
    value += v_f32_mov_dual_group_all_b(value, 0xffffffff, 1, 0, 3, 2, MkWrA(3, 3, 3, 3), 0);
    value += v_f32_mov_dual_group_all_b(value, 0xffffffff, 2, 3, 0, 1, MkWrA(3, 3, 3, 3), 0);
    value += v_f32_mov_group_b(value, 0xffffffff, 63, 0);
    const float64 a = v_f32_shuffle_b(value, (uchar256)0x80, 0, value) +
                      v_f32_shuffle_b(value, (uchar256)0x81, 0, value);
    const float64 b = v_f32_shuffle_b(value, (uchar256)0x82, 0, value) +
                      v_f32_shuffle_b(value, (uchar256)0x83, 0, value);
    const float64 c = v_f32_shuffle_b(value, (uchar256)0x84, 0, value) +
                      v_f32_shuffle_b(value, (uchar256)0x85, 0, value);
    const float64 d = v_f32_shuffle_b(value, (uchar256)0x86, 0, value) +
                      v_f32_shuffle_b(value, (uchar256)0x87, 0, value);
    return (a + b) + (c + d);
}
