// SPDX-License-Identifier: Apache-2.0
// FP32 MLA PV product [32,512] -> exact BF16 boundary -> inverse RoPE ->
// E4M3 [4,1,4096].  The standalone path first writes the FP32 MME result as
// BF16 and immediately reloads it in the inverse-RoPE/wo_a quantizer.  Round
// in registers here so the arithmetic contract stays identical while the
// transient BF16 tensor remains inside the compound graph.
#pragma clang fp contract(off)
#define DSV4_ROPE_INVERSE 1
#define DSV4_QNORM_HELPERS_ONLY 1
#define DSV4_ROPE_SECOND_TERM_FMA 1
#include "deepseek_v4_qnorm_rope_kv_pack_bf16.h"

static inline bfloat128 woa_replace_upper_half(
        bfloat128 original, bfloat128 replacement) {
    original = v_bf16_mov_dual_group_b(
        replacement, 0xFFFFFFFF, 0, 2, MkWr(1, 1), original);
    return v_bf16_mov_dual_group_b(
        replacement, 0xFFFFFFFF, 1, 3, MkWr(1, 1), original);
}

void main(tensor product, tensor positions, tensor phase,
          tensor output, tensor scales) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    bfloat128 values[32];
    for (int group = start[1]; group < end[1]; ++group) {
        const int position = s_i32_ld_g(
            gen_addr((int5){0,0,0,0,0}, positions));
        float64 maximum = 0;
        #pragma loop_unroll(4)
        for (int tile = 0; tile < 32; ++tile) {
            const int head = group * 8 + tile / 4;
            const int offset = (tile & 3) * 128;
            int5 at = {offset, head, 0, 0, 0};
            float128 source;
            source.v1 = v_f32_ld_tnsr_b(at, product);
            at[0] += 64;
            source.v2 = v_f32_ld_tnsr_b(at, product);
            // Match the standalone cast_f32_to_bf16 node before any RoPE
            // arithmetic.  This is a semantic boundary, not an approximation.
            values[tile] = convert_float128_to_bfloat128(
                source, SW_RHNE | SW_LINEAR);
            if ((tile & 3) == 3) {
                const float64 expanded =
                    convert_bfloat128_to_float128(
                        values[tile], SW_LINEAR).v2;
                float128 rotated = {0};
                rotated.v1 = dsv4_qkv_apply_pairwise_rope_f32(
                    expanded, phase, position);
                // Preserve the standalone inverse-RoPE BF16 boundary before
                // dynamic FP8 quantization.
                values[tile] = woa_replace_upper_half(
                    values[tile],
                    convert_float128_to_bfloat128(
                        rotated, SW_RHNE | SW_LINEAR));
            }
            const float128 wide =
                v_convert_bf16_to_f32_all_b(values[tile]);
            maximum = v_f32_max_b(maximum, v_f32_abs_b(wide.v1));
            maximum = v_f32_max_b(maximum, v_f32_abs_b(wide.v2));
        }
        maximum = v_f32_reduce_max(maximum);
        const uint64 bits = as_uint64(maximum);
        int64 power = convert_uint64_to_int64(bits >> 23, 0) - 134;
        power += v_i32_sel_grt_u32_b(
            bits & 0x7fffff, 0x700000, 1, 0);
        power = v_i32_sel_eq_f32_b(maximum, 0.0f, 0, power);
        const float64 scale = as_float64((power + 127) << 23);
        const float64 inverse = as_float64((127 - power) << 23);
        const int5 scale_at = {0, 0, group, 0, 0};
        v_f32_st_tnsr_partial(scale_at, scales, scale, 0, 0);
        #pragma loop_unroll(2)
        for (int tile = 0; tile < 32; ++tile) {
            const float128 wide =
                v_convert_bf16_to_f32_all_b(values[tile]);
            minifloat256 q = 0;
            q = v_convert_f32_to_f8_b(
                wide.v1 * inverse, 0, SW_RHNE | SW_CLIP_FP, q);
            q = v_convert_f32_to_f8_b(
                wide.v2 * inverse, 2, SW_RHNE | SW_CLIP_FP, q);
            const minifloat256 sparse = q;
            q = v_f8_pack_b(
                sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            q = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, q);
            q = v_f8_mov_dual_group_pack_b(
                q, SW_PACK21, (minifloat256)0);
            uchar256 raw = *((uchar256*)&q);
            raw = v_u8_sel_eq_u8_b(raw & 0x78, 0, 0, raw);
            q = *((minifloat256*)&raw);
            const int5 out_at = {tile * 128, 0, group, 0, 0};
            v_f8_st_tnsr_partial(out_at, output, q, 127, 0);
        }
    }
}
