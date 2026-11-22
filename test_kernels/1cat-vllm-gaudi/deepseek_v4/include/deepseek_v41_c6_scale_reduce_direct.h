// SPDX-License-Identifier: Apache-2.0
#ifndef DSV41_SHARED_FINALIZE
#define DSV41_SHARED_FINALIZE 0
#endif
#ifndef DSV41_DIAGONAL_FINALIZE
#define DSV41_DIAGONAL_FINALIZE 0
#endif
// Scale the six W2 FP32 rows, preserve each row's BF16 boundary, and reduce
// them in routing order without materializing the six-row BF16 tensor.
#ifndef DSV41_N256_DIAGONAL_ROUTES
#define DSV41_N256_DIAGONAL_ROUTES 0
#endif
#ifndef DSV41_N256_REDUCE_SHARED
#define DSV41_N256_REDUCE_SHARED 0
#endif
void main(tensor product, tensor ids, tensor activation_scale, tensor channel,
#if DSV41_N256_REDUCE_SHARED
          tensor shared,
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
#if DSV41_N256_DIAGONAL_ROUTES
            const int5 coordinate = {n + (slot % 3) * get_dim_size(output, 0), slot % 3, row / 3};
#else
            const int5 coordinate = {n, 0, row};
#endif
            int5 upper_coordinate = coordinate;
            upper_coordinate[0] += 64;
            const float64 low_acc = v_f32_ld_tnsr_b(
                coordinate, product);
            const float64 high_acc = v_f32_ld_tnsr_b(
                upper_coordinate, product);
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
        bfloat128 combined=v_convert_f32_to_bf16_all_b(accumulated,SW_RHNE);
#if DSV41_N256_REDUCE_SHARED
        const float128 routed=v_convert_bf16_to_f32_all_b(combined);
        const float128 dense=v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b((int5){n,0,token},shared));
        float128 sum;
        sum.v1=routed.v1+dense.v1;sum.v2=routed.v2+dense.v2;
        combined=v_convert_f32_to_bf16_all_b(sum,SW_RHNE);
#endif
        v_bf16_st_tnsr((int5){n,0,token},output,combined);
    }
    }
}
