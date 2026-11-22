// SPDX-License-Identifier: Apache-2.0
// BF16 [T,32,512] -> inverse RoPE -> E4M3 [4,T,4096].  The inverse
// rotation keeps the model's BF16 boundary before dynamic quantization, but
// does not materialize and reread the 32x512 intermediate tensor.
#pragma clang fp contract(off)
#define DSV4_ROPE_INVERSE 1
#define DSV4_QNORM_HELPERS_ONLY 1
#define DSV4_ROPE_SECOND_TERM_FMA 1
#include "deepseek_v4_qnorm_rope_kv_pack_bf16.h"

// A 128-lane BF16 register contains four 32-lane dual groups.  The converted
// 64-lane RoPE tail occupies groups 0/1; replace groups 2/3 of the original
// tile so the regular wo_a conversion and FP8 packing see exactly the same
// register layout as a standalone tensor store followed by reload.
static inline bfloat128 woa_replace_upper_half(
        bfloat128 original, bfloat128 replacement) {
    original = v_bf16_mov_dual_group_b(
        replacement, 0xFFFFFFFF, 0, 2, MkWr(1, 1), original);
    return v_bf16_mov_dual_group_b(
        replacement, 0xFFFFFFFF, 1, 3, MkWr(1, 1), original);
}

void main(tensor input, tensor positions, tensor phase, tensor output, tensor scales) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    bfloat128 values[32];
    for (int group = start[1]; group < end[1]; ++group) {
        for (int token = start[0]; token < end[0]; ++token) {
            const int position = s_i32_ld_g(gen_addr((int5){token,0,0,0,0}, positions));
            float64 maximum = 0;
            #pragma loop_unroll(4)
            for (int tile = 0; tile < 32; ++tile) {
                const int head = group * 8 + tile / 4;
                const int offset = (tile & 3) * 128;
                const int5 at = {offset, head, token, 0, 0};
                values[tile] = v_bf16_ld_tnsr_b(at, input);
                if ((tile & 3) == 3) {
                    const int5 tail_at = {offset + 64, head, token, 0, 0};
                    const bfloat128 tail = v_bf16_ld_tnsr_partial_b(tail_at, input, 63, 0);
                    const float64 expanded = convert_bfloat128_to_float128(tail, SW_LINEAR).v1;
                    float128 rotated = {0};
                    rotated.v1 = dsv4_qkv_apply_pairwise_rope_f32(
                        expanded, phase, position);
                    // Preserve the standalone inverse-RoPE BF16 rounding
                    // boundary before dynamic FP8 quantization.
                    values[tile] = woa_replace_upper_half(
                        values[tile],
                        convert_float128_to_bfloat128(rotated, SW_LINEAR));
                }
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
            const int5 scale_at = {0, token, group, 0, 0};
            v_f32_st_tnsr_partial(scale_at, scales, scale, 0, 0);
            #pragma loop_unroll(2)
            for (int tile = 0; tile < 32; ++tile) {
                const float128 wide = v_convert_bf16_to_f32_all_b(values[tile]);
                minifloat256 q = 0;
                q = v_convert_f32_to_f8_b(wide.v1 * inverse, 0, SW_RHNE | SW_CLIP_FP, q);
                q = v_convert_f32_to_f8_b(wide.v2 * inverse, 2, SW_RHNE | SW_CLIP_FP, q);
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
