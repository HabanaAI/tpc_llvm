// SPDX-License-Identifier: Apache-2.0
// Checkpoint FP4 value encoding, with E4M3FN (g16) or UE8M0 (g32) scales.
#ifdef DSV41_DECODED_KV_WRITE
#include "deepseek_v41_kv_decode.h"
#endif
static inline int64 fp4_scale_e4m3(float64 value) {
    const float64 magnitude = v_f32_min_b(value, 448.0f);
    const uint64 bits = as_uint64(magnitude);
    const uint64 rounded = bits + 0x7ffff + ((bits >> 20) & 1);
    int64 code = convert_uint64_to_int64(rounded >> 20, 0) - 960;
    const int64 tiny = v_convert_f32_to_i32_b(magnitude * 512.0f, 0, SW_RHNE);
    code = v_i32_sel_less_f32_b(magnitude, 0.015625f, tiny, code);
    return v_i32_min_b(v_i32_max_b(code, 0), 126);
}
#ifdef DSV41_DECODED_KV_WRITE
static inline bfloat128 fp4_pack_number(float64 number, tensor output, int group,
                                       int output_row, int group_size, int width,
                                       tensor decoded, bool write_decoded, int decoded_row) {
    bfloat128 result = 0;
#else
static inline void fp4_pack_number(float64 number, tensor output, int group,
                                  int output_row, int group_size, int width) {
#endif
    const float64 magnitude = v_f32_abs_b(number);
    const float64 nan_lanes = v_f32_sel_grt_u32_b(as_uint64(magnitude), 0x7f800000, 1.0f, 0.0f);
    // Match the reference ordered amax: a NaN in the first group lane
    // persists, while subsequent NaNs do not replace a finite accumulator.
    const float64 first_nan = v_f32_sel_eq_u32_b(read_lane_id_4b_b(), 0, nan_lanes, 0.0f);
    const float64 any_nan = v_f32_reduce_max(first_nan);
    const float minimum = group_size == 16 ? 0.01171875f : 0x1.8p-124f;
    float64 maximum = v_f32_max_b(v_f32_reduce_max(v_f32_sel_grt_u32_b(as_uint64(magnitude), 0x7f800000, 0.0f, magnitude)), minimum);
    maximum = v_f32_sel_grt_f32_b(any_nan, 0.0f, minimum, maximum);
    int64 scale_code;
    float64 scale;
    if (group_size == 16) {
        scale_code = fp4_scale_e4m3(maximum * 0.16666666666666667f);
        const int64 exponent = (scale_code >> 3) & 15;
        const int64 mantissa = scale_code & 7;
        const float64 normal = as_float64(((exponent + 120) << 23) | (mantissa << 20));
        const float64 tiny = convert_int64_to_float64(mantissa, 0) * 0.001953125f;
        scale = v_f32_sel_eq_i32_b(exponent, 0, tiny, normal);
    } else {
        const uint64 bits = as_uint64(maximum);
        const int64 adjustment = v_i32_sel_grt_u32_b(bits & 0x7fffff, 0x400000, 1, 0);
        scale_code = convert_uint64_to_int64(bits >> 23, 0) - 2 + adjustment;
        scale = as_float64(scale_code << 23);
    }
    int64 code = v_i32_sel_grt_f32_b(magnitude, scale * 0.25f, 1, 0);
    code += v_i32_sel_geq_f32_b(magnitude, scale * 0.75f, 1, 0);
    code += v_i32_sel_grt_f32_b(magnitude, scale * 1.25f, 1, 0);
    code += v_i32_sel_geq_f32_b(magnitude, scale * 1.75f, 1, 0);
    code += v_i32_sel_grt_f32_b(magnitude, scale * 2.5f, 1, 0);
    code += v_i32_sel_geq_f32_b(magnitude, scale * 3.5f, 1, 0);
    code += v_i32_sel_grt_f32_b(magnitude, scale * 5.0f, 1, 0);
    code = v_i32_sel_eq_f32_b(magnitude, 0.0f, 0, code);
    uint256 wide = {0};
    wide.v1 = as_uint64(code) | ((as_uint64(number) >> 28) & 8);
#ifdef DSV41_DECODED_KV_WRITE
    if (write_decoded) {
        const uint64 magnitude_code = wide.v1 & 7;
        const float64 numeric_code = convert_uint64_to_float64(
            magnitude_code, 0);
        float64 decoded_value = v_f32_sel_less_u32_b(
            magnitude_code, 6, numeric_code - 2.0f,
            numeric_code * 2.0f - 8.0f);
        decoded_value = v_f32_sel_less_u32_b(
            magnitude_code, 4, numeric_code * 0.5f, decoded_value);
#ifndef DSV41_FP4_PRESERVE_ZERO_SIGN
        decoded_value = as_float64(as_uint64(decoded_value)
                                   | ((wide.v1 & 8) << 28));
#endif
        // Decoded cache writes currently request only the g16 result.  The
        // query roundtrip below also uses this helper for UE8M0 g32, where
        // ``scale`` is the exact power-of-two value represented by the scale
        // byte.  Keeping both paths here guarantees the encoder and decoded
        // result share the same midpoint decisions.
        decoded_value *= group_size == 16
            ? e4m3fn(as_uint64(scale_code)) : scale;
#ifdef DSV41_FP4_PRESERVE_ZERO_SIGN
        // TPC multiplication can canonicalize -0. Apply the FP4 sign after
        // scaling so code 0x8 is converted to BF16 negative zero exactly.
        decoded_value = as_float64(as_uint64(decoded_value)
                                   | ((wide.v1 & 8) << 28));
#else
#ifdef DSV41_FP4_INDEX_ZERO_SIGN
        if (group_size == 32)
            decoded_value = as_float64(as_uint64(decoded_value) | ((wide.v1 & 8) << 28));
        else
#endif
            decoded_value = v_f32_sel_eq_f32_b(decoded_value, 0.0f, 0.0f, decoded_value);
#endif
        float128 converted = {0};
        converted.v1 = decoded_value;
        const bfloat128 decoded_bf16 = convert_float128_to_bfloat128(
            converted, SW_RHNE | SW_LINEAR);
        result = decoded_bf16;
        if (decoded_row >= 0)
            v_bf16_st_tnsr_partial(
                (int5){group * group_size, decoded_row, 0, 0, 0}, decoded,
                decoded_bf16, group_size - 1, 0);
    }
#endif
#ifndef DSV41_FP4_DECODE_ONLY
    const uchar256 codes = convert_uint256_to_uchar256(wide, SW_LINEAR);
    // Adjacent byte codes already form little-endian 16-bit pairs. Compress
    // their nibbles before narrowing, avoiding a byte-shuffle routing table.
    const ushort128 pairs = as_ushort128(codes);
    ushort256 packed_words = {0};
    packed_words.v1 = (pairs & 15) | ((pairs >> 4) & 240);
    const uchar256 packed = convert_ushort256_to_uchar256(packed_words, SW_LINEAR);
    v_u8_st_tnsr_partial((int5){group * group_size / 2, output_row, 0, 0, 0},
                         output, packed, group_size / 2 - 1, 0);
    wide.v1 = as_uint64(scale_code);
    const uchar256 scales = convert_uint256_to_uchar256(wide, SW_LINEAR);
    v_u8_st_tnsr_partial((int5){width / 2 + group, output_row, 0, 0, 0}, output, scales, 0, 0);
#endif
#ifdef DSV41_DECODED_KV_WRITE
    return result;
#endif
}
static inline void fp4_pack_group(tensor value, tensor output, int group, int input_row,
                                 int output_row, int group_size
#ifdef DSV41_DECODED_KV_WRITE
                                 , tensor decoded, bool write_decoded, int decoded_row
#endif
                                 ) {
    const int width = get_dim_size(value, 0);
    const bfloat128 input = v_bf16_ld_tnsr_partial_b(
        (int5){group * group_size, input_row, 0, 0, 0}, value, group_size - 1, 0);
    const float64 number = convert_bfloat128_to_float128(input, SW_LINEAR).v1;
    fp4_pack_number(number, output, group, output_row, group_size, width
#ifdef DSV41_DECODED_KV_WRITE
                    , decoded, write_decoded, decoded_row
#endif
                    );
}
