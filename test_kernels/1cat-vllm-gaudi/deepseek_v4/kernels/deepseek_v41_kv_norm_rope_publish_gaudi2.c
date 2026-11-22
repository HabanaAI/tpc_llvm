// SPDX-License-Identifier: Apache-2.0
// Fuse the exact V4.1 KV RMSNorm and forward RoPE while preserving the BF16
// boundary between them.  One index-space point owns one token row, so this
// remains useful for batched decode rather than being a C1-only special case.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"

#define DSV4_QNORM_HELPERS_ONLY 1
#define DSV4_ROPE_SECOND_TERM_FMA 1
#include "deepseek_v4_qnorm_rope_kv_pack_bf16.h"

static inline float64_pair_t kv_bf16_to_f32_linear(bfloat128 input) {
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

#define DSV41_DECODED_KV_WRITE 1
#define DSV41_OPTIONAL_DECODED_KV_WRITE 1
#include "deepseek_v41_swa_pack.h"

#ifdef DSV41_KV_PUBLISH_FUNCTION
static inline void kv_norm_publish_groups(tensor input, tensor weight, tensor positions, tensor phase,
#else
void main(tensor input, tensor weight, tensor positions, tensor phase,
#endif
          tensor cache, tensor decoded, tensor output, tensor completion,
          float epsilon, float inverse_width, int decoded_offset
#ifdef DSV41_KV_PUBLISH_FUNCTION
          , const int5 begin, const int5 end
#endif
          ) {
#ifndef DSV41_KV_PUBLISH_FUNCTION
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
#endif
    bfloat128 cached[4];
#ifdef DSV41_KV_PUBLISH_FUNCTION
    const int row_end = end[1] ? end[1] : 1;
#else
    const int row_end = end[1];
#endif
    for (int row = begin[1]; row < row_end; ++row) {
    for (int group = begin[0]; group < end[0]; ++group) {
        float128 squares = {0};
        #pragma unroll (4)
        for (int tile = 0; tile < 4; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
            const bfloat128 value = v_bf16_ld_tnsr_b(at, input);
            cached[tile] = value;
            squares = v_bf16_mac_acc32_b(
                value, value, squares, (e_no_negation) << 1);
        }
        const float64 rrms = positive_rsqrt(
            row_sum(squares.v1 + squares.v2) * inverse_width + epsilon);
        const int position = s_i32_ld_g(
            gen_addr((int5){row, 0, 0, 0, 0}, positions));

        const int ring = position & 255;
        const bool valid = position >= 0 && ring < get_dim_size(cache, 1) &&
                           (decoded_offset < 0 || decoded_offset + ring < get_dim_size(decoded, 1));
        {
            const int tile = group / 4;
            const int5 wt = {tile * 128, 0, 0, 0, 0};
            const float128 w = v_convert_bf16_to_f32_all_b(
                v_bf16_ld_tnsr_b(wt, weight));
            float128 value = v_convert_bf16_to_f32_all_b(cached[tile]);
            value.v1 = (value.v1 * rrms) * w.v1;
            value.v2 = (value.v2 * rrms) * w.v2;
            const bfloat128 rounded = v_convert_f32_to_bf16_all_b(value);
            float64_pair_t linear = kv_bf16_to_f32_linear(rounded);
            if (tile == 3) {
                float128 rotated = {0};
                rotated.v1 = dsv4_qkv_apply_pairwise_rope_f32(linear.v2, phase, position);
                const bfloat128 rounded_tail = convert_float128_to_bfloat128(rotated, SW_LINEAR);
                linear.v2 = convert_bfloat128_to_float128(rounded_tail, SW_LINEAR).v1;
            }
            {
                const int part = group & 3;
                const float64 half_values = part < 2 ? linear.v1 : linear.v2;
                const float64 selected = (part & 1)
                    ? v_f32_mov_dual_group_all_b(half_values, 0xffffffff, 2, 3, 2, 3, MkWrA(3, 3, 3, 3), 0)
                    : half_values;
                const float64 group_value = v_f32_sel_less_i32_b((int64)V_LANE_ID_32, 32, selected, (float64)0);
                const float128 packed_group = {group_value, (float64)0};
                v_bf16_st_tnsr_partial((int5){32 * group, row}, output,
                    convert_float128_to_bfloat128(packed_group, SW_RHNE | SW_LINEAR), 31, 0);
                if (valid) swa_pack_number(group_value, cache, group, ring, 512, decoded, decoded_offset < 0 ? -1 : decoded_offset + ring);
                s_i32_st_g(gen_addr((int5){group, row}, completion), valid ? position : -1);
            }
        }
    }
}
}
