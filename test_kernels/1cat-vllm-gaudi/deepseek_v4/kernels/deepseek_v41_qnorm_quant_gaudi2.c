// SPDX-License-Identifier: Apache-2.0
// Preserve the qualified attention RMSNorm BF16 boundary, then apply the
// exact power-of-two dense activation quantizer without materializing the
// normalized row. One index point owns one row; the implementation is valid
// for every row count admitted by the host contract, not only C1 decode.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"

#ifndef DSV41_QNORM_PUBLISH
#define DSV41_QNORM_PUBLISH 0
#endif
#ifndef DSV41_QNORM_TILES
#define DSV41_QNORM_TILES 10
#endif
#ifndef DSV41_QNORM_REMATERIALIZE
#define DSV41_QNORM_REMATERIALIZE 0
#endif
#ifndef DSV41_QNORM_EXTRA_INPUTS
#define DSV41_QNORM_EXTRA_INPUTS
#endif
#ifndef DSV41_QNORM_LOAD_VALUE
#define DSV41_QNORM_LOAD_VALUE(at) v_bf16_ld_tnsr_b(at, input)
#endif
#ifndef DSV41_QNORM_INPUT_WIDTH
#define DSV41_QNORM_INPUT_WIDTH get_dim_size(input, 0)
#endif
#ifdef DSV41_QNORM_FUNCTION
static inline void qnorm_quant_row(tensor input, tensor weight, DSV41_QNORM_EXTRA_INPUTS tensor quantized, tensor scales,
#else
void main(tensor input, tensor weight, tensor quantized, tensor scales,
#endif
#if DSV41_QNORM_PUBLISH
          tensor normalized,
#endif
          float epsilon, float inverse_width
#ifdef DSV41_QNORM_FUNCTION
          , const int5 begin, const int5 end
#endif
          ) {
#ifndef DSV41_QNORM_FUNCTION
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
#endif
    const int tiles = DSV41_QNORM_INPUT_WIDTH / 128;
#if !DSV41_QNORM_REMATERIALIZE
    bfloat128 cached[DSV41_QNORM_TILES];
#endif
    for (int row = begin[0]; row < end[0]; ++row) {
        float128 squares = {0};
        for (int tile = 0; tile < tiles; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
            const bfloat128 value = DSV41_QNORM_LOAD_VALUE(at);
#if !DSV41_QNORM_REMATERIALIZE
            cached[tile] = value;
#endif
            squares = v_bf16_mac_acc32_b(value, value, squares, (e_no_negation) << 1);
        }
        const float64 rrms = positive_rsqrt(row_sum(squares.v1 + squares.v2) * inverse_width + epsilon);

        // Reproduce attention_norm exactly, including its FP32 multiply order
        // and BF16 output rounding, while retaining the rounded values locally.
        float64 maximum = 0;
        for (int tile = 0; tile < tiles; ++tile) {
            const int5 wt = {tile * 128, 0, 0, 0, 0};
            const float128 w = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wt, weight));
#if DSV41_QNORM_REMATERIALIZE
            float128 value = v_convert_bf16_to_f32_all_b(DSV41_QNORM_LOAD_VALUE(((int5){tile * 128, row})));
#else
            float128 value = v_convert_bf16_to_f32_all_b(cached[tile]);
#endif
            value.v1 = (value.v1 * rrms) * w.v1;
            value.v2 = (value.v2 * rrms) * w.v2;
            const bfloat128 rounded = v_convert_f32_to_bf16_all_b(value);
#if !DSV41_QNORM_REMATERIALIZE
            cached[tile] = rounded;
#endif
#if DSV41_QNORM_PUBLISH
            const int5 output_at = {tile * 128, row, 0, 0, 0};
            v_bf16_st_tnsr(output_at, normalized, rounded);
#endif
            const float128 expanded = v_convert_bf16_to_f32_all_b(rounded);
            maximum = v_f32_max_b(maximum, v_f32_abs_b(expanded.v1));
            maximum = v_f32_max_b(maximum, v_f32_abs_b(expanded.v2));
        }

        // Keep the dense quantizer's scale selection byte-for-byte identical.
        maximum = v_f32_reduce_max(maximum);
        const uint64 maximum_bits = as_uint64(maximum);
        int64 power = convert_uint64_to_int64(maximum_bits >> 23, 0) - 134;
        power += v_i32_sel_grt_u32_b(maximum_bits & 0x7fffff, 0x700000, 1, 0);
        power = v_i32_sel_eq_f32_b(maximum, 0.0f, 0, power);
        const float64 scale = as_float64((power + 127) << 23);
        const float64 inverse = as_float64((127 - power) << 23);
#if DSV41_QNORM_PUBLISH
        // Dense quantization already uses this exact BF16 power-of-two
        // reciprocal. Avoid expanding each rounded row into two FP32 vectors.
        const float128 inverse_pair = {inverse, inverse};
        const bfloat128 inverse_bf16 = v_convert_f32_to_bf16_all_b(inverse_pair);
#endif
        const int5 scale_at = {0, row, 0, 0, 0};
        v_f32_st_tnsr_partial(scale_at, scales, scale, 0, 0);

        for (int tile = 0; tile < tiles; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
#if DSV41_QNORM_PUBLISH
#if DSV41_QNORM_REMATERIALIZE
            float128 rematerialized = v_convert_bf16_to_f32_all_b(DSV41_QNORM_LOAD_VALUE(at));
            const float128 weight_value = v_convert_bf16_to_f32_all_b(
                v_bf16_ld_tnsr_b((int5){tile * 128, 0}, weight));
            rematerialized.v1 = (rematerialized.v1 * rrms) * weight_value.v1;
            rematerialized.v2 = (rematerialized.v2 * rrms) * weight_value.v2;
            const bfloat128 value = v_convert_f32_to_bf16_all_b(rematerialized) * inverse_bf16;
#else
            const bfloat128 value = cached[tile] * inverse_bf16;
#endif
            minifloat256 q = v_convert_bf16_to_f8_b(value, 0, SW_RHNE | SW_CLIP_FP, (minifloat256)0);
#else
            const float128 value = v_convert_bf16_to_f32_all_b(cached[tile]);
            minifloat256 q = 0;
            q = v_convert_f32_to_f8_b(value.v1 * inverse, 0, SW_RHNE | SW_CLIP_FP, q);
            q = v_convert_f32_to_f8_b(value.v2 * inverse, 2, SW_RHNE | SW_CLIP_FP, q);
#endif
            const minifloat256 sparse = q;
            q = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            q = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, q);
            q = v_f8_mov_dual_group_pack_b(q, SW_PACK21, (minifloat256)0);
            uchar256 raw = *((uchar256*)&q);
            raw = v_u8_sel_eq_u8_b(raw & 0x78, 0, 0, raw);
            q = *((minifloat256*)&raw);
            v_f8_st_tnsr_partial(at, quantized, q, 127, 0);
        }
    }
}
