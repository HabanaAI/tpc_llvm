/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

#define DSV4_PAGED_HEAD_DIM 512
#define DSV4_PAGED_FP8_DIM 448
#define DSV4_PAGED_FP8_BLOCKS 7
#define DSV4_PAGED_DATA_BYTES 576
#define DSV4_PAGED_SCALE_BYTES 8
#ifndef DSV4_PAGED_NATIVE_FP8_QK
#define DSV4_PAGED_NATIVE_FP8_QK 1
#endif
#define DSV4_PAGED_DEBUG_QUERY_CONVERT 0
#define DSV4_PAGED_DEBUG_MAC_PARTS 0
#ifndef DSV4_PAGED_LOCAL_TOPK
#define DSV4_PAGED_LOCAL_TOPK 0
#endif
#ifndef DSV4_PAGED_SEQUENTIAL_TOPK
#define DSV4_PAGED_SEQUENTIAL_TOPK 0
#endif
#ifndef DSV4_PAGED_FUSED_QNORM
#define DSV4_PAGED_FUSED_QNORM 0
#endif
#ifndef DSV4_PAGED_SWA_ONLY
#define DSV4_PAGED_SWA_ONLY 0
#endif
#ifndef DSV4_PAGED_HEADS_PER_PROGRAM
#define DSV4_PAGED_HEADS_PER_PROGRAM 1
#endif
#ifndef DSV4_PAGED_SPLITKV_PARTIAL
#define DSV4_PAGED_SPLITKV_PARTIAL 0
#endif
#ifndef DSV4_PAGED_SPLITKV_TILED
#define DSV4_PAGED_SPLITKV_TILED 0
#endif
#ifndef DSV4_PAGED_SPLITKV_TILE_SIZE
#define DSV4_PAGED_SPLITKV_TILE_SIZE 8
#endif
#ifndef DSV4_PAGED_SPLITKV_CACHE_BF16_VALUES
#define DSV4_PAGED_SPLITKV_CACHE_BF16_VALUES 0
#endif
#ifndef DSV4_PAGED_SPLITKV_CACHE_BF16_CHUNKS
#define DSV4_PAGED_SPLITKV_CACHE_BF16_CHUNKS 4
#endif
#if DSV4_PAGED_FUSED_QNORM
#define DSV4_QNORM_HELPERS_ONLY 1
#include "deepseek_v4_qnorm_rope_kv_pack_bf16.h"
#undef DSV4_QNORM_HELPERS_ONLY
#endif

typedef struct {
    float64 score;
    float128 values[4];
} dsv4_paged_token;

typedef struct {
    float64 scores[2];
    float128 values[4];
} dsv4_paged_pair_token;

float dsv4_paged_e8m0_scale(unsigned char encoded)
{
    unsigned bits = ((unsigned)encoded) << 23;
    if (encoded == 0) {
        bits = 0x00400000;
    }
    return as_float(bits);
}

bfloat256 dsv4_paged_convert_e4m3fn(minifloat256 values)
{
    const uchar256 raw = as_uchar256(values);
    const uchar256 magnitude = v_u8_and_b(raw, 0x7f);
    const bool256 extended =
        v_u8_cmp_geq_b(magnitude, 120)
        & v_u8_cmp_leq_b(magnitude, 126);
    const bool256 invalid = v_u8_cmp_eq_b(magnitude, 127);
    uchar256 adjusted_raw =
        v_u8_sub_vb(raw, 8, 0, raw, extended, 0);
    adjusted_raw = v_u8_mov_vb(0, 0, adjusted_raw, invalid, 0);
    const minifloat256 adjusted = *((minifloat256*)&adjusted_raw);
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

float256 dsv4_paged_fp8_mac_e4m3fn(
    minifloat256 query,
    minifloat256 raw_values)
{
    const uchar256 raw = as_uchar256(raw_values);
    const uchar256 magnitude = v_u8_and_b(raw, 0x7f);
    const bool256 extended =
        v_u8_cmp_geq_b(magnitude, 120)
        & v_u8_cmp_leq_b(magnitude, 126);
    const bool256 invalid = v_u8_cmp_eq_b(magnitude, 127);
    uchar256 adjusted_raw =
        v_u8_sub_vb(raw, 8, 0, raw, extended, 0);
    adjusted_raw = v_u8_mov_vb(0, 0, adjusted_raw, invalid, 0);
    const uchar256 correction_raw =
        v_u8_mov_vb(adjusted_raw, 0, 0, extended, 0);
    const minifloat256 adjusted = *((minifloat256*)&adjusted_raw);
    const minifloat256 correction = *((minifloat256*)&correction_raw);
    float256 accum = {0};
    accum = v_f8_mac_acc32_b(
        query, adjusted, accum, SW_FP8_BIAS7);
    return v_f8_mac_acc32_b(
        query, correction, accum, SW_FP8_BIAS7);
}

dsv4_paged_token dsv4_paged_load_token(
    tensor cache_storage_u8,
    int base_offset,
    int block_stride,
    int block_size,
    int source_slot,
    tensor score_debug,
    int debug_head,
    int debug_batch,
    minifloat256 query_low,
    minifloat256 query_high,
    bfloat128 query_0,
    bfloat128 query_1,
    bfloat128 query_2,
    bfloat128 query_3)
{
    const int block = source_slot / block_size;
    const int position = source_slot - block * block_size;
    const int block_offset = base_offset + block * block_stride;
    const int data_offset = position * DSV4_PAGED_DATA_BYTES;
    const int scale_offset =
        block_size * DSV4_PAGED_DATA_BYTES
        + position * DSV4_PAGED_SCALE_BYTES;

    int5 cache_coords = {block_offset + data_offset, 0, 0, 0, 0};
    const minifloat256 fp8_low = v_f8_ld_tnsr_b(
        cache_coords,
        cache_storage_u8,
        SW_DT_OVERRIDE | SW_DT_FP8_143);
    cache_coords[0] = block_offset + data_offset + 256;
    const minifloat256 fp8_high = v_f8_ld_tnsr_b(
        cache_coords,
        cache_storage_u8,
        SW_DT_OVERRIDE | SW_DT_FP8_143);

    float scales[DSV4_PAGED_FP8_BLOCKS];
    #pragma unroll (DSV4_PAGED_FP8_BLOCKS)
    for (int scale_index = 0;
         scale_index < DSV4_PAGED_FP8_BLOCKS;
         ++scale_index) {
        cache_coords[0] = block_offset + scale_offset + scale_index;
        const unsigned char encoded =
            s_u8_ld_g(gen_addr(cache_coords, cache_storage_u8));
        scales[scale_index] = dsv4_paged_e8m0_scale(encoded);
    }

    const float256 dot_low =
        dsv4_paged_fp8_mac_e4m3fn(query_low, fp8_low);
    const float256 dot_high =
        dsv4_paged_fp8_mac_e4m3fn(query_high, fp8_high);
    #if DSV4_PAGED_DEBUG_MAC_PARTS
    if (debug_head == 0) {
        const float64 dot_parts[8] = {
            v_f32_reduce_add(dot_low.v1),
            v_f32_reduce_add(dot_low.v2),
            v_f32_reduce_add(dot_low.v3),
            v_f32_reduce_add(dot_low.v4),
            v_f32_reduce_add(dot_high.v1),
            v_f32_reduce_add(dot_high.v2),
            v_f32_reduce_add(dot_high.v3),
            v_f32_reduce_add(dot_high.v4)};
        #pragma unroll (8)
        for (int part = 0; part < 8; ++part) {
            int5 debug_coords = {part, debug_batch, 0, 0, 0};
            v_f32_st_tnsr_partial(
                debug_coords, score_debug, dot_parts[part], 0, 0);
        }
    }
    #endif
    const uint64 fp32_lanes = V_LANE_ID_32;
    const float64 low_scale_01 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[0], scales[1]);
    const float64 low_scale_23 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[2], scales[3]);
    const float64 high_scale_45 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[4], scales[5]);
    const float64 high_scale_6 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[6], 0.0f);
    float64 score_lanes =
        (dot_low.v1 + dot_low.v3) * low_scale_01
        + (dot_low.v2 + dot_low.v4) * low_scale_23
        + (dot_high.v1 + dot_high.v3) * high_scale_45
        + (dot_high.v2 + dot_high.v4) * high_scale_6;

    const bfloat256 fp8_low_bf16 =
        dsv4_paged_convert_e4m3fn(fp8_low);
    const bfloat256 fp8_high_bf16 =
        dsv4_paged_convert_e4m3fn(fp8_high);
    dsv4_paged_token token;
    token.values[0] =
        v_convert_bf16_to_f32_all_b(fp8_low_bf16.v1);
    token.values[1] =
        v_convert_bf16_to_f32_all_b(fp8_low_bf16.v2);
    token.values[2] =
        v_convert_bf16_to_f32_all_b(fp8_high_bf16.v1);
    token.values[3] =
        v_convert_bf16_to_f32_all_b(fp8_high_bf16.v2);

    const float64 scale_01 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[0], scales[1]);
    const float64 scale_23 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[2], scales[3]);
    const float64 scale_45 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[4], scales[5]);
    token.values[0].v1 *= scale_01;
    token.values[0].v2 *= scale_01;
    token.values[1].v1 *= scale_23;
    token.values[1].v2 *= scale_23;
    token.values[2].v1 *= scale_45;
    token.values[2].v2 *= scale_45;
    token.values[3].v1 *= scales[6];
    token.values[3].v2 *= scales[6];

    cache_coords[0] =
        block_offset + data_offset + DSV4_PAGED_FP8_DIM;
    const uchar256 rope_raw =
        v_u8_ld_tnsr_b(cache_coords, cache_storage_u8);
    const bfloat128 rope_and_padding = *((bfloat128*)&rope_raw);
    const float128 rope_f32 =
        v_convert_bf16_to_f32_all_b(rope_and_padding);
    const float64 rope_even_high =
        v_element_shift_up_f32(rope_f32.v1, 32);
    const float64 rope_odd_high =
        v_element_shift_up_f32(rope_f32.v2, 32);
    token.values[3].v1 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, token.values[3].v1, rope_even_high);
    token.values[3].v2 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, token.values[3].v2, rope_odd_high);

    #if DSV4_PAGED_NATIVE_FP8_QK
    float128 rope_key = {0};
    rope_key.v1 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, 0.0f, rope_even_high);
    rope_key.v2 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, 0.0f, rope_odd_high);
    const bfloat128 rope_key_bf16 =
        v_convert_f32_to_bf16_all_b(rope_key, SW_RHNE);
    float128 rope_dot = {0};
    rope_dot = v_bf16_mac_acc32_b(
        query_3, rope_key_bf16, rope_dot, 0);
    score_lanes += rope_dot.v1 + rope_dot.v2;
    #else
    score_lanes = 0.0f;
    const bfloat128 query_chunks[4] = {
        query_0, query_1, query_2, query_3};
    #pragma unroll (4)
    for (int dot_chunk = 0; dot_chunk < 4; ++dot_chunk) {
        const bfloat128 value_bf16 = v_convert_f32_to_bf16_all_b(
            token.values[dot_chunk], SW_RHNE);
        float128 bf16_dot = {0};
        bf16_dot = v_bf16_mac_acc32_b(
            query_chunks[dot_chunk], value_bf16, bf16_dot, 0);
        score_lanes += bf16_dot.v1 + bf16_dot.v2;
    }
    #endif
    token.score = v_f32_reduce_add(score_lanes);
    return token;
}

dsv4_paged_pair_token dsv4_paged_load_token_pair(
    tensor cache_storage_u8,
    int base_offset,
    int block_stride,
    int block_size,
    int source_slot,
    minifloat256 query_low_0,
    minifloat256 query_high_0,
    bfloat128 query_rope_0,
    minifloat256 query_low_1,
    minifloat256 query_high_1,
    bfloat128 query_rope_1)
{
    const int block = source_slot / block_size;
    const int position = source_slot - block * block_size;
    const int block_offset = base_offset + block * block_stride;
    const int data_offset = position * DSV4_PAGED_DATA_BYTES;
    const int scale_offset =
        block_size * DSV4_PAGED_DATA_BYTES
        + position * DSV4_PAGED_SCALE_BYTES;

    int5 cache_coords = {block_offset + data_offset, 0, 0, 0, 0};
    const minifloat256 fp8_low = v_f8_ld_tnsr_b(
        cache_coords,
        cache_storage_u8,
        SW_DT_OVERRIDE | SW_DT_FP8_143);
    cache_coords[0] = block_offset + data_offset + 256;
    const minifloat256 fp8_high = v_f8_ld_tnsr_b(
        cache_coords,
        cache_storage_u8,
        SW_DT_OVERRIDE | SW_DT_FP8_143);

    float scales[DSV4_PAGED_FP8_BLOCKS];
    #pragma unroll (DSV4_PAGED_FP8_BLOCKS)
    for (int scale_index = 0;
         scale_index < DSV4_PAGED_FP8_BLOCKS;
         ++scale_index) {
        cache_coords[0] = block_offset + scale_offset + scale_index;
        const unsigned char encoded =
            s_u8_ld_g(gen_addr(cache_coords, cache_storage_u8));
        scales[scale_index] = dsv4_paged_e8m0_scale(encoded);
    }

    const float256 dot_low_0 =
        dsv4_paged_fp8_mac_e4m3fn(query_low_0, fp8_low);
    const float256 dot_high_0 =
        dsv4_paged_fp8_mac_e4m3fn(query_high_0, fp8_high);
    const float256 dot_low_1 =
        dsv4_paged_fp8_mac_e4m3fn(query_low_1, fp8_low);
    const float256 dot_high_1 =
        dsv4_paged_fp8_mac_e4m3fn(query_high_1, fp8_high);
    const uint64 fp32_lanes = V_LANE_ID_32;
    const float64 low_scale_01 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[0], scales[1]);
    const float64 low_scale_23 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[2], scales[3]);
    const float64 high_scale_45 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[4], scales[5]);
    const float64 high_scale_6 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[6], 0.0f);
    float64 score_lanes_0 =
        (dot_low_0.v1 + dot_low_0.v3) * low_scale_01
        + (dot_low_0.v2 + dot_low_0.v4) * low_scale_23
        + (dot_high_0.v1 + dot_high_0.v3) * high_scale_45
        + (dot_high_0.v2 + dot_high_0.v4) * high_scale_6;
    float64 score_lanes_1 =
        (dot_low_1.v1 + dot_low_1.v3) * low_scale_01
        + (dot_low_1.v2 + dot_low_1.v4) * low_scale_23
        + (dot_high_1.v1 + dot_high_1.v3) * high_scale_45
        + (dot_high_1.v2 + dot_high_1.v4) * high_scale_6;

    const bfloat256 fp8_low_bf16 =
        dsv4_paged_convert_e4m3fn(fp8_low);
    const bfloat256 fp8_high_bf16 =
        dsv4_paged_convert_e4m3fn(fp8_high);
    dsv4_paged_pair_token token;
    token.values[0] =
        v_convert_bf16_to_f32_all_b(fp8_low_bf16.v1);
    token.values[1] =
        v_convert_bf16_to_f32_all_b(fp8_low_bf16.v2);
    token.values[2] =
        v_convert_bf16_to_f32_all_b(fp8_high_bf16.v1);
    token.values[3] =
        v_convert_bf16_to_f32_all_b(fp8_high_bf16.v2);

    const float64 scale_01 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[0], scales[1]);
    const float64 scale_23 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[2], scales[3]);
    const float64 scale_45 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[4], scales[5]);
    token.values[0].v1 *= scale_01;
    token.values[0].v2 *= scale_01;
    token.values[1].v1 *= scale_23;
    token.values[1].v2 *= scale_23;
    token.values[2].v1 *= scale_45;
    token.values[2].v2 *= scale_45;
    token.values[3].v1 *= scales[6];
    token.values[3].v2 *= scales[6];

    cache_coords[0] =
        block_offset + data_offset + DSV4_PAGED_FP8_DIM;
    const uchar256 rope_raw =
        v_u8_ld_tnsr_b(cache_coords, cache_storage_u8);
    const bfloat128 rope_and_padding = *((bfloat128*)&rope_raw);
    const float128 rope_f32 =
        v_convert_bf16_to_f32_all_b(rope_and_padding);
    const float64 rope_even_high =
        v_element_shift_up_f32(rope_f32.v1, 32);
    const float64 rope_odd_high =
        v_element_shift_up_f32(rope_f32.v2, 32);
    token.values[3].v1 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, token.values[3].v1, rope_even_high);
    token.values[3].v2 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, token.values[3].v2, rope_odd_high);

    float128 rope_key = {0};
    rope_key.v1 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, 0.0f, rope_even_high);
    rope_key.v2 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, 0.0f, rope_odd_high);
    const bfloat128 rope_key_bf16 =
        v_convert_f32_to_bf16_all_b(rope_key, SW_RHNE);
    float128 rope_dot_0 = {0};
    rope_dot_0 = v_bf16_mac_acc32_b(
        query_rope_0, rope_key_bf16, rope_dot_0, 0);
    score_lanes_0 += rope_dot_0.v1 + rope_dot_0.v2;
    float128 rope_dot_1 = {0};
    rope_dot_1 = v_bf16_mac_acc32_b(
        query_rope_1, rope_key_bf16, rope_dot_1, 0);
    score_lanes_1 += rope_dot_1.v1 + rope_dot_1.v2;
    token.scores[0] = v_f32_reduce_add(score_lanes_0);
    token.scores[1] = v_f32_reduce_add(score_lanes_1);
    return token;
}

#if DSV4_PAGED_SPLITKV_TILED

typedef struct {
    float64 scores[2];
} dsv4_paged_pair_scores;

typedef struct {
    float128 values[4];
} dsv4_paged_values;

typedef struct {
    bfloat128 values[DSV4_PAGED_SPLITKV_CACHE_BF16_CHUNKS];
} dsv4_paged_bf16_values;

dsv4_paged_pair_scores dsv4_paged_load_pair_scores(
    tensor cache_storage_u8,
    int base_offset,
    int block_stride,
    int block_size,
    int source_slot,
    minifloat256 query_low_0,
    minifloat256 query_high_0,
    bfloat128 query_rope_0,
    minifloat256 query_low_1,
    minifloat256 query_high_1,
    bfloat128 query_rope_1)
{
    const int block = source_slot / block_size;
    const int position = source_slot - block * block_size;
    const int block_offset = base_offset + block * block_stride;
    const int data_offset = position * DSV4_PAGED_DATA_BYTES;
    const int scale_offset =
        block_size * DSV4_PAGED_DATA_BYTES
        + position * DSV4_PAGED_SCALE_BYTES;

    int5 cache_coords = {block_offset + data_offset, 0, 0, 0, 0};
    const minifloat256 fp8_low = v_f8_ld_tnsr_b(
        cache_coords,
        cache_storage_u8,
        SW_DT_OVERRIDE | SW_DT_FP8_143);
    cache_coords[0] = block_offset + data_offset + 256;
    const minifloat256 fp8_high = v_f8_ld_tnsr_b(
        cache_coords,
        cache_storage_u8,
        SW_DT_OVERRIDE | SW_DT_FP8_143);

    float scales[DSV4_PAGED_FP8_BLOCKS];
    #pragma unroll (DSV4_PAGED_FP8_BLOCKS)
    for (int scale_index = 0;
         scale_index < DSV4_PAGED_FP8_BLOCKS;
         ++scale_index) {
        cache_coords[0] = block_offset + scale_offset + scale_index;
        const unsigned char encoded =
            s_u8_ld_g(gen_addr(cache_coords, cache_storage_u8));
        scales[scale_index] = dsv4_paged_e8m0_scale(encoded);
    }

    const float256 dot_low_0 =
        dsv4_paged_fp8_mac_e4m3fn(query_low_0, fp8_low);
    const float256 dot_high_0 =
        dsv4_paged_fp8_mac_e4m3fn(query_high_0, fp8_high);
    const float256 dot_low_1 =
        dsv4_paged_fp8_mac_e4m3fn(query_low_1, fp8_low);
    const float256 dot_high_1 =
        dsv4_paged_fp8_mac_e4m3fn(query_high_1, fp8_high);
    const uint64 fp32_lanes = V_LANE_ID_32;
    const float64 low_scale_01 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[0], scales[1]);
    const float64 low_scale_23 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[2], scales[3]);
    const float64 high_scale_45 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[4], scales[5]);
    const float64 high_scale_6 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[6], 0.0f);
    float64 score_lanes_0 =
        (dot_low_0.v1 + dot_low_0.v3) * low_scale_01
        + (dot_low_0.v2 + dot_low_0.v4) * low_scale_23
        + (dot_high_0.v1 + dot_high_0.v3) * high_scale_45
        + (dot_high_0.v2 + dot_high_0.v4) * high_scale_6;
    float64 score_lanes_1 =
        (dot_low_1.v1 + dot_low_1.v3) * low_scale_01
        + (dot_low_1.v2 + dot_low_1.v4) * low_scale_23
        + (dot_high_1.v1 + dot_high_1.v3) * high_scale_45
        + (dot_high_1.v2 + dot_high_1.v4) * high_scale_6;

    cache_coords[0] =
        block_offset + data_offset + DSV4_PAGED_FP8_DIM;
    const uchar256 rope_raw =
        v_u8_ld_tnsr_b(cache_coords, cache_storage_u8);
    const bfloat128 rope_and_padding = *((bfloat128*)&rope_raw);
    const float128 rope_f32 =
        v_convert_bf16_to_f32_all_b(rope_and_padding);
    const float64 rope_even_high =
        v_element_shift_up_f32(rope_f32.v1, 32);
    const float64 rope_odd_high =
        v_element_shift_up_f32(rope_f32.v2, 32);
    float128 rope_key = {0};
    rope_key.v1 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, 0.0f, rope_even_high);
    rope_key.v2 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, 0.0f, rope_odd_high);
    const bfloat128 rope_key_bf16 =
        v_convert_f32_to_bf16_all_b(rope_key, SW_RHNE);
    float128 rope_dot_0 = {0};
    rope_dot_0 = v_bf16_mac_acc32_b(
        query_rope_0, rope_key_bf16, rope_dot_0, 0);
    score_lanes_0 += rope_dot_0.v1 + rope_dot_0.v2;
    float128 rope_dot_1 = {0};
    rope_dot_1 = v_bf16_mac_acc32_b(
        query_rope_1, rope_key_bf16, rope_dot_1, 0);
    score_lanes_1 += rope_dot_1.v1 + rope_dot_1.v2;

    dsv4_paged_pair_scores scores;
    scores.scores[0] = v_f32_reduce_add(score_lanes_0);
    scores.scores[1] = v_f32_reduce_add(score_lanes_1);
    return scores;
}

dsv4_paged_values dsv4_paged_load_values(
    tensor cache_storage_u8,
    int base_offset,
    int block_stride,
    int block_size,
    int source_slot)
{
    const int block = source_slot / block_size;
    const int position = source_slot - block * block_size;
    const int block_offset = base_offset + block * block_stride;
    const int data_offset = position * DSV4_PAGED_DATA_BYTES;
    const int scale_offset =
        block_size * DSV4_PAGED_DATA_BYTES
        + position * DSV4_PAGED_SCALE_BYTES;

    int5 cache_coords = {block_offset + data_offset, 0, 0, 0, 0};
    const minifloat256 fp8_low = v_f8_ld_tnsr_b(
        cache_coords,
        cache_storage_u8,
        SW_DT_OVERRIDE | SW_DT_FP8_143);
    cache_coords[0] = block_offset + data_offset + 256;
    const minifloat256 fp8_high = v_f8_ld_tnsr_b(
        cache_coords,
        cache_storage_u8,
        SW_DT_OVERRIDE | SW_DT_FP8_143);

    float scales[DSV4_PAGED_FP8_BLOCKS];
    #pragma unroll (DSV4_PAGED_FP8_BLOCKS)
    for (int scale_index = 0;
         scale_index < DSV4_PAGED_FP8_BLOCKS;
         ++scale_index) {
        cache_coords[0] = block_offset + scale_offset + scale_index;
        const unsigned char encoded =
            s_u8_ld_g(gen_addr(cache_coords, cache_storage_u8));
        scales[scale_index] = dsv4_paged_e8m0_scale(encoded);
    }

    const bfloat256 fp8_low_bf16 =
        dsv4_paged_convert_e4m3fn(fp8_low);
    const bfloat256 fp8_high_bf16 =
        dsv4_paged_convert_e4m3fn(fp8_high);
    dsv4_paged_values token;
    token.values[0] =
        v_convert_bf16_to_f32_all_b(fp8_low_bf16.v1);
    token.values[1] =
        v_convert_bf16_to_f32_all_b(fp8_low_bf16.v2);
    token.values[2] =
        v_convert_bf16_to_f32_all_b(fp8_high_bf16.v1);
    token.values[3] =
        v_convert_bf16_to_f32_all_b(fp8_high_bf16.v2);

    const uint64 fp32_lanes = V_LANE_ID_32;
    const float64 scale_01 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[0], scales[1]);
    const float64 scale_23 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[2], scales[3]);
    const float64 scale_45 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[4], scales[5]);
    token.values[0].v1 *= scale_01;
    token.values[0].v2 *= scale_01;
    token.values[1].v1 *= scale_23;
    token.values[1].v2 *= scale_23;
    token.values[2].v1 *= scale_45;
    token.values[2].v2 *= scale_45;
    token.values[3].v1 *= scales[6];
    token.values[3].v2 *= scales[6];

    cache_coords[0] =
        block_offset + data_offset + DSV4_PAGED_FP8_DIM;
    const uchar256 rope_raw =
        v_u8_ld_tnsr_b(cache_coords, cache_storage_u8);
    const bfloat128 rope_and_padding = *((bfloat128*)&rope_raw);
    const float128 rope_f32 =
        v_convert_bf16_to_f32_all_b(rope_and_padding);
    const float64 rope_even_high =
        v_element_shift_up_f32(rope_f32.v1, 32);
    const float64 rope_odd_high =
        v_element_shift_up_f32(rope_f32.v2, 32);
    token.values[3].v1 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, token.values[3].v1, rope_even_high);
    token.values[3].v2 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, token.values[3].v2, rope_odd_high);
    return token;
}

typedef struct {
    float128 values[2];
} dsv4_paged_high_values;

dsv4_paged_high_values dsv4_paged_load_high_values(
    tensor cache_storage_u8,
    int base_offset,
    int block_stride,
    int block_size,
    int source_slot)
{
    const int block = source_slot / block_size;
    const int position = source_slot - block * block_size;
    const int block_offset = base_offset + block * block_stride;
    const int data_offset = position * DSV4_PAGED_DATA_BYTES;
    const int scale_offset =
        block_size * DSV4_PAGED_DATA_BYTES
        + position * DSV4_PAGED_SCALE_BYTES;
    int5 cache_coords = {
        block_offset + data_offset + 256, 0, 0, 0, 0};
    const minifloat256 fp8_high = v_f8_ld_tnsr_b(
        cache_coords,
        cache_storage_u8,
        SW_DT_OVERRIDE | SW_DT_FP8_143);
    float scales[3];
    #pragma unroll (3)
    for (int scale_index = 0; scale_index < 3; ++scale_index) {
        cache_coords[0] =
            block_offset + scale_offset + 4 + scale_index;
        scales[scale_index] = dsv4_paged_e8m0_scale(
            s_u8_ld_g(gen_addr(cache_coords, cache_storage_u8)));
    }
    const bfloat256 fp8_high_bf16 =
        dsv4_paged_convert_e4m3fn(fp8_high);
    dsv4_paged_high_values high_values;
    high_values.values[0] =
        v_convert_bf16_to_f32_all_b(fp8_high_bf16.v1);
    high_values.values[1] =
        v_convert_bf16_to_f32_all_b(fp8_high_bf16.v2);
    const uint64 fp32_lanes = V_LANE_ID_32;
    const float64 scale_45 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, scales[0], scales[1]);
    high_values.values[0].v1 *= scale_45;
    high_values.values[0].v2 *= scale_45;
    high_values.values[1].v1 *= scales[2];
    high_values.values[1].v2 *= scales[2];

    cache_coords[0] =
        block_offset + data_offset + DSV4_PAGED_FP8_DIM;
    const uchar256 rope_raw =
        v_u8_ld_tnsr_b(cache_coords, cache_storage_u8);
    const bfloat128 rope_and_padding = *((bfloat128*)&rope_raw);
    const float128 rope_f32 =
        v_convert_bf16_to_f32_all_b(rope_and_padding);
    const float64 rope_even_high =
        v_element_shift_up_f32(rope_f32.v1, 32);
    const float64 rope_odd_high =
        v_element_shift_up_f32(rope_f32.v2, 32);
    high_values.values[1].v1 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, high_values.values[1].v1, rope_even_high);
    high_values.values[1].v2 = v_f32_sel_less_u32_b(
        fp32_lanes, 32, high_values.values[1].v2, rope_odd_high);
    return high_values;
}

#endif

typedef struct {
    float64 next_max;
    float64 previous_scale;
    float64 weight;
} dsv4_paged_softmax_step;

inline dsv4_paged_softmax_step dsv4_paged_make_softmax_step(
    float64 running_max,
    float64 score)
{
    // Exactly one side of the online-softmax update has exponent zero. Compute
    // the non-trivial decay once and select the identity value for the other.
    const float64 decay = v_exp_cephes_f32(
        -v_f32_abs_b(score - running_max));
    dsv4_paged_softmax_step step;
    step.next_max = v_f32_max_b(running_max, score);
    step.previous_scale = v_f32_sel_grt_f32_b(
        score, running_max, decay, 1.0f);
    step.weight = v_f32_sel_grt_f32_b(
        score, running_max, 1.0f, decay);
    return step;
}

#define DSV4_PAGED_ONLINE_UPDATE(TOKEN)                                  \
    do {                                                                 \
        const float64 score = (TOKEN).score * scale_value;               \
        const dsv4_paged_softmax_step step =                              \
            dsv4_paged_make_softmax_step(running_max, score);            \
        running_sum = running_sum * step.previous_scale + step.weight;   \
        _Pragma("unroll (4)")                                            \
        for (int update_chunk = 0; update_chunk < 4; ++update_chunk) {   \
            output_accum[update_chunk].v1 = v_f32_mac_b(                 \
                (TOKEN).values[update_chunk].v1,                         \
                step.weight,                                             \
                output_accum[update_chunk].v1 * step.previous_scale);    \
            output_accum[update_chunk].v2 = v_f32_mac_b(                 \
                (TOKEN).values[update_chunk].v2,                         \
                step.weight,                                             \
                output_accum[update_chunk].v2 * step.previous_scale);    \
        }                                                                \
        running_max = step.next_max;                                     \
    } while (0)

#if DSV4_PAGED_SPLITKV_PARTIAL

// FlashMLA-style split-KV decode. Each program owns two adjacent query heads
// and one contiguous slice of the combined compressed/SWA token stream. It
// emits the unnormalized FP32 online-softmax state (m, l, o); a second kernel
// combines the splits and applies the attention sink.
void main(
    tensor q,
    tensor compressed_storage_u8,
    tensor compressed_geometry,
    tensor topk_shape_buffer,
    tensor split_shape_buffer,
    tensor token_to_req_indices,
    tensor block_table,
    tensor is_valid_token,
    tensor seq_lens,
    tensor swa_storage_u8,
    tensor swa_geometry,
    tensor swa_indices,
    tensor swa_lens,
    tensor partial_output,
    tensor partial_stats)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
    int5 geometry_coords = {0, 0, 0, 0, 0};
    const int compressed_base =
        s_i32_ld_g(gen_addr(geometry_coords, compressed_geometry));
    geometry_coords[0] = 1;
    const int compressed_stride =
        s_i32_ld_g(gen_addr(geometry_coords, compressed_geometry));
    geometry_coords[0] = 2;
    const int compressed_block_size =
        s_i32_ld_g(gen_addr(geometry_coords, compressed_geometry));
    geometry_coords[0] = 3;
    const int compressed_blocks =
        s_i32_ld_g(gen_addr(geometry_coords, compressed_geometry));
    const int compressed_slots = compressed_block_size * compressed_blocks;

    geometry_coords[0] = 0;
    const int swa_base =
        s_i32_ld_g(gen_addr(geometry_coords, swa_geometry));
    geometry_coords[0] = 1;
    const int swa_stride =
        s_i32_ld_g(gen_addr(geometry_coords, swa_geometry));
    geometry_coords[0] = 2;
    const int swa_block_size =
        s_i32_ld_g(gen_addr(geometry_coords, swa_geometry));
    geometry_coords[0] = 3;
    const int swa_blocks =
        s_i32_ld_g(gen_addr(geometry_coords, swa_geometry));
    const int swa_slots = swa_block_size * swa_blocks;

    const float scale_value = 0.04419417382415922f;
    const int topk_width = get_dim_size(topk_shape_buffer, 0);
    const int swa_width = get_dim_size(swa_indices, 0);
    const int split_count = get_dim_size(split_shape_buffer, 0);
    const int head_count = get_dim_size(q, 1);

    for (int batch = index_start[2]; batch < index_end[2]; ++batch) {
        int5 batch_coords = {batch, 0, 0, 0, 0};
#if DSV4_PAGED_SWA_ONLY
        // The SWA-only specialization keeps the local/pair ABI to reuse the
        // two-head MQA program, but the compressed stream is absent.
        const int request_index = 0;
        int topk_count = 0;
#else
        const int request_index = s_i32_ld_g(
            gen_addr(batch_coords, token_to_req_indices));
        const int valid_token = s_i32_ld_g(
            gen_addr(batch_coords, is_valid_token));
        int5 seq_coords = {request_index, 0, 0, 0, 0};
        const int sequence_length = s_i32_ld_g(
            gen_addr(seq_coords, seq_lens));
        int topk_count = sequence_length / 4;
        topk_count = topk_count < topk_width ? topk_count : topk_width;
        topk_count = valid_token != 0 ? topk_count : 0;
#endif
        int swa_count = s_i32_ld_g(gen_addr(batch_coords, swa_lens));
        swa_count = swa_count < swa_width ? swa_count : swa_width;
        const int token_count = topk_count + swa_count;

        for (int head_program = index_start[1];
             head_program < index_end[1];
             ++head_program) {
            const int first_head = head_program * 2;
            const bool active[2] = {
                first_head < head_count,
                first_head + 1 < head_count};
            bfloat128 query_vectors[2][4];
            minifloat256 query_low[2];
            minifloat256 query_high[2];
            #pragma unroll (2)
            for (int head_offset = 0; head_offset < 2; ++head_offset) {
                const int head = first_head + head_offset;
                #pragma unroll (4)
                for (int chunk = 0; chunk < 4; ++chunk) {
                    if (active[head_offset]) {
                        int5 q_coords = {
                            chunk * 128, head, batch, 0, 0};
                        query_vectors[head_offset][chunk] =
                            v_bf16_ld_tnsr_b(q_coords, q);
                    } else {
                        query_vectors[head_offset][chunk] = 0.0f;
                    }
                }
                bfloat256 query_low_bf16;
                query_low_bf16.v1 = query_vectors[head_offset][0];
                query_low_bf16.v2 = query_vectors[head_offset][1];
                bfloat256 query_high_bf16;
                query_high_bf16.v1 = query_vectors[head_offset][2];
                query_high_bf16.v2 = query_vectors[head_offset][3];
                query_low[head_offset] = v_convert_bf16_to_f8_all_b(
                    query_low_bf16, SW_RHNE | SW_FP8_BIAS7);
                query_high[head_offset] = v_convert_bf16_to_f8_all_b(
                    query_high_bf16, SW_RHNE | SW_FP8_BIAS7);
            }

            for (int split = index_start[0]; split < index_end[0]; ++split) {
                const int token_begin = token_count * split / split_count;
                const int token_end = token_count * (split + 1) / split_count;
                float128 output_accum[2][4] = {0};
                float64 running_max[2] = {
                    -3.402823466e+38f, -3.402823466e+38f};
                float64 running_sum[2] = {0.0f, 0.0f};

#if DSV4_PAGED_SPLITKV_TILED
                const uint64 tile_lanes =
                    V_LANE_ID_32 & (DSV4_PAGED_SPLITKV_TILE_SIZE - 1);
                for (int tile_begin = token_begin;
                     tile_begin < token_end;
                     tile_begin += DSV4_PAGED_SPLITKV_TILE_SIZE) {
                    int source_slot_tile[DSV4_PAGED_SPLITKV_TILE_SIZE];
                    int source_is_swa_tile[DSV4_PAGED_SPLITKV_TILE_SIZE];
#if DSV4_PAGED_SPLITKV_CACHE_BF16_VALUES
                    dsv4_paged_bf16_values
                        cached_values[DSV4_PAGED_SPLITKV_TILE_SIZE];
#endif
                    float64 score_tiles[2] = {
                        -3.402823466e+38f, -3.402823466e+38f};
                    int has_valid = 0;

                    for (int tile_offset = 0;
                         tile_offset < DSV4_PAGED_SPLITKV_TILE_SIZE;
                         ++tile_offset) {
                        const int stream_position = tile_begin + tile_offset;
                        int source_slot = -1;
                        int source_is_swa = 0;
                        if (stream_position < token_end) {
                            if (stream_position < topk_count) {
                                source_slot = stream_position;
                                const int logical_block =
                                    source_slot / compressed_block_size;
                                const int block_offset = source_slot
                                    - logical_block * compressed_block_size;
                                int5 block_coords = {
                                    logical_block,
                                    request_index,
                                    0,
                                    0,
                                    0};
                                const int physical_block = s_i32_ld_g(
                                    gen_addr(block_coords, block_table));
                                source_slot =
                                    physical_block * compressed_block_size
                                    + block_offset;
                                if (source_slot < 0
                                    || source_slot >= compressed_slots) {
                                    source_slot = -1;
                                }
                            } else {
                                source_is_swa = 1;
                                const int swa_position =
                                    stream_position - topk_count;
                                int5 index_coords = {
                                    swa_position, batch, 0, 0, 0};
                                source_slot = s_i32_ld_g(
                                    gen_addr(index_coords, swa_indices));
                                if (source_slot < 0
                                    || source_slot >= swa_slots) {
                                    source_slot = -1;
                                }
                            }
                        }
                        source_slot_tile[tile_offset] = source_slot;
                        source_is_swa_tile[tile_offset] = source_is_swa;
                        if (source_slot < 0) {
                            continue;
                        }
                        has_valid = 1;

                        dsv4_paged_pair_scores token_scores;
#if DSV4_PAGED_SPLITKV_CACHE_BF16_VALUES
                        dsv4_paged_pair_token token;
                        if (source_is_swa) {
                            token = dsv4_paged_load_token_pair(
                                swa_storage_u8,
                                swa_base,
                                swa_stride,
                                swa_block_size,
                                source_slot,
                                query_low[0],
                                query_high[0],
                                query_vectors[0][3],
                                query_low[1],
                                query_high[1],
                                query_vectors[1][3]);
                        } else {
                            token = dsv4_paged_load_token_pair(
                                compressed_storage_u8,
                                compressed_base,
                                compressed_stride,
                                compressed_block_size,
                                source_slot,
                                query_low[0],
                                query_high[0],
                                query_vectors[0][3],
                                query_low[1],
                                query_high[1],
                                query_vectors[1][3]);
                        }
                        token_scores.scores[0] = token.scores[0];
                        token_scores.scores[1] = token.scores[1];
                        #pragma unroll (DSV4_PAGED_SPLITKV_CACHE_BF16_CHUNKS)
                        for (int chunk = 0;
                             chunk < DSV4_PAGED_SPLITKV_CACHE_BF16_CHUNKS;
                             ++chunk) {
                            cached_values[tile_offset].values[chunk] =
                                v_convert_f32_to_bf16_all_b(
                                    token.values[chunk], SW_RHNE);
                        }
#else
                        if (source_is_swa) {
                            token_scores = dsv4_paged_load_pair_scores(
                                swa_storage_u8,
                                swa_base,
                                swa_stride,
                                swa_block_size,
                                source_slot,
                                query_low[0],
                                query_high[0],
                                query_vectors[0][3],
                                query_low[1],
                                query_high[1],
                                query_vectors[1][3]);
                        } else {
                            token_scores = dsv4_paged_load_pair_scores(
                                compressed_storage_u8,
                                compressed_base,
                                compressed_stride,
                                compressed_block_size,
                                source_slot,
                                query_low[0],
                                query_high[0],
                                query_vectors[0][3],
                                query_low[1],
                                query_high[1],
                                query_vectors[1][3]);
                        }
#endif
                        #pragma unroll (2)
                        for (int head_offset = 0;
                             head_offset < 2;
                             ++head_offset) {
                            float64 score = v_f32_shuffle_b(
                                token_scores.scores[head_offset],
                                0x80,
                                0,
                                token_scores.scores[head_offset]);
                            score *= scale_value;
                            score_tiles[head_offset] =
                                v_f32_sel_eq_u32_b(
                                    tile_lanes,
                                    tile_offset,
                                    score,
                                    score_tiles[head_offset]);
                        }
                    }
                    if (!has_valid) {
                        continue;
                    }

                    float64 tile_weights[2] = {0.0f, 0.0f};
                    for (int head_offset = 0;
                         head_offset < 2;
                         ++head_offset) {
                        if (!active[head_offset]) {
                            continue;
                        }
                        float64 tile_max =
                            v_f32_reduce_max(score_tiles[head_offset]);
                        tile_max = v_f32_shuffle_b(
                            tile_max, 0x80, 0, tile_max);
                        const float64 next_max = v_f32_max_b(
                            running_max[head_offset], tile_max);
                        const float64 previous_scale = v_exp_cephes_f32(
                            running_max[head_offset] - next_max);
                        float64 weights = v_exp_cephes_f32(
                            score_tiles[head_offset] - next_max);
                        float64 tile_sum = v_f32_reduce_add(weights)
                            * (DSV4_PAGED_SPLITKV_TILE_SIZE / 64.0f);
                        tile_sum = v_f32_shuffle_b(
                            tile_sum, 0x80, 0, tile_sum);
                        running_sum[head_offset] =
                            running_sum[head_offset] * previous_scale
                            + tile_sum;
                        #pragma unroll (4)
                        for (int chunk = 0; chunk < 4; ++chunk) {
                            output_accum[head_offset][chunk].v1 *=
                                previous_scale;
                            output_accum[head_offset][chunk].v2 *=
                                previous_scale;
                        }
                        running_max[head_offset] = next_max;
                        tile_weights[head_offset] = weights;
                    }

                    for (int tile_offset = 0;
                         tile_offset < DSV4_PAGED_SPLITKV_TILE_SIZE;
                         ++tile_offset) {
                        const int source_slot = source_slot_tile[tile_offset];
                        if (source_slot < 0) {
                            continue;
                        }
                        dsv4_paged_values token_values;
#if DSV4_PAGED_SPLITKV_CACHE_BF16_VALUES
                        #pragma unroll (DSV4_PAGED_SPLITKV_CACHE_BF16_CHUNKS)
                        for (int chunk = 0;
                             chunk < DSV4_PAGED_SPLITKV_CACHE_BF16_CHUNKS;
                             ++chunk) {
                            token_values.values[chunk] =
                                v_convert_bf16_to_f32_all_b(
                                    cached_values[tile_offset].values[chunk]);
                        }
#if DSV4_PAGED_SPLITKV_CACHE_BF16_CHUNKS < 4
                        dsv4_paged_high_values high_values;
                        if (source_is_swa_tile[tile_offset]) {
                            high_values = dsv4_paged_load_high_values(
                                swa_storage_u8,
                                swa_base,
                                swa_stride,
                                swa_block_size,
                                source_slot);
                        } else {
                            high_values = dsv4_paged_load_high_values(
                                compressed_storage_u8,
                                compressed_base,
                                compressed_stride,
                                compressed_block_size,
                                source_slot);
                        }
#if DSV4_PAGED_SPLITKV_CACHE_BF16_CHUNKS < 3
                        token_values.values[2] = high_values.values[0];
#endif
                        token_values.values[3] = high_values.values[1];
#endif
#else
                        if (source_is_swa_tile[tile_offset]) {
                            token_values = dsv4_paged_load_values(
                                swa_storage_u8,
                                swa_base,
                                swa_stride,
                                swa_block_size,
                                source_slot);
                        } else {
                            token_values = dsv4_paged_load_values(
                                compressed_storage_u8,
                                compressed_base,
                                compressed_stride,
                                compressed_block_size,
                                source_slot);
                        }
#endif
                        #pragma unroll (2)
                        for (int head_offset = 0;
                             head_offset < 2;
                             ++head_offset) {
                            if (!active[head_offset]) {
                                continue;
                            }
                            const float64 weight = v_f32_shuffle_b(
                                tile_weights[head_offset],
                                0x80 + tile_offset,
                                0,
                                tile_weights[head_offset]);
                            #pragma unroll (4)
                            for (int chunk = 0; chunk < 4; ++chunk) {
                                output_accum[head_offset][chunk].v1 =
                                    v_f32_mac_b(
                                        token_values.values[chunk].v1,
                                        weight,
                                        output_accum[head_offset][chunk].v1);
                                output_accum[head_offset][chunk].v2 =
                                    v_f32_mac_b(
                                        token_values.values[chunk].v2,
                                        weight,
                                        output_accum[head_offset][chunk].v2);
                            }
                        }
                    }
                }
#else
                for (int stream_position = token_begin;
                     stream_position < token_end;
                     ++stream_position) {
                    int source_slot;
                    tensor source_storage;
                    int source_base;
                    int source_stride;
                    int source_block_size;
                    int source_slots;
                    if (stream_position < topk_count) {
                        source_slot = stream_position;
                        const int logical_block =
                            source_slot / compressed_block_size;
                        const int block_offset = source_slot
                            - logical_block * compressed_block_size;
                        int5 block_coords = {
                            logical_block, request_index, 0, 0, 0};
                        const int physical_block = s_i32_ld_g(
                            gen_addr(block_coords, block_table));
                        source_slot = physical_block * compressed_block_size
                            + block_offset;
                        source_storage = compressed_storage_u8;
                        source_base = compressed_base;
                        source_stride = compressed_stride;
                        source_block_size = compressed_block_size;
                        source_slots = compressed_slots;
                    } else {
                        const int swa_position =
                            stream_position - topk_count;
                        int5 index_coords = {
                            swa_position, batch, 0, 0, 0};
                        source_slot = s_i32_ld_g(
                            gen_addr(index_coords, swa_indices));
                        source_storage = swa_storage_u8;
                        source_base = swa_base;
                        source_stride = swa_stride;
                        source_block_size = swa_block_size;
                        source_slots = swa_slots;
                    }
                    if (source_slot < 0 || source_slot >= source_slots) {
                        continue;
                    }

                    const dsv4_paged_pair_token token =
                        dsv4_paged_load_token_pair(
                            source_storage,
                            source_base,
                            source_stride,
                            source_block_size,
                            source_slot,
                            query_low[0],
                            query_high[0],
                            query_vectors[0][3],
                            query_low[1],
                            query_high[1],
                            query_vectors[1][3]);
                    #pragma unroll (2)
                    for (int head_offset = 0;
                         head_offset < 2;
                         ++head_offset) {
                        if (!active[head_offset]) {
                            continue;
                        }
                        const float64 score =
                            token.scores[head_offset] * scale_value;
                        const dsv4_paged_softmax_step step =
                            dsv4_paged_make_softmax_step(
                                running_max[head_offset], score);
                        running_sum[head_offset] =
                            running_sum[head_offset] * step.previous_scale
                            + step.weight;
                        #pragma unroll (4)
                        for (int chunk = 0; chunk < 4; ++chunk) {
                            output_accum[head_offset][chunk].v1 = v_f32_mac_b(
                                token.values[chunk].v1,
                                step.weight,
                                output_accum[head_offset][chunk].v1
                                    * step.previous_scale);
                            output_accum[head_offset][chunk].v2 = v_f32_mac_b(
                                token.values[chunk].v2,
                                step.weight,
                                output_accum[head_offset][chunk].v2
                                    * step.previous_scale);
                        }
                        running_max[head_offset] = step.next_max;
                    }
                }
#endif

                #pragma unroll (2)
                for (int head_offset = 0;
                     head_offset < 2;
                     ++head_offset) {
                    const int head = first_head + head_offset;
                    if (!active[head_offset]) {
                        continue;
                    }
                    #pragma unroll (4)
                    for (int chunk = 0; chunk < 4; ++chunk) {
                        int5 output_coords = {
                            chunk * 128, head, split, batch, 0};
                        v_f32_st_tnsr(
                            output_coords,
                            partial_output,
                            output_accum[head_offset][chunk].v1);
                        output_coords[0] += 64;
                        v_f32_st_tnsr(
                            output_coords,
                            partial_output,
                            output_accum[head_offset][chunk].v2);
                    }
                    int5 stats_coords = {0, head, split, batch, 0};
                    v_f32_st_tnsr_partial(
                        stats_coords,
                        partial_stats,
                        running_max[head_offset],
                        0,
                        0);
                    stats_coords[0] = 1;
                    v_f32_st_tnsr_partial(
                        stats_coords,
                        partial_stats,
                        running_sum[head_offset],
                        0,
                        0);
                }
            }
        }
    }
}

#elif DSV4_PAGED_HEADS_PER_PROGRAM == 1
void main(
    tensor q,
#if DSV4_PAGED_FUSED_QNORM
    tensor positions,
    tensor cos_sin_cache,
#endif
    tensor compressed_storage_u8,
    tensor compressed_geometry,
    tensor topk_indices,
#if DSV4_PAGED_LOCAL_TOPK
#if !DSV4_PAGED_FUSED_QNORM
    tensor token_to_req_indices,
#endif
    tensor block_table,
    tensor is_valid_token,
#if DSV4_PAGED_SEQUENTIAL_TOPK
    tensor seq_lens,
#endif
#else
    tensor topk_lens,
#endif
    tensor swa_storage_u8,
    tensor swa_geometry,
    tensor swa_indices,
    tensor swa_lens,
    tensor attn_sink,
    tensor output,
    tensor score_debug)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
    int5 geometry_coords = {0, 0, 0, 0, 0};
    const int compressed_base =
        s_i32_ld_g(gen_addr(geometry_coords, compressed_geometry));
    geometry_coords[0] = 1;
    const int compressed_stride =
        s_i32_ld_g(gen_addr(geometry_coords, compressed_geometry));
    geometry_coords[0] = 2;
    const int compressed_block_size =
        s_i32_ld_g(gen_addr(geometry_coords, compressed_geometry));
    geometry_coords[0] = 3;
    const int compressed_blocks =
        s_i32_ld_g(gen_addr(geometry_coords, compressed_geometry));
    const int compressed_slots = compressed_block_size * compressed_blocks;

    geometry_coords[0] = 0;
    const int swa_base =
        s_i32_ld_g(gen_addr(geometry_coords, swa_geometry));
    geometry_coords[0] = 1;
    const int swa_stride =
        s_i32_ld_g(gen_addr(geometry_coords, swa_geometry));
    geometry_coords[0] = 2;
    const int swa_block_size =
        s_i32_ld_g(gen_addr(geometry_coords, swa_geometry));
    geometry_coords[0] = 3;
    const int swa_blocks =
        s_i32_ld_g(gen_addr(geometry_coords, swa_geometry));
    const int swa_slots = swa_block_size * swa_blocks;

    const float scale_value = 0.04419417382415922f;
    const int topk_width = get_dim_size(topk_indices, 0);
    const int swa_width = get_dim_size(swa_indices, 0);

    for (int batch = index_start[2]; batch < index_end[2]; ++batch) {
        int5 batch_coords = {batch, 0, 0, 0, 0};
#if DSV4_PAGED_LOCAL_TOPK
#if DSV4_PAGED_FUSED_QNORM
        const int request_index = batch;
#else
        const int request_index = s_i32_ld_g(
            gen_addr(batch_coords, token_to_req_indices));
#endif
        const int valid_token = s_i32_ld_g(
            gen_addr(batch_coords, is_valid_token));
#if DSV4_PAGED_SEQUENTIAL_TOPK
        int5 seq_coords = {request_index, 0, 0, 0, 0};
        const int sequence_length = s_i32_ld_g(
            gen_addr(seq_coords, seq_lens));
        int topk_count = sequence_length / 4;
        topk_count = topk_count < topk_width ? topk_count : topk_width;
        topk_count = valid_token != 0 ? topk_count : 0;
#else
        int topk_count = valid_token != 0 ? topk_width : 0;
#endif
#else
#if DSV4_PAGED_SWA_ONLY
        // This specialization mirrors FlashMLA's SWA-only layer plan. Keep
        // the generic ABI so the same graph plumbing can be reused, but make
        // the absent compressed-cache stream a compile-time constant.
        int topk_count = 0;
#else
        int topk_count = s_i32_ld_g(gen_addr(batch_coords, topk_lens));
#endif
#endif
        int swa_count = s_i32_ld_g(gen_addr(batch_coords, swa_lens));
        topk_count = topk_count < topk_width ? topk_count : topk_width;
        swa_count = swa_count < swa_width ? swa_count : swa_width;

        for (int head = index_start[1]; head < index_end[1]; ++head) {
            int5 sink_coords = {head, 0, 0, 0, 0};
            const float sink_value =
                s_f32_ld_g(gen_addr(sink_coords, attn_sink));
            if (sink_value < -3.0e38f) {
                const bfloat128 zero = 0.0f;
                #pragma unroll (4)
                for (int chunk = 0; chunk < 4; ++chunk) {
                    int5 output_coords = {
                        chunk * 128, head, batch, 0, 0};
                    v_bf16_st_tnsr(output_coords, output, zero);
                }
                int5 debug_coords = {head, batch, 0, 0, 0};
                v_f32_st_tnsr_partial(
                    debug_coords, score_debug, 0.0f, 0, 0);
                continue;
            }

            bfloat128 query_vectors[4];
#if DSV4_PAGED_FUSED_QNORM
            int5 position_coords = {batch, 0, 0, 0, 0};
            const int query_position =
                s_i32_ld_g(gen_addr(position_coords, positions));
            float64 query_chunks[8];
            float64 query_sumsq = 0.0f;
            #pragma unroll (4)
            for (int chunk = 0; chunk < 4; ++chunk) {
                int5 q_coords = {chunk * 128, head, batch, 0, 0};
                const bfloat128 q_bf16 =
                    v_bf16_ld_tnsr_b(q_coords, q);
                const float128 q_f32 =
                    convert_bfloat128_to_float128(q_bf16, SW_LINEAR);
                query_chunks[chunk * 2] = q_f32.v1;
                query_chunks[chunk * 2 + 1] = q_f32.v2;
                float64 chunk_sumsq = v_f32_reduce_add(
                    q_f32.v1 * q_f32.v1 + q_f32.v2 * q_f32.v2);
                chunk_sumsq = v_f32_shuffle_b(
                    chunk_sumsq, 0x80, 0, chunk_sumsq);
                query_sumsq += chunk_sumsq;
            }
            const float64 query_rrms =
                v_rsqrt_f32(query_sumsq / 512.0f + 1.0e-6f);
            #pragma unroll (4)
            for (int chunk = 0; chunk < 4; ++chunk) {
                float128 normalized = {0};
                normalized.v1 = query_chunks[chunk * 2] * query_rrms;
                normalized.v2 = query_chunks[chunk * 2 + 1] * query_rrms;
                if (chunk == 3) {
                    normalized.v2 = dsv4_qkv_apply_pairwise_rope_f32(
                        normalized.v2,
                        cos_sin_cache,
                        query_position);
                }
                query_vectors[chunk] =
                    convert_float128_to_bfloat128(
                        normalized, SW_RHNE | SW_LINEAR);
            }
#else
            #pragma unroll (4)
            for (int chunk = 0; chunk < 4; ++chunk) {
                int5 q_coords = {chunk * 128, head, batch, 0, 0};
                query_vectors[chunk] = v_bf16_ld_tnsr_b(q_coords, q);
            }
#endif
            bfloat256 query_low_bf16;
            query_low_bf16.v1 = query_vectors[0];
            query_low_bf16.v2 = query_vectors[1];
            bfloat256 query_high_bf16;
            query_high_bf16.v1 = query_vectors[2];
            query_high_bf16.v2 = query_vectors[3];
            const minifloat256 query_low =
                v_convert_bf16_to_f8_all_b(
                    query_low_bf16, SW_RHNE | SW_FP8_BIAS7);
            const minifloat256 query_high =
                v_convert_bf16_to_f8_all_b(
                    query_high_bf16, SW_RHNE | SW_FP8_BIAS7);

            #if DSV4_PAGED_DEBUG_QUERY_CONVERT
            const uchar256 query_low_raw = as_uchar256(query_low);
            const uchar256 query_high_raw = as_uchar256(query_high);
            const bfloat128 query_low_packed =
                *((bfloat128*)&query_low_raw);
            const bfloat128 query_high_packed =
                *((bfloat128*)&query_high_raw);
            int5 output_coords = {0, head, batch, 0, 0};
            v_bf16_st_tnsr(output_coords, output, query_low_packed);
            output_coords[0] = 128;
            v_bf16_st_tnsr(output_coords, output, query_high_packed);
            const bfloat128 zero = 0.0f;
            output_coords[0] = 256;
            v_bf16_st_tnsr(output_coords, output, zero);
            output_coords[0] = 384;
            v_bf16_st_tnsr(output_coords, output, zero);
            continue;
            #endif


            float128 output_accum[4] = {0};
            float64 running_max = -3.402823466e+38f;
            float64 running_sum = 0.0f;
            #if !DSV4_PAGED_DEBUG_MAC_PARTS
            float64 last_score = 0.0f;
            #endif

            for (int position = 0; position < topk_count; ++position) {
                int source_slot;
#if DSV4_PAGED_SEQUENTIAL_TOPK
                source_slot = position;
#else
                int5 index_coords = {position, batch, 0, 0, 0};
                source_slot = s_i32_ld_g(
                    gen_addr(index_coords, topk_indices));
#endif
#if DSV4_PAGED_LOCAL_TOPK
#if !DSV4_PAGED_SEQUENTIAL_TOPK
                if (source_slot < 0) {
                    break;
                }
#endif
                const int logical_block =
                    source_slot / compressed_block_size;
                const int block_offset =
                    source_slot - logical_block * compressed_block_size;
                int5 block_coords = {
                    logical_block, request_index, 0, 0, 0};
                const int physical_block = s_i32_ld_g(
                    gen_addr(block_coords, block_table));
                source_slot =
                    physical_block * compressed_block_size + block_offset;
#endif
                if (source_slot < 0 || source_slot >= compressed_slots) {
                    continue;
                }
                const dsv4_paged_token token = dsv4_paged_load_token(
                    compressed_storage_u8,
                    compressed_base,
                    compressed_stride,
                    compressed_block_size,
                    source_slot,
                    score_debug,
                    head,
                    batch,
                    query_low,
                    query_high,
                    query_vectors[0],
                    query_vectors[1],
                    query_vectors[2],
                    query_vectors[3]);
                #if !DSV4_PAGED_DEBUG_MAC_PARTS
                last_score = token.score;
                #endif
                DSV4_PAGED_ONLINE_UPDATE(token);
            }

            for (int position = 0; position < swa_count; ++position) {
                int5 index_coords = {position, batch, 0, 0, 0};
                const int source_slot =
                    s_i32_ld_g(gen_addr(index_coords, swa_indices));
                if (source_slot < 0 || source_slot >= swa_slots) {
                    continue;
                }
                const dsv4_paged_token token = dsv4_paged_load_token(
                    swa_storage_u8,
                    swa_base,
                    swa_stride,
                    swa_block_size,
                    source_slot,
                    score_debug,
                    head,
                    batch,
                    query_low,
                    query_high,
                    query_vectors[0],
                    query_vectors[1],
                    query_vectors[2],
                    query_vectors[3]);
                #if !DSV4_PAGED_DEBUG_MAC_PARTS
                last_score = token.score;
                #endif
                DSV4_PAGED_ONLINE_UPDATE(token);
            }

            const float64 sink_score = sink_value;
            const float64 final_max = v_f32_max_b(running_max, sink_score);
            const float64 data_scale =
                v_exp_cephes_f32(running_max - final_max);
            const float64 sink_weight =
                v_exp_cephes_f32(sink_score - final_max);
            running_sum = running_sum * data_scale + sink_weight;
            const float64 inverse_sum = v_reciprocal_f32(running_sum);

            #pragma unroll (4)
            for (int chunk = 0; chunk < 4; ++chunk) {
                output_accum[chunk].v1 *= data_scale * inverse_sum;
                output_accum[chunk].v2 *= data_scale * inverse_sum;
                const bfloat128 result = v_convert_f32_to_bf16_all_b(
                    output_accum[chunk], SW_RHNE);
                int5 output_coords = {
                    chunk * 128, head, batch, 0, 0};
                v_bf16_st_tnsr(output_coords, output, result);
            }
            #if !DSV4_PAGED_DEBUG_MAC_PARTS
            int5 debug_coords = {head, batch, 0, 0, 0};
            v_f32_st_tnsr_partial(
                debug_coords, score_debug, last_score, 0, 0);
            #endif
        }
    }
}
#else

// H200-style MQA head tiling prototype. A program owns two adjacent query
// heads so their identical KV traversal stays on the same TPC and can reuse
// the immediately preceding cache lines. Per-head arithmetic order is kept
// identical to the single-head kernel for exact A/B validation.
void main(
    tensor q,
    tensor compressed_storage_u8,
    tensor compressed_geometry,
    tensor topk_indices,
    tensor token_to_req_indices,
    tensor block_table,
    tensor is_valid_token,
    tensor seq_lens,
    tensor swa_storage_u8,
    tensor swa_geometry,
    tensor swa_indices,
    tensor swa_lens,
    tensor attn_sink,
    tensor output,
    tensor score_debug)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
    int5 geometry_coords = {0, 0, 0, 0, 0};
    const int compressed_base =
        s_i32_ld_g(gen_addr(geometry_coords, compressed_geometry));
    geometry_coords[0] = 1;
    const int compressed_stride =
        s_i32_ld_g(gen_addr(geometry_coords, compressed_geometry));
    geometry_coords[0] = 2;
    const int compressed_block_size =
        s_i32_ld_g(gen_addr(geometry_coords, compressed_geometry));
    geometry_coords[0] = 3;
    const int compressed_blocks =
        s_i32_ld_g(gen_addr(geometry_coords, compressed_geometry));
    const int compressed_slots = compressed_block_size * compressed_blocks;

    geometry_coords[0] = 0;
    const int swa_base =
        s_i32_ld_g(gen_addr(geometry_coords, swa_geometry));
    geometry_coords[0] = 1;
    const int swa_stride =
        s_i32_ld_g(gen_addr(geometry_coords, swa_geometry));
    geometry_coords[0] = 2;
    const int swa_block_size =
        s_i32_ld_g(gen_addr(geometry_coords, swa_geometry));
    geometry_coords[0] = 3;
    const int swa_blocks =
        s_i32_ld_g(gen_addr(geometry_coords, swa_geometry));
    const int swa_slots = swa_block_size * swa_blocks;

    const float scale_value = 0.04419417382415922f;
    const int topk_width = get_dim_size(topk_indices, 0);
    const int swa_width = get_dim_size(swa_indices, 0);
    const int head_count = get_dim_size(q, 1);

    for (int batch = index_start[2]; batch < index_end[2]; ++batch) {
        int5 batch_coords = {batch, 0, 0, 0, 0};
        const int request_index = s_i32_ld_g(
            gen_addr(batch_coords, token_to_req_indices));
        const int valid_token = s_i32_ld_g(
            gen_addr(batch_coords, is_valid_token));
        int5 seq_coords = {request_index, 0, 0, 0, 0};
        const int sequence_length = s_i32_ld_g(
            gen_addr(seq_coords, seq_lens));
        int topk_count = sequence_length / 4;
        topk_count = topk_count < topk_width ? topk_count : topk_width;
        topk_count = valid_token != 0 ? topk_count : 0;
        int swa_count = s_i32_ld_g(gen_addr(batch_coords, swa_lens));
        swa_count = swa_count < swa_width ? swa_count : swa_width;

        for (int head_group = index_start[1];
             head_group < index_end[1];
             ++head_group) {
            const int first_head =
                head_group * DSV4_PAGED_HEADS_PER_PROGRAM;
            bfloat128 query_vectors[DSV4_PAGED_HEADS_PER_PROGRAM][4];
            minifloat256 query_low[DSV4_PAGED_HEADS_PER_PROGRAM];
            minifloat256 query_high[DSV4_PAGED_HEADS_PER_PROGRAM];
            float128 output_accum[DSV4_PAGED_HEADS_PER_PROGRAM][4] = {0};
            float64 running_max[DSV4_PAGED_HEADS_PER_PROGRAM];
            float64 running_sum[DSV4_PAGED_HEADS_PER_PROGRAM];
            float sink_values[DSV4_PAGED_HEADS_PER_PROGRAM];
            float64 last_score[DSV4_PAGED_HEADS_PER_PROGRAM];
            bool active[DSV4_PAGED_HEADS_PER_PROGRAM];

            #pragma unroll (DSV4_PAGED_HEADS_PER_PROGRAM)
            for (int head_offset = 0;
                 head_offset < DSV4_PAGED_HEADS_PER_PROGRAM;
                 ++head_offset) {
                const int head = first_head + head_offset;
                active[head_offset] = head < head_count;
                int5 sink_coords = {head, 0, 0, 0, 0};
                sink_values[head_offset] = active[head_offset]
                    ? s_f32_ld_g(gen_addr(sink_coords, attn_sink))
                    : -3.402823466e+38f;
                active[head_offset] = active[head_offset]
                    && sink_values[head_offset] >= -3.0e38f;
                running_max[head_offset] = -3.402823466e+38f;
                running_sum[head_offset] = 0.0f;
                last_score[head_offset] = 0.0f;

                if (!active[head_offset]) {
                    continue;
                }
                #pragma unroll (4)
                for (int chunk = 0; chunk < 4; ++chunk) {
                    int5 q_coords = {
                        chunk * 128, head, batch, 0, 0};
                    query_vectors[head_offset][chunk] =
                        v_bf16_ld_tnsr_b(q_coords, q);
                }
                bfloat256 query_low_bf16;
                query_low_bf16.v1 = query_vectors[head_offset][0];
                query_low_bf16.v2 = query_vectors[head_offset][1];
                bfloat256 query_high_bf16;
                query_high_bf16.v1 = query_vectors[head_offset][2];
                query_high_bf16.v2 = query_vectors[head_offset][3];
                query_low[head_offset] = v_convert_bf16_to_f8_all_b(
                    query_low_bf16, SW_RHNE | SW_FP8_BIAS7);
                query_high[head_offset] = v_convert_bf16_to_f8_all_b(
                    query_high_bf16, SW_RHNE | SW_FP8_BIAS7);
            }

            for (int position = 0; position < topk_count; ++position) {
                int source_slot = position;
                const int logical_block =
                    source_slot / compressed_block_size;
                const int block_offset =
                    source_slot - logical_block * compressed_block_size;
                int5 block_coords = {
                    logical_block, request_index, 0, 0, 0};
                const int physical_block = s_i32_ld_g(
                    gen_addr(block_coords, block_table));
                source_slot =
                    physical_block * compressed_block_size + block_offset;
                if (source_slot < 0 || source_slot >= compressed_slots) {
                    continue;
                }

                const dsv4_paged_pair_token token =
                    dsv4_paged_load_token_pair(
                        compressed_storage_u8,
                        compressed_base,
                        compressed_stride,
                        compressed_block_size,
                        source_slot,
                        query_low[0],
                        query_high[0],
                        query_vectors[0][3],
                        query_low[1],
                        query_high[1],
                        query_vectors[1][3]);
                #pragma unroll (DSV4_PAGED_HEADS_PER_PROGRAM)
                for (int head_offset = 0;
                     head_offset < DSV4_PAGED_HEADS_PER_PROGRAM;
                     ++head_offset) {
                    if (!active[head_offset]) {
                        continue;
                    }
                    last_score[head_offset] = token.scores[head_offset];
                    const float64 score =
                        token.scores[head_offset] * scale_value;
                    const float64 next_max = v_f32_max_b(
                        running_max[head_offset], score);
                    const float64 previous_scale = v_exp_cephes_f32(
                        running_max[head_offset] - next_max);
                    const float64 weight =
                        v_exp_cephes_f32(score - next_max);
                    running_sum[head_offset] =
                        running_sum[head_offset] * previous_scale + weight;
                    #pragma unroll (4)
                    for (int chunk = 0; chunk < 4; ++chunk) {
                        output_accum[head_offset][chunk].v1 = v_f32_mac_b(
                            token.values[chunk].v1,
                            weight,
                            output_accum[head_offset][chunk].v1
                                * previous_scale);
                        output_accum[head_offset][chunk].v2 = v_f32_mac_b(
                            token.values[chunk].v2,
                            weight,
                            output_accum[head_offset][chunk].v2
                                * previous_scale);
                    }
                    running_max[head_offset] = next_max;
                }
            }

            for (int position = 0; position < swa_count; ++position) {
                int5 index_coords = {position, batch, 0, 0, 0};
                const int source_slot = s_i32_ld_g(
                    gen_addr(index_coords, swa_indices));
                if (source_slot < 0 || source_slot >= swa_slots) {
                    continue;
                }

                const dsv4_paged_pair_token token =
                    dsv4_paged_load_token_pair(
                        swa_storage_u8,
                        swa_base,
                        swa_stride,
                        swa_block_size,
                        source_slot,
                        query_low[0],
                        query_high[0],
                        query_vectors[0][3],
                        query_low[1],
                        query_high[1],
                        query_vectors[1][3]);
                #pragma unroll (DSV4_PAGED_HEADS_PER_PROGRAM)
                for (int head_offset = 0;
                     head_offset < DSV4_PAGED_HEADS_PER_PROGRAM;
                     ++head_offset) {
                    if (!active[head_offset]) {
                        continue;
                    }
                    last_score[head_offset] = token.scores[head_offset];
                    const float64 score =
                        token.scores[head_offset] * scale_value;
                    const float64 next_max = v_f32_max_b(
                        running_max[head_offset], score);
                    const float64 previous_scale = v_exp_cephes_f32(
                        running_max[head_offset] - next_max);
                    const float64 weight =
                        v_exp_cephes_f32(score - next_max);
                    running_sum[head_offset] =
                        running_sum[head_offset] * previous_scale + weight;
                    #pragma unroll (4)
                    for (int chunk = 0; chunk < 4; ++chunk) {
                        output_accum[head_offset][chunk].v1 = v_f32_mac_b(
                            token.values[chunk].v1,
                            weight,
                            output_accum[head_offset][chunk].v1
                                * previous_scale);
                        output_accum[head_offset][chunk].v2 = v_f32_mac_b(
                            token.values[chunk].v2,
                            weight,
                            output_accum[head_offset][chunk].v2
                                * previous_scale);
                    }
                    running_max[head_offset] = next_max;
                }
            }

            #pragma unroll (DSV4_PAGED_HEADS_PER_PROGRAM)
            for (int head_offset = 0;
                 head_offset < DSV4_PAGED_HEADS_PER_PROGRAM;
                 ++head_offset) {
                const int head = first_head + head_offset;
                if (head >= head_count) {
                    continue;
                }
                if (!active[head_offset]) {
                    const bfloat128 zero = 0.0f;
                    #pragma unroll (4)
                    for (int chunk = 0; chunk < 4; ++chunk) {
                        int5 output_coords = {
                            chunk * 128, head, batch, 0, 0};
                        v_bf16_st_tnsr(output_coords, output, zero);
                    }
                    continue;
                }
                const float64 sink_score = sink_values[head_offset];
                const float64 final_max = v_f32_max_b(
                    running_max[head_offset], sink_score);
                const float64 data_scale = v_exp_cephes_f32(
                    running_max[head_offset] - final_max);
                const float64 sink_weight =
                    v_exp_cephes_f32(sink_score - final_max);
                running_sum[head_offset] =
                    running_sum[head_offset] * data_scale + sink_weight;
                const float64 inverse_sum =
                    v_reciprocal_f32(running_sum[head_offset]);
                #pragma unroll (4)
                for (int chunk = 0; chunk < 4; ++chunk) {
                    output_accum[head_offset][chunk].v1 *=
                        data_scale * inverse_sum;
                    output_accum[head_offset][chunk].v2 *=
                        data_scale * inverse_sum;
                    const bfloat128 result = v_convert_f32_to_bf16_all_b(
                        output_accum[head_offset][chunk], SW_RHNE);
                    int5 output_coords = {
                        chunk * 128, head, batch, 0, 0};
                    v_bf16_st_tnsr(output_coords, output, result);
                }
                int5 debug_coords = {head, batch, 0, 0, 0};
                v_f32_st_tnsr_partial(
                    debug_coords,
                    score_debug,
                    last_score[head_offset],
                    0,
                    0);
            }
        }
    }
}
#endif
