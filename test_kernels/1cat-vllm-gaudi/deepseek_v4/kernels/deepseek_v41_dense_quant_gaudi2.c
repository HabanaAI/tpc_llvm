// SPDX-License-Identifier: Apache-2.0
// A complete K row shares one power-of-two scale. Reload the small activation
// instead of retaining dynamically indexed vector arrays in local memory.
#ifndef DSV41_DENSE_BF16_PAIR
#define DSV41_DENSE_BF16_PAIR 0
#endif
#if DSV41_DENSE_BF16_PAIR
void main(tensor input, tensor output) {
#else
void main(tensor input, tensor output, tensor scales) {
#endif
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = get_dim_size(input, 0);
    const int tiles = width / 128;
    for (int row = start[0]; row < end[0]; ++row) {
        float64 maximum = 0;
        #pragma loop_unroll(4) pipelined taken
        for (int tile = 0; tile < tiles; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
            const float128 wide = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(at, input));
            maximum = v_f32_max_b(maximum, v_f32_abs_b(wide.v1));
            maximum = v_f32_max_b(maximum, v_f32_abs_b(wide.v2));
        }
        if (width % 128) {
            const int5 at = {tiles * 128, row, 0, 0, 0};
            const bfloat128 tail = v_bf16_ld_tnsr_partial_b(at, input, width % 128 - 1, 0);
            const float128 wide = v_convert_bf16_to_f32_all_b(tail);
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
        // A power-of-two reciprocal is exactly representable in BF16. Its
        // product has the same FP8 rounding in the representable FP8 range.
        const float128 inverse_pair = {inverse, inverse};
        const bfloat128 inverse_bf16 = v_convert_f32_to_bf16_all_b(inverse_pair);
#if DSV41_DENSE_BF16_PAIR
        const bfloat128 scale_bf16 = v_convert_f32_to_bf16_all_b((float128){scale, scale});
        const int rows = get_dim_size(input, 1);
#else
        const int5 scale_at = {0, row, 0, 0, 0};
        v_f32_st_tnsr_partial(scale_at, scales, scale, 0, 0);
#endif
        #pragma loop_unroll(4) pipelined taken
        for (int tile = 0; tile < tiles; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
            #if DSV41_DENSE_BF16_PAIR
            const bfloat128 original = v_bf16_ld_tnsr_b(at, input);
            v_bf16_st_tnsr_partial(at, output, original, 127, 0);
            const bfloat128 value = original * inverse_bf16;
#else
            const bfloat128 value = v_bf16_ld_tnsr_b(at, input) * inverse_bf16;
#endif
            minifloat256 q = v_convert_bf16_to_f8_b(value, 0, SW_RHNE | SW_CLIP_FP, (minifloat256)0);
#if DSV41_DENSE_BF16_PAIR
            uchar256 bits = *((uchar256*)&q);
            bits = v_u8_sel_eq_u8_b(bits & 0x78, 0, 0, bits);
            q = *((minifloat256*)&bits);
            const bfloat128 restored = v_convert_f8_to_bf16_b(q) * scale_bf16;
            v_bf16_st_tnsr_partial((int5){at[0], row + rows}, output, restored, 127, 0);
#else
            const minifloat256 sparse = q;
            q = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            q = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, q);
            q = v_f8_mov_dual_group_pack_b(q, SW_PACK21, (minifloat256)0);
            uchar256 raw = *((uchar256*)&q);
            raw = v_u8_sel_eq_u8_b(raw & 0x78, 0, 0, raw);
            q = *((minifloat256*)&raw);
            v_f8_st_tnsr_partial(at, output, q, 127, 0);
#endif
        }
        if (width % 128) {
            const int5 at = {tiles * 128, row, 0, 0, 0};
            const int last = width % 128 - 1;
            #if DSV41_DENSE_BF16_PAIR
            const bfloat128 original = v_bf16_ld_tnsr_partial_b(at, input, last, 0);
            v_bf16_st_tnsr_partial(at, output, original, last, 0);
            const bfloat128 value = original * inverse_bf16;
#else
            const bfloat128 value = v_bf16_ld_tnsr_partial_b(at, input, last, 0) * inverse_bf16;
#endif
            minifloat256 q = v_convert_bf16_to_f8_b(value, 0, SW_RHNE | SW_CLIP_FP, (minifloat256)0);
#if DSV41_DENSE_BF16_PAIR
            uchar256 bits = *((uchar256*)&q);
            bits = v_u8_sel_eq_u8_b(bits & 0x78, 0, 0, bits);
            q = *((minifloat256*)&bits);
            const bfloat128 restored = v_convert_f8_to_bf16_b(q) * scale_bf16;
            v_bf16_st_tnsr_partial((int5){at[0], row + rows}, output, restored, last, 0);
#else
            const minifloat256 sparse = q;
            q = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            q = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, q);
            q = v_f8_mov_dual_group_pack_b(q, SW_PACK21, (minifloat256)0);
            uchar256 raw = *((uchar256*)&q);
            raw = v_u8_sel_eq_u8_b(raw & 0x78, 0, 0, raw);
            q = *((minifloat256*)&raw);
            v_f8_st_tnsr_partial(at, output, q, last, 0);
#endif
        }
    }
}
