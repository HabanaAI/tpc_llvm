// SPDX-License-Identifier: Apache-2.0
// Fuse the production FFN RMSNorm BF16 boundary with the exact activation
// quantizer consumed by the N256 expert graph.  One index-space point owns a
// complete row; the rounded normalized row remains in VLM for quantization.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"

static inline float64 round_bf16(float64 value) {
    float64_pair_t pair;
    pair.v1 = value;
    pair.v2 = value;
    return v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(pair)).v1;
}

static inline float64 reciprocal_without_lookup(float64 value) {
    float64 estimate = as_float64((int64)0x7ef311c3 - as_int64(value));
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    return v_f32_sel_eq_f32_b(value, as_float64((int64)0x7f800000), 0.0f, estimate);
}

static inline float64 row_max_without_lookup(float64 value) {
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(
        value, 0xffffffff, 1, 0, 3, 2, MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(
        value, 0xffffffff, 2, 3, 0, 1, MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_group_b(value, 0xffffffff, 63, 0));
    float64 result = 0;
    #pragma loop_unroll(8)
    for (int lane = 0; lane < 8; ++lane) {
        result = v_f32_max_b(result,
            v_f32_shuffle_b(value, (uchar256)(0x80 | lane), 0, value));
    }
    return result;
}

void main(tensor input, tensor weight, tensor normalized, tensor quantized,
          tensor scales,
#ifdef DSV41_FFN_DUAL_QUANT
          tensor dense_quantized, tensor dense_scales,
#endif
          float epsilon, float inverse_width) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int tiles = get_dim_size(input, 0) / 128;
    bfloat128 cached[40];

    for (int row = begin[0]; row < end[0]; ++row) {
        float128 squares = {0};
        for (int tile = 0; tile < tiles; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
            const bfloat128 value = v_bf16_ld_tnsr_b(at, input);
            cached[tile] = value;
            squares = v_bf16_mac_acc32_b(value, value, squares,
                                         (e_no_negation) << 1);
        }
        const float64 rrms = positive_rsqrt(
            row_sum(squares.v1 + squares.v2) * inverse_width + epsilon);

        float64 maximum = 0;
        for (int tile = 0; tile < tiles; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
            const int5 wt = {tile * 128, 0, 0, 0, 0};
            const float128 w = v_convert_bf16_to_f32_all_b(
                v_bf16_ld_tnsr_b(wt, weight));
            float128 value = v_convert_bf16_to_f32_all_b(cached[tile]);
            value.v1 = (value.v1 * rrms) * w.v1;
            value.v2 = (value.v2 * rrms) * w.v2;
            const bfloat128 rounded = v_convert_f32_to_bf16_all_b(value);
            cached[tile] = rounded;
            v_bf16_st_tnsr(at, normalized, rounded);
            const float128 expanded = v_convert_bf16_to_f32_all_b(rounded);
            maximum = v_f32_max_b(maximum, v_f32_abs_b(expanded.v1));
            maximum = v_f32_max_b(maximum, v_f32_abs_b(expanded.v2));
        }

        maximum = row_max_without_lookup(maximum);
        const float64 raw_scale = round_bf16(
            maximum * (float)(bf16)(1.0f / 240.0f));
        const float64 scale = round_bf16(
            raw_scale + (float)(bf16)(1.0e-8f / 240.0f));
        const float64 inverse = round_bf16(reciprocal_without_lookup(scale));
#ifdef DSV41_FFN_BF16_QUANT
        const float128 inverse_pair = {inverse, inverse};
        const bfloat128 sparse_inverse_bf16 = v_convert_f32_to_bf16_all_b(inverse_pair);
#endif
        const int5 scale_at = {0, row, 0, 0, 0};
        v_f32_st_tnsr(scale_at, scales, scale);
#ifdef DSV41_FFN_DUAL_QUANT
        // The shared expert uses a power-of-two scale and flushes FP8
        // subnormals. Reuse the row amax, but preserve both quantizers.
        const uint64 bits = as_uint64(maximum);
        int64 power = convert_uint64_to_int64(bits >> 23, 0) - 134;
        power += v_i32_sel_grt_u32_b(bits & 0x7fffff, 0x700000, 1, 0);
        power = v_i32_sel_eq_f32_b(maximum, 0.0f, 0, power);
        const float64 dense_scale = as_float64((power + 127) << 23);
        const float64 dense_inverse = as_float64((127 - power) << 23);
        const float128 dense_inverse_pair = {dense_inverse, dense_inverse};
        const bfloat128 dense_inverse_bf16 = v_convert_f32_to_bf16_all_b(dense_inverse_pair);
        v_f32_st_tnsr_partial(scale_at, dense_scales, dense_scale, 0, 0);
#endif

        for (int tile = 0; tile < tiles; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
#ifdef DSV41_FFN_BF16_QUANT
            // Both operands are BF16 here; retain the explicit BF16 product
            // boundary before FP8 conversion without a FP32 expand/round trip.
            const bfloat128 scaled = cached[tile] * sparse_inverse_bf16;
            minifloat256 packed = v_convert_bf16_to_f8_b(
                scaled, 0, SW_RHNE | SW_CLIP_FP, (minifloat256)0);
#else
            const float64_pair_t value =
                v_convert_bf16_to_f32_all_b(cached[tile]);
            minifloat256 packed = 0;
            packed = v_convert_f32_to_f8_b(
                round_bf16(value.v1 * inverse), 0, SW_CLIP_FP, packed);
            packed = v_convert_f32_to_f8_b(
                round_bf16(value.v2 * inverse), 2, SW_CLIP_FP, packed);
#endif
            const minifloat256 sparse = packed;
            packed = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2,
                                 (minifloat256)0);
            packed = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, packed);
            packed = v_f8_mov_dual_group_pack_b(
                packed, SW_PACK21, (minifloat256)0);
            v_f8_st_tnsr_partial(at, quantized, packed, 127, 0);
#ifdef DSV41_FFN_DUAL_QUANT
            const bfloat128 dense_value = cached[tile] * dense_inverse_bf16;
            minifloat256 dense_q = v_convert_bf16_to_f8_b(
                dense_value, 0, SW_RHNE | SW_CLIP_FP, (minifloat256)0);
            const minifloat256 dense_sparse = dense_q;
            dense_q = v_f8_pack_b(dense_sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            dense_q = v_f8_pack_b(dense_sparse, SW_GROUP_1 | SW_STRIDE_2, dense_q);
            dense_q = v_f8_mov_dual_group_pack_b(dense_q, SW_PACK21, (minifloat256)0);
            uchar256 raw = *((uchar256*)&dense_q);
            raw = v_u8_sel_eq_u8_b(raw & 0x78, 0, 0, raw);
            dense_q = *((minifloat256*)&raw);
            v_f8_st_tnsr_partial(at, dense_quantized, dense_q, 127, 0);
#endif
        }
    }
}
