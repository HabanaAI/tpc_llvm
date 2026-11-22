// SPDX-License-Identifier: Apache-2.0
// Runtime expert addressing follows 1cat-vllm's SM70 QPN M1 implementation.
// Keep the checkpoint's row-major packed layout and construct BF16 bits
// directly: no FP8 conversion and no selected packed-weight temporary.

#ifndef DSV4_MXFP4_NORMAL_SCALES
#define DSV4_MXFP4_NORMAL_SCALES 0
#endif

static inline ushort128 decode_bits(ushort128 nibble, ushort128 scale)
{
    const ushort128 magnitude = v_u16_and_b(nibble, 7);
    const ushort128 sign = v_u16_shl_b(v_u16_and_b(nibble, 8), 12);
    // The E2M1 exponent/mantissa bits map to adjacent BF16 bit fields.
    // Adding the scale bias at once avoids separating and recombining them.
    ushort128 bits = v_u16_shl_b(scale, 7) + v_u16_shl_b(magnitude, 6) - 128;
    bits = v_u16_sel_eq_u16_b(magnitude, 1, bits - 64, bits);
    bits = v_u16_min_b(bits, 0x7f80);
#if !DSV4_MXFP4_NORMAL_SCALES
    // E8M0 code zero is 2^-127, not zero. Preserve BF16 subnormals.
    const ushort128 subnormal = v_u16_sel_less_u16_b(
        magnitude, 4, v_u16_shl_b(magnitude, 5), bits);
    bits = v_u16_sel_eq_u16_b(scale, 0, subnormal, bits);
    const ushort128 half_subnormal = v_u16_sel_eq_u16_b(
        magnitude, 1, 0x0040, bits);
    bits = v_u16_sel_eq_u16_b(scale, 1, half_subnormal, bits);
#endif
    bits = v_u16_sel_eq_u16_b(magnitude, 0, 0, bits);
#if !DSV4_MXFP4_NORMAL_SCALES
    bits = v_u16_sel_eq_u16_b(scale, 255, 0x7fc0, bits);
#endif
    return v_u16_or_b(bits, sign);
}

void main(tensor expert_ids, tensor packed, tensor scales, tensor output)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int experts = get_dim_size(packed, 2);
    for (int slot = start[2]; slot < end[2]; ++slot) {
        const int expert = s_i32_ld_g(gen_addr((int5){slot, 0, 0, 0, 0}, expert_ids));
        for (int row = start[1]; row < end[1]; ++row) {
            for (int tile = start[0]; tile < end[0]; ++tile) {
                int5 dst = {tile * 512, row, slot, 0, 0};
                if (expert < 0 || expert >= experts) {
                    for (int part = 0; part < 4; ++part) {
                        v_bf16_st_tnsr(dst, output, (bfloat128){0});
                        dst[0] += 128;
                    }
                    continue;
                }
                const uchar256 bytes = v_u8_ld_tnsr_b(
                    (int5){tile * 256, row, expert, 0, 0}, packed);
                const uchar256 raw_scale = v_u8_ld_tnsr_b(
                    (int5){tile * 16, row, expert, 0, 0}, scales);
                const uchar256 replicated = v_u8_mov_dual_group_all_b(
                    raw_scale, 0xffffffff, 0, 0, 0, 0,
                    MkWrA(0b11, 0b11, 0b11, 0b11), (uchar256){0});
                const uchar256 indices = v_u8_or_b(
                    v_u8_shr_b((uchar256)V_LANE_ID_8, 4), 0x80);
                const ushort256 scale = convert_uchar256_to_ushort256(
                    v_u8_shuffle_b(replicated, indices, 0, (uchar256){0}), SW_LINEAR);
                const ushort256 low = convert_uchar256_to_ushort256(
                    v_u8_and_b(bytes, 15), SW_LINEAR);
                const ushort256 high = convert_uchar256_to_ushort256(
                    v_u8_shr_b(bytes, 4), SW_LINEAR);
                const uint128 lo0 = convert_ushort128_to_uint128(decode_bits(low.v1, scale.v1), SW_LINEAR);
                const uint128 lo1 = convert_ushort128_to_uint128(decode_bits(low.v2, scale.v2), SW_LINEAR);
                const uint128 hi0 = convert_ushort128_to_uint128(decode_bits(high.v1, scale.v1), SW_LINEAR);
                const uint128 hi1 = convert_ushort128_to_uint128(decode_bits(high.v2, scale.v2), SW_LINEAR);
                const uint64 interleaved[4] = {
                    v_u32_or_b(lo0.v1, v_u32_shl_b(hi0.v1, 16)),
                    v_u32_or_b(lo0.v2, v_u32_shl_b(hi0.v2, 16)),
                    v_u32_or_b(lo1.v1, v_u32_shl_b(hi1.v1, 16)),
                    v_u32_or_b(lo1.v2, v_u32_shl_b(hi1.v2, 16)),
                };
                #pragma unroll (4)
                for (int part = 0; part < 4; ++part) {
                    v_bf16_st_tnsr(dst, output, *((bfloat128*)&interleaved[part]));
                    dst[0] += 128;
                }
            }
        }
    }
}
