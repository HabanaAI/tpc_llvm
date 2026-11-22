/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

#define DSV4_MXFP4_FP8_TOPK 6
#define DSV4_MXFP4_FP8_EXPERTS 256
#define DSV4_MXFP4_FP8_W13_ROWS 2048
#define DSV4_MXFP4_FP8_W13_PACKED_COLS 2048
#define DSV4_MXFP4_FP8_W13_COLS 4096
#define DSV4_MXFP4_FP8_W2_ROWS 4096
#define DSV4_MXFP4_FP8_W2_PACKED_COLS 512
#define DSV4_MXFP4_FP8_W2_COLS 1024
#define DSV4_MXFP4_FP8_SCALE_GROUP 32
#define DSV4_MXFP4_FP8_PACKED_PER_VECTOR 256
#define DSV4_MXFP4_FP8_SCALE_PER_VECTOR 16

static inline bfloat256 dsv4_mxfp4_scale_vector(
    tensor scales,
    int scale_offset,
    int row,
    int expert)
{
    int5 coords = {scale_offset, row, expert, 0, 0};
    const uchar256 raw_codes = v_u8_ld_tnsr_b(coords, scales);
    const uchar256 replicated_codes = v_u8_mov_dual_group_all_b(
        raw_codes,
        0xffffffff,
        0,
        0,
        0,
        0,
        MkWrA(0b11, 0b11, 0b11, 0b11),
        (uchar256){0});
    const uchar256 lanes = (uchar256)V_LANE_ID_8;
    const uchar256 broadcast_indices = v_u8_or_b(
        v_u8_shr_b(lanes, 4), 0x80);
    const uchar256 codes = v_u8_shuffle_b(
        replicated_codes, broadcast_indices, 0, (uchar256){0});
    const ushort256 wide_codes = convert_uchar256_to_ushort256(
        codes, SW_LINEAR | SW_RHNE);
    const ushort128 low_bits = v_u16_shl_b(
        v_u16_add_b(wide_codes.v1, 1), 7);
    const ushort128 high_bits = v_u16_shl_b(
        v_u16_add_b(wide_codes.v2, 1), 7);

    // The FP8 MoE receives d_scale_w=0.5. The +1 exponent encodes twice
    // the E8M0 scale; the indexed W4A16 path cancels it after conversion.
    bfloat256 result;
    result.v1 = *((bfloat128*)&low_bits);
    result.v2 = *((bfloat128*)&high_bits);
    return result;
}

static inline bfloat256 dsv4_mxfp4_nibbles_to_bf16(
    uchar256 nibbles,
    bfloat256 scales)
{
    const uchar256 magnitudes = v_u8_and_b(nibbles, 0x07);
    const bfloat256 magnitudes_bf16 =
        v_convert_u8_to_bf16_all_b(magnitudes, SW_LINEAR);
    const bfloat256 nibbles_bf16 =
        v_convert_u8_to_bf16_all_b(nibbles, SW_LINEAR);
    bfloat256 values;

    values.v1 = magnitudes_bf16.v1 * 0.5f;
    values.v2 = magnitudes_bf16.v2 * 0.5f;
    values.v1 = v_bf16_sel_eq_bf16_b(
        magnitudes_bf16.v1, 5.0f, 3.0f, values.v1);
    values.v2 = v_bf16_sel_eq_bf16_b(
        magnitudes_bf16.v2, 5.0f, 3.0f, values.v2);
    values.v1 = v_bf16_sel_eq_bf16_b(
        magnitudes_bf16.v1, 6.0f, 4.0f, values.v1);
    values.v2 = v_bf16_sel_eq_bf16_b(
        magnitudes_bf16.v2, 6.0f, 4.0f, values.v2);
    values.v1 = v_bf16_sel_eq_bf16_b(
        magnitudes_bf16.v1, 7.0f, 6.0f, values.v1);
    values.v2 = v_bf16_sel_eq_bf16_b(
        magnitudes_bf16.v2, 7.0f, 6.0f, values.v2);
    values.v1 = v_bf16_sel_geq_bf16_b(
        nibbles_bf16.v1, 8.0f, -values.v1, values.v1);
    values.v2 = v_bf16_sel_geq_bf16_b(
        nibbles_bf16.v2, 8.0f, -values.v2, values.v2);
    values.v1 *= scales.v1;
    values.v2 *= scales.v2;

    return values;
}

static inline uchar256 dsv4_mxfp4_nibbles_to_fp8(
    uchar256 nibbles,
    bfloat256 scales)
{
    const bfloat256 values = dsv4_mxfp4_nibbles_to_bf16(
        nibbles, scales);

    const minifloat256 encoded = v_convert_bf16_to_f8_all_b(
        values, SW_LINEAR | SW_RHNE | SW_FP8_BIAS7);
    return as_uchar256(encoded);
}

static inline void dsv4_mxfp4_expand_vector(
    tensor packed,
    tensor scales,
    int packed_offset,
    int scale_offset,
    int row,
    int expert,
    int output_offset,
    int output_row,
    int output_expert,
    tensor output)
{
    int5 source = {packed_offset, row, expert, 0, 0};
    const uchar256 packed_values = v_u8_ld_tnsr_b(source, packed);
    const bfloat256 scale_values = dsv4_mxfp4_scale_vector(
        scales, scale_offset, row, expert);
    const uchar256 low = dsv4_mxfp4_nibbles_to_fp8(
        v_u8_and_b(packed_values, 0x0f), scale_values);
    const uchar256 high = dsv4_mxfp4_nibbles_to_fp8(
        v_u8_shr_b(packed_values, 4), scale_values);

    const ushort256 low_wide = convert_uchar256_to_ushort256(
        low, SW_LINEAR | SW_RHNE);
    const ushort256 high_wide = convert_uchar256_to_ushort256(
        high, SW_LINEAR | SW_RHNE);
    const ushort128 interleaved_low = v_u16_or_b(
        low_wide.v1, v_u16_shl_b(high_wide.v1, 8));
    const ushort128 interleaved_high = v_u16_or_b(
        low_wide.v2, v_u16_shl_b(high_wide.v2, 8));
    const uchar256 output_low = as_uchar256(interleaved_low);
    const uchar256 output_high = as_uchar256(interleaved_high);

    int5 destination = {
        output_offset, output_row, output_expert, 0, 0};
    v_f8_st_tnsr(
        destination, output, *((minifloat256*)&output_low));
    destination[0] += 256;
    v_f8_st_tnsr(
        destination, output, *((minifloat256*)&output_high));
}

#ifndef DSV4_MXFP4_DEQUANT_HELPERS_ONLY
void main(
    tensor expert_ids,
    tensor w13,
    tensor w2,
    tensor w13_scale,
    tensor w2_scale,
    tensor fp8_w13,
    tensor fp8_w2)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;

    for (int selected = index_start[1]; selected < index_end[1]; ++selected) {
        int5 id_coords = {selected, 0, 0, 0, 0};
        const int source_expert =
            s_i32_ld_g(gen_addr(id_coords, expert_ids));

        for (int row = index_start[0]; row < index_end[0]; ++row) {
            #pragma unroll (8)
            for (int chunk = 0; chunk < 8; ++chunk) {
                dsv4_mxfp4_expand_vector(
                    w13,
                    w13_scale,
                    chunk * DSV4_MXFP4_FP8_PACKED_PER_VECTOR,
                    chunk * DSV4_MXFP4_FP8_SCALE_PER_VECTOR,
                    row,
                    source_expert,
                    chunk * DSV4_MXFP4_FP8_PACKED_PER_VECTOR * 2,
                    row,
                    selected,
                    fp8_w13);
            }

            #pragma unroll (2)
            for (int row_half = 0; row_half < 2; ++row_half) {
                const int w2_row =
                    row + row_half * DSV4_MXFP4_FP8_W13_ROWS;
                #pragma unroll (2)
                for (int chunk = 0; chunk < 2; ++chunk) {
                    dsv4_mxfp4_expand_vector(
                        w2,
                        w2_scale,
                        chunk * DSV4_MXFP4_FP8_PACKED_PER_VECTOR,
                        chunk * DSV4_MXFP4_FP8_SCALE_PER_VECTOR,
                        w2_row,
                        source_expert,
                        chunk * DSV4_MXFP4_FP8_PACKED_PER_VECTOR * 2,
                        w2_row,
                        selected,
                        fp8_w2);
                }
            }
        }
    }
}
#endif
