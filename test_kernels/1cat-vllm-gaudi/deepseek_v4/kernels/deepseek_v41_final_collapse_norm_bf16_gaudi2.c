// SPDX-License-Identifier: Apache-2.0
// Fuse the PP1 four-way residual collapse with the qualified final RMSNorm.
// Preserve the existing BF16 boundary between collapse and norm exactly.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"

void main(tensor residual, tensor pre_mix, tensor weight, tensor output,
          float epsilon, float inverse_width) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    bfloat128 cached[40];

    for (int token = begin[0]; token < end[0]; ++token) {
        const float p0 = s_f32_ld_g(gen_addr((int5){0, token, 0, 0, 0}, pre_mix));
        const float p1 = s_f32_ld_g(gen_addr((int5){1, token, 0, 0, 0}, pre_mix));
        const float p2 = s_f32_ld_g(gen_addr((int5){2, token, 0, 0, 0}, pre_mix));
        const float p3 = s_f32_ld_g(gen_addr((int5){3, token, 0, 0, 0}, pre_mix));
        float128 squares = {0};

        for (int tile = 0; tile < 40; ++tile) {
            const int width = tile * 128;
            const float128 r0 = v_convert_bf16_to_f32_all_b(
                v_bf16_ld_tnsr_b((int5){width, 0, token, 0, 0}, residual));
            const float128 r1 = v_convert_bf16_to_f32_all_b(
                v_bf16_ld_tnsr_b((int5){width, 1, token, 0, 0}, residual));
            const float128 r2 = v_convert_bf16_to_f32_all_b(
                v_bf16_ld_tnsr_b((int5){width, 2, token, 0, 0}, residual));
            const float128 r3 = v_convert_bf16_to_f32_all_b(
                v_bf16_ld_tnsr_b((int5){width, 3, token, 0, 0}, residual));
            float128 collapsed;
            collapsed.v1 = r0.v1 * p0;
            collapsed.v1 = collapsed.v1 + r1.v1 * p1;
            collapsed.v1 = collapsed.v1 + r2.v1 * p2;
            collapsed.v1 = collapsed.v1 + r3.v1 * p3;
            collapsed.v2 = r0.v2 * p0;
            collapsed.v2 = collapsed.v2 + r1.v2 * p1;
            collapsed.v2 = collapsed.v2 + r2.v2 * p2;
            collapsed.v2 = collapsed.v2 + r3.v2 * p3;
            const bfloat128 rounded =
                v_convert_f32_to_bf16_all_b(collapsed, SW_RHNE);
            cached[tile] = rounded;
            squares = v_bf16_mac_acc32_b(rounded, rounded, squares,
                                         (e_no_negation) << 1);
        }

        const float64 rrms = positive_rsqrt(
            row_sum(squares.v1 + squares.v2) * inverse_width + epsilon);
        for (int tile = 0; tile < 40; ++tile) {
            const int5 at = {tile * 128, token, 0, 0, 0};
            const int5 wt = {tile * 128, 0, 0, 0, 0};
            const float128 w = v_convert_bf16_to_f32_all_b(
                v_bf16_ld_tnsr_b(wt, weight));
            float128 value = v_convert_bf16_to_f32_all_b(cached[tile]);
            value.v1 = (value.v1 * rrms) * w.v1;
            value.v2 = (value.v2 * rrms) * w.v2;
            v_bf16_st_tnsr(at, output,
                           v_convert_f32_to_bf16_all_b(value, SW_RHNE));
        }
    }
}
