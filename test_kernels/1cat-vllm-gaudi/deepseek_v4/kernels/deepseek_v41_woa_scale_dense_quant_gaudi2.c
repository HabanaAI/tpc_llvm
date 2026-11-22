// SPDX-License-Identifier: Apache-2.0
// Preserve the wo_a BF16/group-32 roundtrip, then emit the dense FP8 row
// directly. C1 keeps its qualified tile ownership; C2-C6 compute the global
// scale once per token and avoid a BF16 intermediate or barrier.
#include "deepseek_v41_group32_roundtrip.h"
static inline bfloat128 roundtrip(tensor product, tensor weight, tensor activation,
                                 int group, int tile, int token) {
    const int5 sx_at = {0, token, group, 0, 0};
    const float sx = s_f32_ld_g(gen_addr(sx_at, activation));
    const int5 a0 = {tile * 128, token, group, 0, 0};
    const int5 a1 = {tile * 128 + 64, token, group, 0, 0};
    float128 scaled;
    scaled.v1 = (v_f32_ld_tnsr_b(a0, product) * v_f32_ld_tnsr_b((int5){tile*128,0,group}, weight)) * sx;
    scaled.v2 = (v_f32_ld_tnsr_b(a1, product) * v_f32_ld_tnsr_b((int5){tile*128+64,0,group}, weight)) * sx;
    return v41_group32_roundtrip_bf16(convert_float128_to_bfloat128(scaled, SW_RHNE | SW_LINEAR));
}
static inline minifloat256 dense_quant(bfloat128 value, float64 inverse) {
    const float128 pair = {inverse, inverse};
    value = value * v_convert_f32_to_bf16_all_b(pair);
    minifloat256 q = v_convert_bf16_to_f8_b(value, 0, SW_RHNE | SW_CLIP_FP, (minifloat256)0);
    const minifloat256 sparse = q;
    q = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
    q = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, q);
    q = v_f8_mov_dual_group_pack_b(q, SW_PACK21, (minifloat256)0);
    uchar256 raw = *((uchar256*)&q);
    raw = v_u8_sel_eq_u8_b(raw & 0x78, 0, 0, raw);
    return *((minifloat256*)&raw);
}
void main(tensor product, tensor weight, tensor activation, tensor output, tensor scales) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int groups = get_dim_size(product, 2);
    // C2-C6: one token owner computes the global maximum once. Re-read
    // the small accumulator instead of keeping a dynamically indexed local
    // vector array; both passes preserve the qualified C1 rounding order.
    if (get_dim_size(product, 1) > 1) {
        for (int token = begin[1]; token < end[1]; ++token) {
            float64 maximum = 0;
            for (int group = 0; group < groups; ++group)
            for (int tile = 0; tile < 8; ++tile) {
                const float128 wide = v_convert_bf16_to_f32_all_b(
                    roundtrip(product, weight, activation, group, tile, token));
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
            for (int group = 0; group < groups; ++group)
            for (int tile = 0; tile < 8; ++tile) {
                const minifloat256 q = dense_quant(
                    roundtrip(product, weight, activation, group, tile, token), inverse);
                v_f8_st_tnsr_partial((int5){group*1024+tile*128,token}, output, q, 127, 0);
            }
            v_f32_st_tnsr_partial((int5){0,token}, scales, scale, 0, 0);
        }
        return;
    }
    for (int token = begin[1]; token < end[1]; ++token)
    for (int owner = begin[0]; owner < end[0]; ++owner) {
        float64 maximum = 0;
        bfloat128 owned = 0;
        for (int group = 0; group < groups; ++group) {
            #pragma loop_unroll(4)
            for (int tile = 0; tile < 8; ++tile) {
                const bfloat128 value = roundtrip(product, weight, activation, group, tile, token);
                const float128 wide = v_convert_bf16_to_f32_all_b(value);
                maximum = v_f32_max_b(maximum, v_f32_abs_b(wide.v1));
                maximum = v_f32_max_b(maximum, v_f32_abs_b(wide.v2));
                if (owner == group * 8 + tile) owned = value;
            }
        }
        maximum = v_f32_reduce_max(maximum);
        const uint64 bits = as_uint64(maximum);
        int64 power = convert_uint64_to_int64(bits >> 23, 0) - 134;
        power += v_i32_sel_grt_u32_b(bits & 0x7fffff, 0x700000, 1, 0);
        power = v_i32_sel_eq_f32_b(maximum, 0.0f, 0, power);
        const float64 scale = as_float64((power + 127) << 23);
        const float64 inverse = as_float64((127 - power) << 23);
        const float128 pair = {inverse, inverse};
        const bfloat128 value = owned * v_convert_f32_to_bf16_all_b(pair);
        minifloat256 q = v_convert_bf16_to_f8_b(value, 0, SW_RHNE | SW_CLIP_FP, (minifloat256)0);
        const minifloat256 sparse = q;
        q = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
        q = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, q);
        q = v_f8_mov_dual_group_pack_b(q, SW_PACK21, (minifloat256)0);
        uchar256 raw = *((uchar256*)&q);
        raw = v_u8_sel_eq_u8_b(raw & 0x78, 0, 0, raw);
        q = *((minifloat256*)&raw);
        const int5 at = {owner * 128, token, 0, 0, 0};
        v_f8_st_tnsr_partial(at, output, q, 127, 0);
        if (owner == 0) {
            const int5 scale_at = {0, token, 0, 0, 0};
            v_f32_st_tnsr_partial(scale_at, scales, scale, 0, 0);
        }
    }
}
