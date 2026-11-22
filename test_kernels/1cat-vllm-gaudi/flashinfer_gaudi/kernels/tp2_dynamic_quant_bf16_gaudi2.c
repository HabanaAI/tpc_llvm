// SPDX-License-Identifier: Apache-2.0
// One index point owns a row. Intermediate products stay in VLM.
static inline float64 round_bf16(float64 value) {
    float64_pair_t pair;
    pair.v1 = value;
    pair.v2 = value;
    return v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(pair)).v1;
}

static inline float64 reciprocal_without_lookup(float64 value) {
    // Positive normal scales are bounded below by epsilon/240 and above
    // by BF16_MAX/240. Three Newton steps preserve the BF16-rounded result.
    float64 estimate = as_float64((int64)0x7ef311c3 - as_int64(value));
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    return v_f32_sel_eq_f32_b(value, as_float64((int64)0x7f800000), 0.0f, estimate);
}

static inline float64 row_max_without_lookup(float64 value) {
    // Fold the four dual groups and their two groups. Unlike the generic
    // reduction helper, explicit lane broadcasts do not reserve the LUT cache.
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(value, 0xffffffff, 1, 0, 3, 2,
                                                      MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(value, 0xffffffff, 2, 3, 0, 1,
                                                      MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_group_b(value, 0xffffffff, 63, 0));
    float64 result = 0;
    #pragma loop_unroll(8)
    for (int lane = 0; lane < 8; ++lane) {
        const uchar256 broadcast = 0x80 | lane;
        result = v_f32_max_b(result, v_f32_shuffle_b(value, broadcast, 0, value));
    }
    return result;
}

void main(tensor input, tensor output, tensor scales) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = get_dim_size(output, 0);
    // No lookup instructions: use the compiler's large VLM allocation budget.
    bfloat128 activated[136];
    for (int row = start[0]; row < end[0]; ++row) {
        float64 maximum = 0;
        #pragma loop_unroll(4) pipelined taken
        for (int tile = 0; tile < width / 128; ++tile) {
            int5 source_coord = {tile * 128, row, 0, 0, 0};
            const bfloat128 value = v_bf16_ld_tnsr_b(source_coord, input);
            activated[tile] = value;
            const float64_pair_t fp32 = v_convert_bf16_to_f32_all_b(value);
            maximum = v_f32_max_b(maximum, v_f32_abs_b(fp32.v1));
            maximum = v_f32_max_b(maximum, v_f32_abs_b(fp32.v2));
        }
        maximum = row_max_without_lookup(maximum);
        // Match the CGUID scale, BF16 epsilon add, reciprocal and cast boundaries.
        const float64 raw_scale = round_bf16(maximum * (float)(bf16)(1.0f / 240.0f));
        const float64 scale = round_bf16(raw_scale + (float)(bf16)(1.0e-8f / 240.0f));
        const float64 inverse = round_bf16(reciprocal_without_lookup(scale));
        int5 scale_coord = {0, row, 0, 0, 0};
        v_f32_st_tnsr(scale_coord, scales, scale);
        #pragma loop_unroll(2) pipelined taken
        for (int tile = 0; tile < width / 128; ++tile) {
            const bfloat128 value = activated[tile];
            const float64_pair_t fp32 = v_convert_bf16_to_f32_all_b(value);
            minifloat256 packed = 0;
            // Combine the paired conversion lanes and both source groups
            // before compacting the dual groups into 128 contiguous FP8 bytes.
            packed = v_convert_f32_to_f8_b(round_bf16(fp32.v1 * inverse), 0, SW_CLIP_FP, packed);
            packed = v_convert_f32_to_f8_b(round_bf16(fp32.v2 * inverse), 2, SW_CLIP_FP, packed);
            const minifloat256 sparse = packed;
            packed = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            packed = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, packed);
            packed = v_f8_mov_dual_group_pack_b(packed, SW_PACK21, (minifloat256)0);
            int5 coord = {tile * 128, row, 0, 0, 0};
            v_f8_st_tnsr_partial(coord, output, packed, 127, 0);
        }
    }
}
