// SPDX-License-Identifier: Apache-2.0
#pragma clang fp contract(off)
#define DSV4_QNORM_HELPERS_ONLY 1
#include "deepseek_v4_qnorm_rope_kv_pack_bf16.h"

static inline float64 swap_pair_dimensions(float64 x) {
    const uint64 lane = V_LANE_ID_32;
    const uchar256 directions = dsv4_qkv_shuffle_directions(
        (lane & 7) + ((((lane >> 3) ^ 1) & 1) << 5) + 0x80);
    return v_f32_shuffle_b(x, directions, 0, 0.0f);
}

static inline float64 interleaved_rope(float64 x, float64 phase, int first_pair, bool inverse) {
    const int64 bits = as_int64(phase);
    // TPC source-group selectors are immediate operands, not scalar registers.
    int64 c, s;
    if (first_pair < 16) {
        c = v_i32_mov_dual_group_all_b(bits, 0xffffffff, 0, 0, 0, 0, MkWrA(3, 3, 3, 3), bits);
        s = v_i32_mov_dual_group_all_b(bits, 0xffffffff, 2, 2, 2, 2, MkWrA(3, 3, 3, 3), bits);
    } else {
        c = v_i32_mov_dual_group_all_b(bits, 0xffffffff, 1, 1, 1, 1, MkWrA(3, 3, 3, 3), bits);
        s = v_i32_mov_dual_group_all_b(bits, 0xffffffff, 3, 3, 3, 3, MkWrA(3, 3, 3, 3), bits);
    }
    const uint64 lane = V_LANE_ID_32;
    const uint64 pair = (lane >> 4) + (first_pair & 15);
    const uchar256 directions = dsv4_qkv_shuffle_directions((pair & 7) + ((pair & 8) << 2) + 0x80);
    const float64 cosine = v_f32_shuffle_b(as_float64(c), directions, 0, 0.0f);
    float64 sine = v_f32_shuffle_b(as_float64(s), directions, 0, 0.0f);
    if (inverse) sine = -sine;
    const float64 other = swap_pair_dimensions(x);
    const float64 real = v_f32_mac_b(other, sine, x * cosine, SW_NEG);
    const float64 imag = v_f32_mac_b(x, cosine, other * sine, 0);
    return v_f32_sel_eq_u32_b(lane & 8, 8, imag, real);
}

static inline float64 head_maximum(float64 x) {
    x = v_f32_max_b(x, swap_pair_dimensions(x));
    int64 bits = as_int64(x);
    int64 swap = v_i32_mov_dual_group_all_b(bits, 0xffffffff, 1, 0, 3, 2, MkWrA(3, 3, 3, 3), bits);
    x = v_f32_max_b(x, as_float64(swap));
    bits = as_int64(x);
    swap = v_i32_mov_dual_group_all_b(bits, 0xffffffff, 2, 3, 0, 1, MkWrA(3, 3, 3, 3), bits);
    return v_f32_max_b(x, as_float64(swap));
}

static inline float64 group32_roundtrip(float64 value, float64 maximum) {
    const uint64 maximum_bits = as_uint64(maximum);
    const int64 exponent = convert_uint64_to_int64(maximum_bits >> 23, 0) - 135;
    const int64 adjustment = v_i32_sel_grt_u32_b(maximum_bits & 0x7fffff, 0x600000, 1, 0);
    const int64 scale_exponent = exponent + adjustment;
    const float64 scale = as_float64((scale_exponent + 127) << 23);
    const float64 reciprocal = as_float64((127 - scale_exponent) << 23);
    const float64 absolute = v_f32_abs_b(value);
    const float64 scaled = absolute * reciprocal;
    const uint64 bits = as_uint64(absolute);
    const uint64 rounded = (bits + 0x7ffff + ((bits >> 20) & 1)) & 0xfff00000;
    const int64 tiny_code = v_convert_f32_to_i32_b(scaled * 512.0f, 0, SW_RHNE);
    const float64 tiny = convert_int64_to_float64(tiny_code, 0) * (scale * 0.001953125f);
    float64 result = v_f32_sel_less_f32_b(scaled, 0.015625f, tiny, as_float64(rounded));
    result = v_f32_min_b(result, scale * 448.0f);
    result = as_float64(as_uint64(result) | (as_uint64(value) & 0x80000000));
    result = v_f32_sel_eq_f32_b(result, 0.0f, 0.0f, result);
    const uint64 nan_bits = 0x7fffffff;
    return v_f32_sel_grt_u32_b(bits, 0x7f800000, as_float64(nan_bits), result);
}
