// SPDX-License-Identifier: Apache-2.0
// Decode mHC control projection and reciprocal RMS share the same 20,480-wide
// BF16 input scan.  Work point zero additionally accumulates the row norm;
// the other 23 work points only produce their control-projection row.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"
#ifndef DSV41_CONTROL_BATCH_REUSE
#define DSV41_CONTROL_BATCH_REUSE 0
#endif

// v_convert_bf16_to_f32_all_b exposes the source as even/odd lanes.  Rebuild
// two linear float64 vectors so both the projection and norm visit K in the
// same order as the original BF16->FP32 graph.  This avoids preparing a
// second, lane-permuted copy of every control weight.
static inline float64_pair_t mhc_bf16_to_f32_linear(bfloat128 input) {
    bfloat128_pair_t unpacked;
    unpacked.v1 = v_bf16_unpack_b(
        input,
        ((e_group_0) << 8) | ((e_every_second_element) << 9) |
            ((e_lower_half_group) << 10),
        unpacked.v1);
    unpacked.v2 = v_bf16_unpack_b(
        input,
        ((e_group_1) << 8) | ((e_every_second_element) << 9) |
            ((e_lower_half_group) << 10),
        unpacked.v2);

    const bfloat128 first_groups = unpacked.v1;
    unpacked.v1 = v_bf16_mov_dual_group_b(
        unpacked.v2, 0xFFFFFFFF, 0, 1, MkWr(1, 1), unpacked.v1);
    unpacked.v1 = v_bf16_mov_dual_group_b(
        first_groups, 0xFFFFFFFF, 1, 2, MkWr(1, 1), unpacked.v1);
    unpacked.v1 = v_bf16_mov_dual_group_b(
        unpacked.v2, 0xFFFFFFFF, 1, 3, MkWr(1, 1), unpacked.v1);
    unpacked.v2 = v_bf16_mov_dual_group_b(
        first_groups, 0xFFFFFFFF, 2, 0, MkWr(1, 1), unpacked.v2);
    unpacked.v2 = v_bf16_mov_dual_group_b(
        unpacked.v2, 0xFFFFFFFF, 2, 1, MkWr(1, 1), unpacked.v2);
    unpacked.v2 = v_bf16_mov_dual_group_b(
        first_groups, 0xFFFFFFFF, 3, 2, MkWr(1, 1), unpacked.v2);

    const float128 first = v_convert_bf16_to_f32_all_b(unpacked.v1);
    const float128 second = v_convert_bf16_to_f32_all_b(unpacked.v2);
    return (float64_pair_t){first.v1, second.v1};
}

void main(tensor activation, tensor weight, tensor output,
          float epsilon, float inverse_width) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
#if DSV41_CONTROL_BATCH_REUSE
    const int tokens=get_dim_size(activation,1);
    for(int row=begin[0];row<end[0];++row) {
        float64 accumulator[6]={{0}};
        float64_pair_t squares={0};
        for(int k=0;k<20480;k+=128) {
            const float64 first=v_f32_ld_tnsr_b((int5){k,row},weight);
            const float64 second=v_f32_ld_tnsr_b((int5){k+64,row},weight);
            #pragma loop_unroll(6)
            for(int token=0;token<6;++token) {
                if(token<tokens) {
                    const float128 x=convert_bfloat128_to_float128(
                        v_bf16_ld_tnsr_b((int5){k,token},activation),SW_LINEAR);
                    accumulator[token]=v_f32_mac_b(x.v1,first,accumulator[token]);
                    accumulator[token]=v_f32_mac_b(x.v2,second,accumulator[token]);
                    if(row==token) {
                        squares.v1+=x.v1*x.v1;squares.v2+=x.v2*x.v2;
                    }
                }
            }
        }
        #pragma loop_unroll(6)
        for(int token=0;token<6;++token)
            if(token<tokens)v_f32_st_tnsr_partial((int5){row,token},output,
                v_f32_reduce_add(accumulator[token]),0,0);
        if(row<tokens) {
            const float64 mean=v_f32_reduce_add(squares.v1+squares.v2)*inverse_width;
            v_f32_st_tnsr_partial((int5){24,row},output,positive_rsqrt(mean+epsilon),0,0);
        }
    }
#else
    for (int token = begin[1]; token < end[1]; ++token) {
        for (int row = begin[0]; row < end[0]; ++row) {
            float64 accumulator = 0.0f;
            float64_pair_t squares = {0};
            for (int k = 0; k < 20480; k += 128) {
                const int5 ac = {k, token, 0, 0, 0};
#ifdef DSV41_CONTROL_UNPACK_LOAD
                // SW_LINEAR lets the compiler combine the packed load and
                // lane conversion. The explicit two-half load prototype had
                // twice as many activation loads and a longer load-use chain.
                const bfloat128 packed = v_bf16_ld_tnsr_b(ac, activation);
                const float128 linear = convert_bfloat128_to_float128(packed, SW_LINEAR);
                const float64_pair_t value = {linear.v1, linear.v2};
#else
                const bfloat128 packed = v_bf16_ld_tnsr_b(ac, activation);
                const float64_pair_t value = mhc_bf16_to_f32_linear(packed);
#endif
                const int5 wc0 = {k, row, 0, 0, 0};
                const int5 wc1 = {k + 64, row, 0, 0, 0};
                accumulator = v_f32_mac_b(value.v1,
                                           v_f32_ld_tnsr_b(wc0, weight),
                                           accumulator);
                accumulator = v_f32_mac_b(value.v2,
                                           v_f32_ld_tnsr_b(wc1, weight),
                                           accumulator);
                if (row == 0) {
                    squares.v1 = v_f32_mac_b(value.v1, value.v1, squares.v1);
                    squares.v2 = v_f32_mac_b(value.v2, value.v2, squares.v2);
                }
            }
            const int5 pc = {row, token, 0, 0, 0};
            v_f32_st_tnsr_partial(pc, output,
                                  v_f32_reduce_add(accumulator), 0, 0);
            if (row == 0) {
                // Match the compiler's FP32 reduction primitive used by
                // flat.square().mean(), including its lane reduction tree.
                const float64 mean_square =
                    v_f32_reduce_add(squares.v1 + squares.v2) * inverse_width;
                const int5 rc = {24, token, 0, 0, 0};
                v_f32_st_tnsr_partial(
                    rc, output, positive_rsqrt(mean_square + epsilon), 0, 0);
            }
        }
    }
#endif
}
