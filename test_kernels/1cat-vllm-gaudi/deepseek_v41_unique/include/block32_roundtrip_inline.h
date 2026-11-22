// SPDX-License-Identifier: Apache-2.0
// Exact block32 arithmetic extracted from the maintained BF16 roundtrip.
// Canonical source sha256: b680bd9d65eb1ea17f4d65c3f737b4067162b5d298efe6aac927bc6ac13f7d78
static inline float64 paired_group_max(float64 value) {
    const bool64 low=v_i32_cmp_less_b((int64)V_LANE_ID_32,32);
    const float64 left=v_f32_reduce_max(v_f32_mov_vb(value,0,(float64)0,low,0));
    const float64 right=v_f32_reduce_max(v_f32_mov_vb(value,0,(float64)0,~low,0));
    return v_f32_mov_vb(left,0,right,low,0);
}
static inline float64 paired_block32_roundtrip(float64 value) {
            const float64 absolute = v_f32_abs_b(value);
            const float64 nan_lanes = v_f32_sel_grt_u32_b(as_uint64(absolute), 0x7f800000, 1.0f, 0.0f);
            const float64 any_nan = paired_group_max(nan_lanes);
            float64 maximum = v_f32_max_b(paired_group_max(absolute), 1.0e-4f);
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
    return result;
}
