/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

#define DSV4_CACHE_HEAD_DIM 512
#define DSV4_CACHE_FP8_DIM 448
#define DSV4_CACHE_FP8_BLOCK 64
#define DSV4_CACHE_FP8_BLOCKS 7
#define DSV4_CACHE_DATA_BYTES 576
#define DSV4_CACHE_SCALE_BYTES 8
#define DSV4_CACHE_BYTES_PER_TOKEN 584

float dsv4_e8m0_scale(unsigned char encoded)
{
    // The cache stores ceil(log2(scale)) with an exponent bias of 127.
    // Values used by the model are normal FP32 powers of two. Preserve the
    // exact exp2(-127) behavior for the clamped zero code as well.
    unsigned bits = ((unsigned)encoded) << 23;
    if (encoded == 0) {
        bits = 0x00400000;
    }
    return as_float(bits);
}

bfloat256 dsv4_convert_e4m3fn(minifloat256 values)
{
    const uchar256 raw = as_uchar256(values);
    const uchar256 magnitude = v_u8_and_b(raw, 0x7f);
    const bool256 extended =
        v_u8_cmp_geq_b(magnitude, 120)
        & v_u8_cmp_leq_b(magnitude, 126);

    // Gaudi2's default FP8-143 mode reserves exponent 15 for Inf/NaN,
    // while PyTorch E4M3FN uses codes 120..126 for finite 256..448.
    // Decode those codes as exponent 14 and multiply the result by two.
    const uchar256 adjusted_raw =
        v_u8_sub_vb(raw, 8, 0, raw, extended, 0);
    const minifloat256 adjusted =
        *((minifloat256*)&adjusted_raw);
    bfloat256 converted =
        v_convert_f8_to_bf16_all_b(adjusted, SW_FP8_BIAS7);
    const bfloat256 magnitude_bf16 =
        v_convert_u8_to_bf16_all_b(magnitude);
    const bool128 extended_low =
        v_bf16_cmp_geq_b(magnitude_bf16.v1, 120.0f)
        & v_bf16_cmp_leq_b(magnitude_bf16.v1, 126.0f);
    const bool128 extended_high =
        v_bf16_cmp_geq_b(magnitude_bf16.v2, 120.0f)
        & v_bf16_cmp_leq_b(magnitude_bf16.v2, 126.0f);
    converted.v1 = v_bf16_mul_vb(
        converted.v1, 2.0f, 0, converted.v1, extended_low, 0);
    converted.v2 = v_bf16_mul_vb(
        converted.v2, 2.0f, 0, converted.v2, extended_high, 0);
    return converted;
}

void dsv4_dequant_gather_slot(
    tensor cache_storage_u8,
    int source_slot,
    int output_slot,
    int base_offset,
    int block_stride,
    int block_size,
    int slot_count,
    tensor output)
{
    const bfloat128 zero = 0.0f;

    if (source_slot < 0 || source_slot >= slot_count) {
        #pragma unroll (4)
        for (int chunk = 0; chunk < 4; ++chunk) {
            int5 output_coords = {chunk * 128, output_slot, 0, 0, 0};
            v_bf16_st_tnsr(output_coords, output, zero);
        }
        return;
    }

    const int block = source_slot / block_size;
    const int position = source_slot - block * block_size;
    const int data_offset = position * DSV4_CACHE_DATA_BYTES;
    const int scale_offset =
        block_size * DSV4_CACHE_DATA_BYTES
        + position * DSV4_CACHE_SCALE_BYTES;

    const int block_offset = base_offset + block * block_stride;
    int5 cache_coords = {
        block_offset + data_offset, 0, 0, 0, 0};
    const minifloat256 fp8_low = v_f8_ld_tnsr_b(
        cache_coords,
        cache_storage_u8,
        SW_DT_OVERRIDE | SW_DT_FP8_143);
    cache_coords[0] = block_offset + data_offset + 256;
    const minifloat256 fp8_high = v_f8_ld_tnsr_b(
        cache_coords,
        cache_storage_u8,
        SW_DT_OVERRIDE | SW_DT_FP8_143);

    const bfloat256 fp8_low_bf16 =
        dsv4_convert_e4m3fn(fp8_low);
    const bfloat256 fp8_high_bf16 =
        dsv4_convert_e4m3fn(fp8_high);
    float128 chunks[4];
    chunks[0] = v_convert_bf16_to_f32_all_b(fp8_low_bf16.v1);
    chunks[1] = v_convert_bf16_to_f32_all_b(fp8_low_bf16.v2);
    chunks[2] = v_convert_bf16_to_f32_all_b(fp8_high_bf16.v1);
    chunks[3] = v_convert_bf16_to_f32_all_b(fp8_high_bf16.v2);

    float scales[DSV4_CACHE_FP8_BLOCKS];
    #pragma unroll (DSV4_CACHE_FP8_BLOCKS)
    for (int scale_index = 0;
         scale_index < DSV4_CACHE_FP8_BLOCKS;
         ++scale_index) {
        cache_coords[0] = block_offset + scale_offset + scale_index;
        const unsigned char encoded =
            s_u8_ld_g(gen_addr(cache_coords, cache_storage_u8));
        scales[scale_index] = dsv4_e8m0_scale(encoded);
    }

    const uint64 fp32_lanes = V_LANE_ID_32;
    const float64 scale_01 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[0], scales[1]);
    const float64 scale_23 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[2], scales[3]);
    const float64 scale_45 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[4], scales[5]);
    chunks[0].v1 *= scale_01;
    chunks[0].v2 *= scale_01;
    chunks[1].v1 *= scale_23;
    chunks[1].v2 *= scale_23;
    chunks[2].v1 *= scale_45;
    chunks[2].v2 *= scale_45;
    chunks[3].v1 *= scales[6];
    chunks[3].v2 *= scales[6];

    cache_coords[0] =
        block_offset + data_offset + DSV4_CACHE_FP8_DIM;
    const uchar256 rope_raw =
        v_u8_ld_tnsr_b(cache_coords, cache_storage_u8);
    const bfloat128 rope_and_padding =
        *((bfloat128*)&rope_raw);
    const float128 rope_f32 =
        v_convert_bf16_to_f32_all_b(rope_and_padding);
    const float64 rope_even_high =
        v_element_shift_up_f32(rope_f32.v1, 32);
    const float64 rope_odd_high =
        v_element_shift_up_f32(rope_f32.v2, 32);
    chunks[3].v1 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, chunks[3].v1, rope_even_high);
    chunks[3].v2 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, chunks[3].v2, rope_odd_high);

    #pragma unroll (4)
    for (int chunk = 0; chunk < 4; ++chunk) {
        const bfloat128 result =
            v_convert_f32_to_bf16_all_b(chunks[chunk], SW_RHNE);
        int5 output_coords = {chunk * 128, output_slot, 0, 0, 0};
        v_bf16_st_tnsr(output_coords, output, result);
    }
}

#ifndef DSV4_DEQUANT_HELPERS_ONLY
void main(
    tensor cache_storage_u8,
    tensor cache_geometry,
    tensor indices,
    tensor output)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
    int5 geometry_coords = {0, 0, 0, 0, 0};
    const int base_offset =
        s_i32_ld_g(gen_addr(geometry_coords, cache_geometry));
    geometry_coords[0] = 1;
    const int block_stride =
        s_i32_ld_g(gen_addr(geometry_coords, cache_geometry));
    geometry_coords[0] = 2;
    const int block_size =
        s_i32_ld_g(gen_addr(geometry_coords, cache_geometry));
    geometry_coords[0] = 3;
    const int block_count =
        s_i32_ld_g(gen_addr(geometry_coords, cache_geometry));
    const int slot_count = block_count * block_size;

    for (int output_slot = index_start[1];
         output_slot < index_end[1];
         ++output_slot) {
        int5 index_coords = {output_slot, 0, 0, 0, 0};
        const int source_slot =
            s_i32_ld_g(gen_addr(index_coords, indices));
        dsv4_dequant_gather_slot(
            cache_storage_u8,
            source_slot,
            output_slot,
            base_offset,
            block_stride,
            block_size,
            slot_count,
            output);
    }
}
#endif
