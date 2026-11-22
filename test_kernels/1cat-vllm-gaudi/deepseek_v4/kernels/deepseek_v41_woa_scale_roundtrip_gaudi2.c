// SPDX-License-Identifier: Apache-2.0
// Fuse wo_a scaling/BF16 emission with the following group-32 activation
// round trip. The first BF16 conversion remains an explicit arithmetic
// boundary; only the intermediate SRAM write/read is removed.
void main(tensor product, tensor weight_scale, tensor activation_scale, tensor output) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int group = start[2]; group < end[2]; ++group) {
        for (int token = start[1]; token < end[1]; ++token) {
            const int5 sx_at = {0, token, group, 0, 0};
            const float sx = s_f32_ld_g(gen_addr(sx_at, activation_scale));
            for (int block = start[0]; block < end[0]; ++block) {
                const int5 p = {block * 32, token, group, 0, 0};
                const int5 sw = {block * 32, 0, group, 0, 0};
                const float64 scaled = v_f32_ld_tnsr_partial_b(p, product, 31, 0) *
                                       v_f32_ld_tnsr_partial_b(sw, weight_scale, 31, 0) * sx;

                float128 first_wide = {0};
                first_wide.v1 = scaled;
                const bfloat128 first_bf16 = convert_float128_to_bfloat128(first_wide, SW_RHNE | SW_LINEAR);
                const float64 value = convert_bfloat128_to_float128(first_bf16, SW_LINEAR).v1;
                const float64 absolute = v_f32_abs_b(value);
                const float64 nan_lanes = v_f32_sel_grt_u32_b(as_uint64(absolute), 0x7f800000, 1.0f, 0.0f);
                const float64 any_nan = v_f32_reduce_max(nan_lanes);
                float64 maximum = v_f32_max_b(v_f32_reduce_max(absolute), 1.0e-4f);
                maximum = v_f32_sel_grt_f32_b(any_nan, 0.0f, 1.0e-4f, maximum);
                const uint64 maximum_bits = as_uint64(maximum);
                const int64 exponent = convert_uint64_to_int64(maximum_bits >> 23, 0) - 135;
                const int64 adjustment = v_i32_sel_grt_u32_b(maximum_bits & 0x7fffff, 0x600000, 1, 0);
                const int64 scale_exponent = exponent + adjustment;
                const float64 scale = as_float64((scale_exponent + 127) << 23);
                const float64 reciprocal = as_float64((127 - scale_exponent) << 23);
                const float64 normalized = absolute * reciprocal;

                const uint64 bits = as_uint64(absolute);
                const uint64 rounded = (bits + 0x7ffff + ((bits >> 20) & 1)) & 0xfff00000;
                const int64 tiny_code = v_convert_f32_to_i32_b(normalized * 512.0f, 0, SW_RHNE);
                const float64 tiny = convert_int64_to_float64(tiny_code, 0) * (scale * 0.001953125f);
                float64 result = v_f32_sel_less_f32_b(normalized, 0.015625f, tiny, as_float64(rounded));
                result = v_f32_min_b(result, scale * 448.0f);
                result = as_float64(as_uint64(result) | (as_uint64(value) & 0x80000000));
                result = v_f32_sel_eq_f32_b(result, 0.0f, 0.0f, result);
                result = v_f32_sel_grt_u32_b(bits, 0x7f800000, as_float64((uint64)0x7fffffff), result);

                float128 final_wide = {0};
                final_wide.v1 = result;
                const bfloat128 converted = convert_float128_to_bfloat128(final_wide, SW_RHNE | SW_LINEAR);
                const int5 destination = {block * 32, group, token, 0, 0};
                v_bf16_st_tnsr_partial(destination, output, converted, 31, 0);
            }
        }
    }
}
