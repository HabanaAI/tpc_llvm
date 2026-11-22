// SPDX-License-Identifier: Apache-2.0
// BF16 [T,G,K] -> E4M3 [G,T,K], with one exact power-of-two scale per row.
void main(tensor input, tensor output, tensor scales) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    bfloat128 values[32];
    for (int group = start[1]; group < end[1]; ++group) {
        for (int token = start[0]; token < end[0]; ++token) {
            float64 maximum = 0;
            #pragma loop_unroll(4)
            for (int tile = 0; tile < 32; ++tile) {
                const int5 at = {tile * 128, group, token, 0, 0};
                values[tile] = v_bf16_ld_tnsr_b(at, input);
                const float128 wide = v_convert_bf16_to_f32_all_b(values[tile]);
                maximum = v_f32_max_b(maximum, v_f32_abs_b(wide.v1));
                maximum = v_f32_max_b(maximum, v_f32_abs_b(wide.v2));
            }
            maximum = v_f32_reduce_max(maximum);
            const uint64 bits = as_uint64(maximum);
            int64 power = convert_uint64_to_int64(bits >> 23, 0) - 134;
            power += v_i32_sel_grt_u32_b(bits & 0x7fffff, 0x700000, 1, 0);
            power = v_i32_sel_eq_f32_b(maximum, 0.0f, 0, power);
            const float64 scale = as_float64((power + 127) << 23);
            const float64 inverse = as_float64((127 - power) << 23);
            const float128 inverse_pair = {inverse, inverse};
            const bfloat128 inverse_bf16 = v_convert_f32_to_bf16_all_b(inverse_pair);
            const int5 scale_at = {0, token, group, 0, 0};
            v_f32_st_tnsr_partial(scale_at, scales, scale, 0, 0);
            #pragma loop_unroll(2)
            for (int tile = 0; tile < 32; ++tile) {
                const bfloat128 value = values[tile] * inverse_bf16;
                minifloat256 q = v_convert_bf16_to_f8_b(value, 0, SW_RHNE | SW_CLIP_FP, (minifloat256)0);
                const minifloat256 sparse = q;
                q = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
                q = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, q);
                q = v_f8_mov_dual_group_pack_b(q, SW_PACK21, (minifloat256)0);
                uchar256 raw = *((uchar256*)&q);
                raw = v_u8_sel_eq_u8_b(raw & 0x78, 0, 0, raw);
                q = *((minifloat256*)&raw);
                const int5 out_at = {tile * 128, token, group, 0, 0};
                v_f8_st_tnsr_partial(out_at, output, q, 127, 0);
            }
        }
    }
}
