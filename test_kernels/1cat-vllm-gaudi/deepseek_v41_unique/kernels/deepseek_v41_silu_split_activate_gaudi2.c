// SPDX-License-Identifier: Apache-2.0
// Separate logical gate/up operands expose two precise 128-channel dependencies.
#include "../../deepseek_v4/include/deepseek_v41_silu_tile_math.h"
void main(tensor gate_product, tensor up_product, tensor ids, tensor activation_scale,
          tensor channel, tensor router, tensor activated, tensor maxima) {
    const int5 start = get_index_space_offset(), end = start + get_index_space_size();
    const int width = get_dim_size(activated, 0), experts = get_dim_size(channel, 2);
    const int scale_rows = get_dim_size(activation_scale, 1);
    for (int row = start[1]; row < end[1]; ++row) {
        const float sx = s_f32_ld_g(gen_addr((int5){0, scale_rows == 1 ? 0 : row}, activation_scale));
        const int expert = s_i32_ld_g(gen_addr((int5){row}, ids));
        const bool valid = expert >= 0 && expert < experts;
        const float route = s_f32_ld_g(gen_addr((int5){row}, router));
        for (int tile = start[0]; tile < end[0]; ++tile) {
            float128 pair;
            #pragma unroll(2)
            for (int half_index = 0; half_index < 2; ++half_index) {
                const int n = tile * 128 + half_index * 64;
                float64 gate = projected_at(gate_product, channel, n, n, row, expert, sx, valid);
                float64 up = projected_at(up_product, channel, n, n + width, row, expert, sx, valid);
                gate = v_f32_min_b(gate, 10.0f);
                up = v_f32_max_b(v_f32_min_b(up, 10.0f), -10.0f);
                const float64 value = v_f32_mul_b(v_f32_mul_b(v_f32_mul_b(gate, v_sigmoid_f32(gate)), up), route);
                if (half_index == 0) pair.v1 = value;
                else pair.v2 = value;
            }
            const bfloat128 value = convert_float128_to_bfloat128(pair, SW_RHNE | SW_LINEAR);
            v_bf16_st_tnsr((int5){tile * 128, 0, row}, activated, value);
            const float128 rounded = v_convert_bf16_to_f32_all_b(value);
            float64 maximum = v_f32_max_b((float64)0, v_f32_abs_b(rounded.v1));
            maximum = row_max_without_lookup(v_f32_max_b(maximum, v_f32_abs_b(rounded.v2)));
            v_f32_st_tnsr_partial((int5){tile, 0, row}, maxima, maximum, 0, 0);
        }
    }
}
