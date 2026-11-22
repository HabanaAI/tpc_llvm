// SPDX-License-Identifier: Apache-2.0
// Reuse the exact group-32 scale/RNE approach of the V4.1 activation codec.
// The result is checkpoint E4M3FN bytes, not Gaudi's native hfloat8 format.
#ifdef DSV41_DECODED_KV_WRITE
#include "deepseek_v41_kv_decode.h"
#endif
#ifdef DSV41_RETURN_DECODED_KV
static inline bfloat128 swa_pack_number(float64 number, tensor output, int group,
#else
static inline void swa_pack_number(float64 number, tensor output, int group,
#endif
                                   int output_row, int width
#ifdef DSV41_DECODED_KV_WRITE
                                  , tensor decoded, int decoded_row
#endif
                                  ) {
    const float64 absolute = v_f32_abs_b(number);
    const uint64 bits = as_uint64(absolute);
    const float64 nan_lanes = v_f32_sel_grt_u32_b(bits, 0x7f800000, 1.0f, 0.0f);
    // Match the reference ordered amax: a NaN in the first group lane
    // persists, while subsequent NaNs do not replace a finite accumulator.
    const float64 first_nan = v_f32_sel_eq_u32_b(read_lane_id_4b_b(), 0, nan_lanes, 0.0f);
    const float64 any_nan = v_f32_reduce_max(first_nan);
    float64 maximum = v_f32_max_b(v_f32_reduce_max(v_f32_sel_grt_u32_b(as_uint64(absolute), 0x7f800000, 0.0f, absolute)), 1.0e-4f);
    maximum = v_f32_sel_grt_f32_b(any_nan, 0.0f, 1.0e-4f, maximum);
    const uint64 maximum_bits = as_uint64(maximum);
    const int64 exponent = convert_uint64_to_int64(maximum_bits >> 23, 0) - 135;
    const int64 adjustment = v_i32_sel_grt_u32_b(maximum_bits & 0x7fffff, 0x600000, 1, 0);
    const int64 scale_exponent = exponent + adjustment;
    const float64 reciprocal = as_float64((127 - scale_exponent) << 23);
    const float64 scaled = absolute * reciprocal;
    const uint64 rounded = bits + 0x7ffff + ((bits >> 20) & 1);
    int64 code = convert_uint64_to_int64(rounded >> 20, 0) - ((scale_exponent + 120) << 3);
    const int64 tiny = v_convert_f32_to_i32_b(scaled * 512.0f, 0, SW_RHNE);
    code = v_i32_sel_less_f32_b(scaled, 0.015625f, tiny, code);
    code = v_i32_min_b(v_i32_max_b(code, 0), 126);
    code = v_i32_sel_eq_u32_b(bits, 0x7f800000, 126, code);
    code = v_i32_sel_grt_u32_b(bits, 0x7f800000, 127, code);
    // Match the existing HPU log2/ceil chain for infinite maxima too:
    // exponent 120, scale byte 247, and saturating E4M3FN value code 126.
    const int64 scale_code = scale_exponent + 127;
    uint256 wide = {0};
    wide.v1 = as_uint64(code) | ((as_uint64(number) >> 24) & 128);
#ifdef DSV41_DECODED_KV_WRITE
#if defined(DSV41_OPTIONAL_DECODED_KV_WRITE) && !defined(DSV41_RETURN_DECODED_KV)
    if (decoded_row >= 0) {
#endif
    float128 decoded_value = {0};
    decoded_value.v1 = e4m3fn(wide.v1) * ue8m0(as_uint64(scale_code));
    const bfloat128 decoded_bf16 = convert_float128_to_bfloat128(
        decoded_value, SW_RHNE | SW_LINEAR);
#if defined(DSV41_OPTIONAL_DECODED_KV_WRITE) && defined(DSV41_RETURN_DECODED_KV)
    if (decoded_row >= 0)
#endif
    v_bf16_st_tnsr_partial(
        (int5){32 * group, decoded_row, 0, 0, 0}, decoded,
        decoded_bf16, 31, 0);
#if defined(DSV41_OPTIONAL_DECODED_KV_WRITE) && !defined(DSV41_RETURN_DECODED_KV)
    }
#endif
#endif
    const uchar256 encoded = convert_uint256_to_uchar256(wide, SW_LINEAR);
    v_u8_st_tnsr_partial((int5){32 * group, output_row, 0, 0, 0}, output, encoded, 31, 0);
    wide.v1 = as_uint64(scale_code);
    const uchar256 scales = convert_uint256_to_uchar256(wide, SW_LINEAR);
    v_u8_st_tnsr_partial((int5){width + group, output_row, 0, 0, 0}, output, scales, 0, 0);
#ifdef DSV41_RETURN_DECODED_KV
    return decoded_bf16;
#endif
}

static inline void swa_pack_group(tensor value, tensor output, int group,
                                  int input_row, int output_row
#ifdef DSV41_DECODED_KV_WRITE
                                  , tensor decoded, int decoded_row
#endif
                                  ) {
    const int width = get_dim_size(value, 0);
    const bfloat128 input = v_bf16_ld_tnsr_partial_b(
        (int5){32 * group, input_row, 0, 0, 0}, value, 31, 0);
    const float64 number =
        convert_bfloat128_to_float128(input, SW_LINEAR).v1;
    swa_pack_number(number, output, group, output_row, width
#ifdef DSV41_DECODED_KV_WRITE
                    , decoded, decoded_row
#endif
                    );
}
