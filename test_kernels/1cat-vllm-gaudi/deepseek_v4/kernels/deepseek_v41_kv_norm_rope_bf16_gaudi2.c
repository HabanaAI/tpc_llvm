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

#ifndef DSV41_KV_NORM_EXTRA_INPUTS
#define DSV41_KV_NORM_EXTRA_INPUTS
#endif
#ifndef DSV41_KV_NORM_LOAD_VALUE
#define DSV41_KV_NORM_LOAD_VALUE(at) v_bf16_ld_tnsr_b(at, input)
#endif
#ifdef DSV41_KV_NORM_FUNCTION
static inline void kv_norm_rope_row(tensor input, tensor weight, DSV41_KV_NORM_EXTRA_INPUTS
          tensor positions, tensor phase, tensor output, float epsilon, float inverse_width,
          const int5 begin, const int5 end) {
#else
void main(tensor input, tensor weight, tensor positions, tensor phase,
          tensor output, float epsilon, float inverse_width) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
#endif
    bfloat128 cached[4];
    for (int row = begin[0]; row < end[0]; ++row) {
        float128 squares = {0};
        #pragma unroll (4)
        for (int tile = 0; tile < 4; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
            const bfloat128 value = DSV41_KV_NORM_LOAD_VALUE(at);
            cached[tile] = value;
            squares = v_bf16_mac_acc32_b(
                value, value, squares, (e_no_negation) << 1);
        }
        const float64 rrms = positive_rsqrt(
            row_sum(squares.v1 + squares.v2) * inverse_width + epsilon);
        const int position = s_i32_ld_g(
            gen_addr((int5){row, 0, 0, 0, 0}, positions));

        #pragma unroll (4)
        for (int tile = 0; tile < 4; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
            const int5 wt = {tile * 128, 0, 0, 0, 0};
            const float128 w = v_convert_bf16_to_f32_all_b(
                v_bf16_ld_tnsr_b(wt, weight));
            float128 value = v_convert_bf16_to_f32_all_b(cached[tile]);
            value.v1 = (value.v1 * rrms) * w.v1;
            value.v2 = (value.v2 * rrms) * w.v2;
            const bfloat128 rounded = v_convert_f32_to_bf16_all_b(value);
            if (tile != 3) {
                v_bf16_st_tnsr(at, output, rounded);
                continue;
            }

            // Match the two original kernels exactly: first round the whole
            // normalized tile to BF16. Repack Gaudi's four interleaved dual
            // groups into linear halves before selecting logical [64:128].
            v_bf16_st_tnsr(at, output, rounded);
            const int5 tail = {448, row, 0, 0, 0};
            const float64 expanded_tail = kv_bf16_to_f32_linear(rounded).v2;
            float128 rotated = {0};
            rotated.v1 = dsv4_qkv_apply_pairwise_rope_f32(
                expanded_tail, phase, position);
            v_bf16_st_tnsr_partial(
                tail, output,
                convert_float128_to_bfloat128(rotated, SW_LINEAR), 63, 0);
        }
    }
}
