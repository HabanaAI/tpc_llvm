// SPDX-License-Identifier: Apache-2.0
// Each feature tile consumes five maxima and emits its original FP8 lanes.
#include "deepseek_v41_silu_tile_math.h"
void main(tensor activated, tensor maxima, tensor output, tensor scales) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int tiles = get_dim_size(maxima, 0);
    for (int row = start[1]; row < end[1]; ++row) {
        float64 maximum = v_f32_ld_tnsr_partial_b((int5){0, 0, row}, maxima, tiles - 1, 0);
        maximum = row_max_without_lookup(maximum);
        const float64 raw_scale = round_bf16(maximum * (float)(bf16)(1.0f / 240.0f));
        const float64 scale = round_bf16(raw_scale + (float)(bf16)(1.0e-8f / 240.0f));
        const float64 inverse = round_bf16(reciprocal_without_lookup(scale));
        if (start[0] == 0 && end[0] > 0)
            v_f32_st_tnsr_partial((int5){0, 0, row}, scales, scale, 0, 0);
        for (int tile = start[0]; tile < end[0]; ++tile) {
            const bfloat128 value = v_bf16_ld_tnsr_b((int5){tile * 128, 0, row}, activated);
            const float128 fp32 = v_convert_bf16_to_f32_all_b(value);
            minifloat256 packed = 0;
            packed = v_convert_f32_to_f8_b(round_bf16(fp32.v1 * inverse), 0, SW_CLIP_FP, packed);
            packed = v_convert_f32_to_f8_b(round_bf16(fp32.v2 * inverse), 2, SW_CLIP_FP, packed);
            const minifloat256 sparse = packed;
            packed = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            packed = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, packed);
            packed = v_f8_mov_dual_group_pack_b(packed, SW_PACK21, (minifloat256)0);
            v_f8_st_tnsr_partial((int5){tile * 128, 0, row}, output, packed, 127, 0);
        }
    }
}
