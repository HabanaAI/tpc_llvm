// SPDX-License-Identifier: Apache-2.0
// Keep MXFP4 data byte-wide while arranging lanes, then construct BF16 bits.

static inline ushort128 decode_normal_mxfp4_bits(ushort128 nibble, ushort128 scale)
{
    const ushort128 magnitude = v_u16_and_b(nibble, 7);
    const ushort128 sign = v_u16_shl_b(v_u16_and_b(nibble, 8), 12);
    ushort128 bits = v_u16_shl_b(scale, 7) + v_u16_shl_b(magnitude, 6) - 128;
    bits = v_u16_sel_eq_u16_b(magnitude, 1, bits - 64, bits);
    bits = v_u16_min_b(bits, 0x7f80);
    bits = v_u16_sel_eq_u16_b(magnitude, 0, 0, bits);
    return v_u16_or_b(bits, sign);
}

#define DECODE_AND_STORE_PART(PART) { \
    const uchar256 packed_part = v_u8_mov_dual_group_all_b( \
        packed_bytes, 0xffffffff, PART, PART, PART, PART, \
        MkWrA(3, 3, 3, 3), (uchar256){0}); \
    const uchar256 repeated = v_u8_shuffle_b( \
        packed_part, packed_indices, 0, (uchar256){0}); \
    const ushort128 nibble = v_u16_and_b( \
        v_u16_shr_b(*((ushort128*)&repeated), nibble_shifts), 15); \
    const uchar256 scale_bytes = v_u8_shuffle_b( \
        replicated_scale, v_u8_add_b(scale_indices, PART * 4), 0, \
        (uchar256){0}); \
    const ushort128 scale = v_u16_and_b(*((ushort128*)&scale_bytes), 255); \
    const ushort128 decoded = decode_normal_mxfp4_bits(nibble, scale); \
    v_bf16_st_tnsr(dst, output, *((bfloat128*)&decoded)); \
    dst[0] += 128; \
}

void main(tensor expert_ids, tensor packed, tensor scales, tensor output)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int experts = get_dim_size(packed, 2);
    const uchar256 lanes = (uchar256)V_LANE_ID_8;
    const uchar256 packed_indices = v_u8_or_b(v_u8_shr_b(lanes, 2), 0x80);
    const uchar256 scale_indices = v_u8_or_b(v_u8_shr_b(lanes, 6), 0x80);
    const short128 nibble_shifts = (short128)v_u16_shl_b(
        v_u16_and_b((ushort128)V_LANE_ID_16, 1), 2);
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
                const uchar256 packed_bytes = v_u8_ld_tnsr_b(
                    (int5){tile * 256, row, expert, 0, 0}, packed);
                const uchar256 raw_scale = v_u8_ld_tnsr_b(
                    (int5){tile * 16, row, expert, 0, 0}, scales);
                const uchar256 replicated_scale = v_u8_mov_dual_group_all_b(
                    raw_scale, 0xffffffff, 0, 0, 0, 0,
                    MkWrA(3, 3, 3, 3), (uchar256){0});
                DECODE_AND_STORE_PART(0)
                DECODE_AND_STORE_PART(1)
                DECODE_AND_STORE_PART(2)
                DECODE_AND_STORE_PART(3)
            }
        }
    }
}

#undef DECODE_AND_STORE_PART
