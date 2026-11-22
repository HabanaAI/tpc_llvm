// SPDX-License-Identifier: Apache-2.0
// Fuse the checkpoint group-32 E4M3FN activation round trip. MME stays BF16.
#ifdef DSV41_FIXED_FP8_QUANT
void main(tensor input, tensor output, float reciprocal_fixed) {
#else
void main(tensor input, tensor output) {
#endif
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    for (int row = begin[1]; row < end[1]; ++row) {
        for (int group = begin[0]; group < end[0]; ++group) {
            const int5 at = {group * 32, row, 0, 0, 0};
            const bfloat128 packed = v_bf16_ld_tnsr_partial_b(at, input, 31, 0);
            const float64 value = convert_bfloat128_to_float128(packed, SW_LINEAR).v1;
            const float64 absolute = v_f32_abs_b(value);
            const float64 nan_lanes = v_f32_sel_grt_u32_b(as_uint64(absolute), 0x7f800000, 1.0f, 0.0f);
            const float64 any_nan = v_f32_reduce_max(nan_lanes);
            float64 maximum = v_f32_max_b(v_f32_reduce_max(absolute), 1.0e-4f);
            maximum = v_f32_sel_grt_f32_b(any_nan, 0.0f, 1.0e-4f, maximum);
            const uint64 maximum_bits = as_uint64(maximum);
            const int64 exponent = convert_uint64_to_int64(maximum_bits >> 23, 0) - 135;
            const int64 adjustment = v_i32_sel_grt_u32_b(maximum_bits & 0x7fffff, 0x600000, 1, 0);
            const int64 scale_exponent = exponent + adjustment;
            const int64 scale_bits = (scale_exponent + 127) << 23;
            const int64 reciprocal_bits = (127 - scale_exponent) << 23;
            const float64 scale = as_float64(scale_bits);
            const float64 reciprocal = as_float64(reciprocal_bits);
            const float64 scaled = absolute * reciprocal;

            // A BF16 input is already exact in FP32. RNE to three fraction bits
            // can therefore operate on the original exponent/mantissa bits.
            const uint64 bits = as_uint64(absolute);
            const uint64 rounded = (bits + 0x7ffff + ((bits >> 20) & 1)) & 0xfff00000;
            const int64 tiny_code = v_convert_f32_to_i32_b(scaled * 512.0f, 0, SW_RHNE);
            const float64 tiny = convert_int64_to_float64(tiny_code, 0) * (scale * 0.001953125f);
            float64 result = v_f32_sel_less_f32_b(scaled, 0.015625f, tiny, as_float64(rounded));
            result = v_f32_min_b(result, scale * 448.0f);
            const uint64 signed_bits = as_uint64(result) | (as_uint64(value) & 0x80000000);
            result = as_float64(signed_bits);
            // Match the existing HPU codec's canonical zero and NaN outputs.
            result = v_f32_sel_eq_f32_b(result, 0.0f, 0.0f, result);
            const uint64 nan_bits = 0x7fffffff;
            result = v_f32_sel_grt_u32_b(bits, 0x7f800000, as_float64(nan_bits), result);
            float128 wide = {0};
            wide.v1 = result;
            const bfloat128 converted = convert_float128_to_bfloat128(wide, SW_RHNE | SW_LINEAR);
#ifdef DSV41_FIXED_FP8_QUANT
            // Preserve the original BF16 round-trip boundary, then encode
            // the fixed-bias operand without a second read or row amax.
            const bfloat128 scaled_fixed = converted * (bfloat128)reciprocal_fixed;
            minifloat256 encoded = v_convert_bf16_to_f8_b(
                scaled_fixed, 0, SW_RHNE | SW_CLIP_FP, (minifloat256)0);
            const minifloat256 sparse = encoded;
            encoded = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            encoded = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, encoded);
            encoded = v_f8_mov_dual_group_pack_b(encoded, SW_PACK21, (minifloat256)0);
            v_f8_st_tnsr_partial(at, output, encoded, 31, 0);
#else
            v_bf16_st_tnsr_partial(at, output, converted, 31, 0);
#endif
        }
    }
}
