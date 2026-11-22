// SPDX-License-Identifier: Apache-2.0
#if defined(DSV41_N256_DIAGONAL_ROUTES) || defined(DSV41_N256_REDUCE_SHARED)
#include "deepseek_v41_c6_scale_reduce_direct.h"
#else
// SPDX-License-Identifier: Apache-2.0
#ifndef DSV41_SHARED_FINALIZE
#define DSV41_SHARED_FINALIZE 0
#endif
#ifndef DSV41_DIAGONAL_FINALIZE
#define DSV41_DIAGONAL_FINALIZE 0
#endif
// Scale the six W2 FP32 rows, preserve each row's BF16 boundary, and reduce
// them in routing order without materializing the six-row BF16 tensor.
void main(tensor product,
#if DSV41_DIAGONAL_FINALIZE
          tensor product_second,
#endif
          tensor ids, tensor activation_scale, tensor channel,
#if DSV41_SHARED_FINALIZE
          tensor shared,
#if defined(DSV41_SCALED_SHARED)
          tensor shared_channel, tensor shared_activation_scale,
#endif
#endif
          tensor output)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int experts = get_dim_size(channel, 2);
    const int batch = get_dim_size(ids, 0) / 6;
    const int first_token = batch == 1 ? 0 : start[1];
    const int last_token = batch == 1 ? 1 : end[1];
    for (int token = first_token; token < last_token; ++token) {
    for (int block = start[0]; block < end[0]; ++block) {
        const int n = block * 128;
        float128 accumulated = {0};
        #pragma loop_unroll(6)
        for (int slot = 0; slot < 6; ++slot) {
            const int row = token * 6 + slot;
            const int expert = s_i32_ld_g(gen_addr((int5){row}, ids));
            const bool valid = expert >= 0 && expert < experts;
            const float sx = s_f32_ld_g(
                gen_addr((int5){0, row}, activation_scale));
#if DSV41_DIAGONAL_FINALIZE
            // Each ordinary GEMM emits three rows against three horizontal
            // weight blocks. Consume only the expert's matching diagonal.
            const int column = n + (slot % 3) * get_dim_size(output, 0);
            const int local_row = slot % 3;
            float64 low_acc, high_acc;
            if (slot < 3) {
                low_acc = v_f32_ld_tnsr_b((int5){column, local_row}, product);
                high_acc = v_f32_ld_tnsr_b((int5){column + 64, local_row}, product);
            } else {
                low_acc = v_f32_ld_tnsr_b((int5){column, local_row}, product_second);
                high_acc = v_f32_ld_tnsr_b((int5){column + 64, local_row}, product_second);
            }
#else
            const float64 low_acc = v_f32_ld_tnsr_b(
                (int5){n, 0, row}, product);
            const float64 high_acc = v_f32_ld_tnsr_b(
                (int5){n + 64, 0, row}, product);
#endif
            const uint64 low_scale_bits = v_u32_ld_tnsr_b(
                (int5){n % 256, n / 256, expert}, channel,
                SW_UNPACK | SW_UNPCK_16_TO_32, (uint64){0}, valid) << 16;
            const uint64 high_scale_bits = v_u32_ld_tnsr_b(
                (int5){(n + 64) % 256, (n + 64) / 256, expert}, channel,
                SW_UNPACK | SW_UNPCK_16_TO_32, (uint64){0}, valid) << 16;
            float64_pair_t scaled;
            scaled.v1 = v_f32_mul_b(v_f32_mul_b(
                low_acc, *((float64*)&low_scale_bits)), sx);
            scaled.v2 = v_f32_mul_b(v_f32_mul_b(
                high_acc, *((float64*)&high_scale_bits)), sx);
            const bfloat128 rounded = convert_float128_to_bfloat128(
                scaled, SW_RHNE | SW_LINEAR);
            const float128 value = v_convert_bf16_to_f32_all_b(rounded);
            accumulated.v1 += value.v1;
            accumulated.v2 += value.v2;
        }
        const bfloat128 routed = v_convert_f32_to_bf16_all_b(accumulated, SW_RHNE);
#if DSV41_SHARED_FINALIZE
        float128 value = v_convert_bf16_to_f32_all_b(routed);
        bfloat128 shared_row = v_bf16_ld_tnsr_b((int5){n, token}, shared);
#if defined(DSV41_SCALED_SHARED)
        const float shared_sx=s_f32_ld_g(gen_addr((int5){0,token},shared_activation_scale));
        float128 combined_scale;
        combined_scale.v1=v_f32_ld_tnsr_b((int5){n,0},shared_channel)*shared_sx;
        combined_scale.v2=v_f32_ld_tnsr_b((int5){n+64,0},shared_channel)*shared_sx;
        shared_row=shared_row*convert_float128_to_bfloat128(combined_scale,SW_RHNE|SW_LINEAR);
#endif
        const float128 shared_value = v_convert_bf16_to_f32_all_b(shared_row);
        value.v1 += shared_value.v1;
        value.v2 += shared_value.v2;
        v_bf16_st_tnsr((int5){n, 0, token}, output, v_convert_f32_to_bf16_all_b(value, SW_RHNE));
#else
        v_bf16_st_tnsr((int5){n, 0, token}, output, routed);
#endif
    }
    }
}

#endif
