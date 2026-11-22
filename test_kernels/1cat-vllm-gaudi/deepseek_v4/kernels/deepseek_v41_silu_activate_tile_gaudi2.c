// SPDX-License-Identifier: Apache-2.0
// Keep five feature tiles independently schedulable per TP4 expert row.
#include "deepseek_v41_silu_tile_math.h"
void main(tensor product, tensor ids, tensor activation_scale, tensor channel,
          tensor router, tensor activated, tensor maxima) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = get_dim_size(activated, 0);
    const int experts = get_dim_size(channel, 2);
    const int scale_rows = get_dim_size(activation_scale, 1);
    for (int row = start[1]; row < end[1]; ++row) {
        const float sx = s_f32_ld_g(gen_addr((int5){0, scale_rows == 1 ? 0 : row}, activation_scale));
        const int expert = s_i32_ld_g(gen_addr((int5){row}, ids));
        const bool valid = expert >= 0 && expert < experts;
        const float route = s_f32_ld_g(gen_addr((int5){row}, router));
        for (int tile = start[0]; tile < end[0]; ++tile) {
            float64_pair_t pair;
            pair.v1 = activated_half(product, channel, tile * 128, row, expert, width, sx, route, valid);
            pair.v2 = activated_half(product, channel, tile * 128 + 64, row, expert, width, sx, route, valid);
            const bfloat128 value = convert_float128_to_bfloat128(pair, SW_RHNE | SW_LINEAR);
            v_bf16_st_tnsr((int5){tile * 128, 0, row}, activated, value);
            const float128 rounded = v_convert_bf16_to_f32_all_b(value);
            float64 maximum = v_f32_max_b((float64)0, v_f32_abs_b(rounded.v1));
            maximum = v_f32_max_b(maximum, v_f32_abs_b(rounded.v2));
            maximum = row_max_without_lookup(maximum);
            v_f32_st_tnsr_partial((int5){tile, 0, row}, maxima, maximum, 0, 0);
        }
    }
}
