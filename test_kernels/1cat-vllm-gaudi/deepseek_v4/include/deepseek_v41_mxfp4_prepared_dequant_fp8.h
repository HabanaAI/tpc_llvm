// SPDX-License-Identifier: Apache-2.0
// Decode block-major Q16/S16 directly into E4M3 SRAM tiles.

#define DSV4_MXFP4_PREPARED_FP8_N_TILE 128

static inline ushort128_pair_t dsv4_prepared_fp8_lookup_scaled(
    uchar256 unpacked,
    ushort128 normalized_scale_code,
    uchar256 table)
{
    const uchar256 directions = v_u8_or_b(unpacked, 0x80);
    const uchar256 base = v_u8_shuffle_b(
        table, directions, 0, (uchar256){0});
    const ushort128 base_words = *((ushort128*)&base);
    const ushort128 unpacked_words = *((ushort128*)&unpacked);
    const ushort128 magnitude0 = v_u16_and_b(unpacked_words, 7);
    const ushort128 magnitude1 = v_u16_and_b(
        v_u16_shr_b(unpacked_words, 8), 7);
    const ushort128 base0 = v_u16_and_b(base_words, 0xff);
    const ushort128 base1 = v_u16_shr_b(base_words, 8);
    const ushort128 exponent = v_u16_shl_b(
        normalized_scale_code - 127, 3);
    ushort128_pair_t encoded;
    encoded.v1 = v_u16_sel_eq_u16_b(
        magnitude0, 0, base0, base0 + exponent);
    encoded.v2 = v_u16_sel_eq_u16_b(
        magnitude1, 0, base1, base1 + exponent);
    return encoded;
}

void main(
    tensor expert_ids,
    tensor q16,
    tensor s16,
    tensor channel_scale,
    tensor lookup,
    tensor output,
    tensor selected_scale)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int experts = get_dim_size(q16, 2);
    const int k_pairs = get_dim_size(q16, 0) / 64;
    const int groups = k_pairs / 16;
    const uchar256 table = v_u8_ld_tnsr_b(
        (int5){0, 0, 0, 0, 0}, lookup);

    for (int slot = start[1]; slot < end[1]; ++slot) {
        const int expert = s_i32_ld_g(
            gen_addr((int5){slot, 0, 0, 0, 0}, expert_ids));
        for (int n_block = start[0]; n_block < end[0]; ++n_block) {
            const int row = n_block * DSV4_MXFP4_PREPARED_FP8_N_TILE;
            if (expert < 0 || expert >= experts) {
                const minifloat256 zero_fp8 = 0;
                for (int pair = 0; pair < k_pairs; ++pair) {
                    v_f8_st_tnsr_partial(
                        (int5){row, pair * 2, slot, 0, 0},
                        output, zero_fp8, 127, 0);
                    v_f8_st_tnsr_partial(
                        (int5){row, pair * 2 + 1, slot, 0, 0},
                        output, zero_fp8, 127, 0);
                }
                const float64 one = 1.0f;
                int5 scale_dst = {row, 0, slot, 0, 0};
                v_f32_st_tnsr(scale_dst, selected_scale, one);
                scale_dst[0] += 64;
                v_f32_st_tnsr(scale_dst, selected_scale, one);
                continue;
            }

            const bfloat128 row_scale = v_bf16_ld_tnsr_b(
                (int5){0, n_block, expert, 0, 0}, channel_scale);
            const ushort128 row_scale_code = v_u16_shr_b(
                *((ushort128*)&row_scale), 7);
            // Widen the stored BF16 bit patterns in memory order. A vector
            // floating conversion produces even/odd lanes, not contiguous
            // halves, and cannot be written to two adjacent F32 tensors.
            const uint64 low_bits = v_u32_ld_tnsr_b(
                (int5){0, n_block, expert, 0, 0}, channel_scale,
                SW_UNPACK | SW_UNPCK_16_TO_32);
            const uint64 high_bits = v_u32_ld_tnsr_b(
                (int5){64, n_block, expert, 0, 0}, channel_scale,
                SW_UNPACK | SW_UNPCK_16_TO_32);
            const uint64 low_f32 = v_u32_shl_b(low_bits, 16);
            const uint64 high_f32 = v_u32_shl_b(high_bits, 16);
            const float64 scale_low = *((float64*)&low_f32);
            const float64 scale_high = *((float64*)&high_f32);
            int5 scale_dst = {row, 0, slot, 0, 0};
            v_f32_st_tnsr(scale_dst, selected_scale, scale_low);
            scale_dst[0] += 64;
            v_f32_st_tnsr(scale_dst, selected_scale, scale_high);

            for (int group = 0; group < groups; ++group) {
                const bfloat128 original_scale = v_bf16_ld_tnsr_b(
                    (int5){group * 128, n_block, expert, 0, 0}, s16);
                const ushort128 original_code = v_u16_shr_b(
                    *((ushort128*)&original_scale), 7);
                const ushort128 rebased = original_code + 127;
                ushort128 normalized_code = v_u16_sel_less_u16_b(
                    rebased,
                    row_scale_code,
                    0,
                    rebased - row_scale_code);
                normalized_code = v_u16_min_b(normalized_code, 255);
                #pragma unroll (16)
                for (int part = 0; part < 16; ++part) {
                    const int pair = group * 16 + part;
                    const uchar256 unpacked = v_u8_ld_tnsr_b(
                        (int5){pair * 64, n_block, expert, 0, 0},
                        q16,
                        SW_UNPACK | SW_UNPCK_4_TO_8);
                    const ushort128_pair_t encoded =
                        dsv4_prepared_fp8_lookup_scaled(
                            unpacked,
                            normalized_code,
                            table);
                    v_u16_st_tnsr_partial(
                        (int5){row, pair * 2, slot, 0, 0},
                        output,
                        encoded.v1,
                        127,
                        0,
                        SW_PACK | SW_PCK_16_TO_8);
                    v_u16_st_tnsr_partial(
                        (int5){row, pair * 2 + 1, slot, 0, 0},
                        output,
                        encoded.v2,
                        127,
                        0,
                        SW_PACK | SW_PCK_16_TO_8);
                }
            }
        }
    }
}

#undef DSV4_MXFP4_PREPARED_FP8_N_TILE
