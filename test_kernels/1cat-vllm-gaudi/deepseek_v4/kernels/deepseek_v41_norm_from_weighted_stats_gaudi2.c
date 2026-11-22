// SPDX-License-Identifier: Apache-2.0
// Fuse the production FFN RMSNorm BF16 boundary with the exact activation
// quantizer consumed by the N256 expert graph.  Feature tiles reuse the producer RSS and weighted amax;
// preserve each BF16 boundary while quantizing the local feature tile.
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

#define DSV41_FFN_DUAL_QUANT
#define DSV41_FFN_BF16_QUANT
void main(tensor input,tensor weight,tensor statistics,tensor normalized,tensor quantized,tensor scales,
          tensor dense_quantized,tensor dense_scales,float epsilon,float inverse_width) {
 const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
 for(int row=begin[1];row<end[1];++row) {
#ifdef DSV41_WEIGHTED_STATS_PREPARED
        const float64 rrms=s_f32_ld_g(gen_addr((int5){0,row},statistics));
        const float64 inverse=s_f32_ld_g(gen_addr((int5){1,row},statistics));
        const float64 scale=s_f32_ld_g(gen_addr((int5){2,row},statistics));
#else
  const float64 squares=v_f32_ld_tnsr_b((int5){0,0,row},statistics);
  const float64 weighted=v_f32_ld_tnsr_b((int5){0,1,row},statistics);
  const float64 rrms=positive_rsqrt(row_sum(squares)*inverse_width+epsilon);
  const float64 maximum=round_bf16(row_max_without_lookup(weighted)*rrms);

        const float64 raw_scale = round_bf16(
            maximum * (float)(bf16)(1.0f / 240.0f));
        const float64 scale = round_bf16(
            raw_scale + (float)(bf16)(1.0e-8f / 240.0f));
        const float64 inverse = round_bf16(reciprocal_without_lookup(scale));
#endif
#ifdef DSV41_FFN_BF16_QUANT
        const float128 inverse_pair = {inverse, inverse};
        const bfloat128 sparse_inverse_bf16 = v_convert_f32_to_bf16_all_b(inverse_pair);
#endif
        const int5 scale_at = {0, row, 0, 0, 0};
        if(begin[0]==0) v_f32_st_tnsr_partial(scale_at, scales, scale, 0, 0);
#ifdef DSV41_FFN_DUAL_QUANT
        // The shared expert uses a power-of-two scale and flushes FP8
        // subnormals. Reuse the row amax, but preserve both quantizers.
#ifdef DSV41_WEIGHTED_STATS_PREPARED
        const float64 dense_inverse=s_f32_ld_g(gen_addr((int5){3,row},statistics));
        const float64 dense_scale=s_f32_ld_g(gen_addr((int5){4,row},statistics));
#else
        const uint64 bits = as_uint64(maximum);
        int64 power = convert_uint64_to_int64(bits >> 23, 0) - 134;
        power += v_i32_sel_grt_u32_b(bits & 0x7fffff, 0x700000, 1, 0);
        power = v_i32_sel_eq_f32_b(maximum, 0.0f, 0, power);
        const float64 dense_scale = as_float64((power + 127) << 23);
        const float64 dense_inverse = as_float64((127 - power) << 23);
#endif
        const float128 dense_inverse_pair = {dense_inverse, dense_inverse};
        const bfloat128 dense_inverse_bf16 = v_convert_f32_to_bf16_all_b(dense_inverse_pair);
        if(begin[0]==0) v_f32_st_tnsr_partial(scale_at, dense_scales, dense_scale, 0, 0);
#endif

        for (int tile = begin[0]; tile < end[0]; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
            const float128 input_value = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(at,input));
            const float128 weight_value = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b((int5){tile*128},weight));
            const float128 result = {(input_value.v1*rrms)*weight_value.v1,(input_value.v2*rrms)*weight_value.v2};
            const bfloat128 cached = v_convert_f32_to_bf16_all_b(result);
            v_bf16_st_tnsr(at,normalized,cached);
#ifdef DSV41_FFN_BF16_QUANT
            // Both operands are BF16 here; retain the explicit BF16 product
            // boundary before FP8 conversion without a FP32 expand/round trip.
            const bfloat128 scaled = cached * sparse_inverse_bf16;
            minifloat256 packed = v_convert_bf16_to_f8_b(
                scaled, 0, SW_RHNE | SW_CLIP_FP, (minifloat256)0);
#else
            const float64_pair_t value =
                v_convert_bf16_to_f32_all_b(cached);
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
            const bfloat128 dense_value = cached * dense_inverse_bf16;
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
