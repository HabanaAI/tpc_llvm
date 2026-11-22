/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

#ifndef DSV4_QNORM_HYBRID_Q
#define DSV4_QNORM_HYBRID_Q 0
#endif

#if DSV4_QNORM_HYBRID_Q
bfloat128 dsv4_qnorm_round_e4m3fn(bfloat128 values)
{
    bfloat256 wide = {0};
    wide.v1 = values;
    const minifloat256 rounded_fp8 =
        v_convert_bf16_to_f8_all_b(wide, SW_RHNE | SW_FP8_BIAS7);
    const bfloat256 rounded_bf16 =
        v_convert_f8_to_bf16_all_b(rounded_fp8, SW_FP8_BIAS7);
    return rounded_bf16.v1;
}
#endif

uchar256 dsv4_qkv_encode_e4m3fn_f32(float64 values)
{
    float256 direct_input = {0};
    direct_input.v1 = values;
    float256 half_input = {0};
    half_input.v1 = values * 0.5f;
    const uchar256 direct_raw = as_uchar256(
        v_convert_f32_to_f8_all_b(
            direct_input, SW_RHNE | SW_FP8_BIAS7 | SW_LINEAR));
    const uchar256 half_raw = as_uchar256(
        v_convert_f32_to_f8_all_b(
            half_input, SW_RHNE | SW_FP8_BIAS7 | SW_LINEAR));
    const uchar256 half_magnitude = v_u8_and_b(half_raw, 0x7f);
    const uchar256 shifted = v_u8_or_b(
        v_u8_and_b(half_raw, 0x80),
        v_u8_add_b(half_magnitude, 8));
    return v_u8_sel_geq_u8_b(
        half_magnitude, 8, shifted, direct_raw);
}

uchar256 dsv4_qkv_shuffle_directions(uint64 lane_codes)
{
    uint256 wide = {0};
    wide.v1 = lane_codes;
    wide.v2 = lane_codes;
    wide.v3 = lane_codes;
    wide.v4 = lane_codes;
    return v_convert_u32_to_u8_all_b(wide);
}

float64 dsv4_qkv_apply_pairwise_rope_f32(
    float64 values,
    tensor cos_sin_cache,
    int position)
{
    const uint64 lanes = V_LANE_ID_32;
    int5 cache_coords = {0, position, 0, 0, 0};
    const float64 cos_sin =
        v_f32_ld_tnsr_b(cache_coords, cos_sin_cache);

    // SHUFFLE selects inside an eight-lane group. Move the four cos and sin
    // source dual-groups first, then select the group and pair offset.
    const int64 cos_sin_bits = *((int64*)&cos_sin);
    const int64 cos_bits = v_i32_mov_dual_group_all_b(
        cos_sin_bits,
        0xFFFFFFFF,
        0,
        0,
        1,
        1,
        MkWrA(0b11, 0b11, 0b11, 0b11),
        cos_sin_bits);
    const int64 sin_bits = v_i32_mov_dual_group_all_b(
        cos_sin_bits,
        0xFFFFFFFF,
        2,
        2,
        3,
        3,
        MkWrA(0b11, 0b11, 0b11, 0b11),
        cos_sin_bits);
    const float64 cos_groups = *((float64*)&cos_bits);
    const float64 sin_groups = *((float64*)&sin_bits);

    const uint64 group_lanes = lanes & 7;
    const uint64 current_group = ((lanes >> 3) & 1) << 5;
    const uint64 pair_offsets = (lanes & 15) >> 1;
    const uint64 alternate_group = ((lanes >> 4) & 1) << 5;
    const uchar256 real_directions = dsv4_qkv_shuffle_directions(
        (group_lanes & 0xfffffffe) + current_group + 0x80);
    const uchar256 imag_directions = dsv4_qkv_shuffle_directions(
        (group_lanes | 1) + current_group + 0x80);
    const uchar256 cos_directions = dsv4_qkv_shuffle_directions(
        pair_offsets + alternate_group + 0x80);
    const uchar256 sin_directions = cos_directions;

    const float64 real = v_f32_shuffle_b(
        values, real_directions, 0, 0.0f);
    const float64 imag = v_f32_shuffle_b(
        values, imag_directions, 0, 0.0f);
    const float64 cos_values = v_f32_shuffle_b(
        cos_groups, cos_directions, 0, 0.0f);
    float64 sin_values = v_f32_shuffle_b(
        sin_groups, sin_directions, 0, 0.0f);
#ifdef DSV4_ROPE_INVERSE
    sin_values = -sin_values;
#endif
#ifdef DSV4_ROPE_SECOND_TERM_FMA
    // V4.1's compiled tensor reference rounds the first product, then fuses
    // the second multiply with the add/subtract. Keep V4's default unchanged.
    const float64 roped_real = v_f32_mac_b(imag, sin_values, real * cos_values, SW_NEG);
    const float64 roped_imag = v_f32_mac_b(imag, cos_values, real * sin_values, 0);
#else
    const float64 roped_real =
        real * cos_values - imag * sin_values;
    const float64 roped_imag =
        imag * cos_values + real * sin_values;
#endif
    return v_f32_sel_eq_u32_b(
        lanes & 1, 1, roped_imag, roped_real);
}

#ifndef DSV4_QNORM_HELPERS_ONLY
// The second index-space dimension mirrors the NVIDIA per-head grid. Each
// program owns one Q head; head zero also owns the single KV branch.
void main(
    tensor q,
    tensor kv,
    tensor cache_storage_u8,
    tensor cache_geometry,
    tensor slots,
    tensor positions,
    tensor cos_sin_cache,
    tensor mutation_token)
{
    const uchar256 broadcast_lane_zero = 0x80;
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
    const int num_heads = get_dim_size(q, 1);
    int5 geometry_coords = {0, 0, 0, 0, 0};
    const int cache_base_offset =
        s_i32_ld_g(gen_addr(geometry_coords, cache_geometry));
    geometry_coords[0] = 1;
    const int cache_block_stride =
        s_i32_ld_g(gen_addr(geometry_coords, cache_geometry));
    geometry_coords[0] = 2;
    const int cache_block_size =
        s_i32_ld_g(gen_addr(geometry_coords, cache_geometry));
    geometry_coords[0] = 3;
    const int cache_block_count =
        s_i32_ld_g(gen_addr(geometry_coords, cache_geometry));
    const int cache_slot_count = cache_block_size * cache_block_count;
    const float eps = 1.0e-6f;

    for (int token = index_start[0]; token < index_end[0]; ++token) {
        int5 position_coords = {token, 0, 0, 0, 0};
        const int position =
            s_i32_ld_g(gen_addr(position_coords, positions));
        const int slot = s_i32_ld_g(gen_addr(position_coords, slots));
        const bool valid_slot = slot >= 0 && slot < cache_slot_count;

        for (int branch = index_start[1];
             branch < index_end[1];
             ++branch) {
            if (branch < num_heads) {
                float64 chunks[8];
                float64 total_sumsq = 0.0f;
                #pragma unroll (4)
                for (int vector_index = 0;
                     vector_index < 4;
                     ++vector_index) {
                    int5 q_coords = {
                        vector_index * 128,
                        branch,
                        token,
                        0,
                        0};
                    const bfloat128 q_bf16 =
                        v_bf16_ld_tnsr_b(q_coords, q);
                    const float128 q_f32 =
                        convert_bfloat128_to_float128(
                            q_bf16, SW_LINEAR);
                    chunks[vector_index * 2] = q_f32.v1;
                    chunks[vector_index * 2 + 1] = q_f32.v2;
                    float64 sumsq = v_f32_reduce_add(
                        q_f32.v1 * q_f32.v1 +
                        q_f32.v2 * q_f32.v2);
                    sumsq = v_f32_shuffle_b(
                        sumsq,
                        broadcast_lane_zero,
                        0,
                        sumsq);
                    total_sumsq += sumsq;
                }

                const float64 rrms =
                    v_rsqrt_f32(total_sumsq / 512.0f + eps);
                #pragma unroll (4)
                for (int vector_index = 0;
                     vector_index < 4;
                     ++vector_index) {
                    float128 normalized = {0};
                    normalized.v1 =
                        chunks[vector_index * 2] * rrms;
                    normalized.v2 =
                        chunks[vector_index * 2 + 1] * rrms;
                    if (vector_index == 3) {
                        normalized.v2 =
                            dsv4_qkv_apply_pairwise_rope_f32(
                                normalized.v2,
                                cos_sin_cache,
                                position);
                    }
                    const bfloat128 result =
                        convert_float128_to_bfloat128(
                            normalized, SW_RHNE | SW_LINEAR);
                    int5 q_coords = {
                        vector_index * 128,
                        branch,
                        token,
                        0,
                        0};
#if DSV4_QNORM_HYBRID_Q
                    const bfloat128 rounded_nope =
                        dsv4_qnorm_round_e4m3fn(result);
                    if (vector_index < 3) {
                        v_bf16_st_tnsr(q_coords, q, rounded_nope);
                    } else {
                        // Dims 384..447 use FP8 QK; dims 448..511 are the
                        // BF16 RoPE tail and must retain their precision.
                        v_bf16_st_tnsr(q_coords, q, result);
                        v_bf16_st_tnsr_partial(
                            q_coords, q, rounded_nope, 63, 0);
                    }
#else
                    v_bf16_st_tnsr(q_coords, q, result);
#endif
                }
            }

            if (branch != 0) {
                continue;
            }

            s_u8_st_g(
                gen_addr(position_coords, mutation_token),
                (unsigned char)valid_slot);
            if (!valid_slot) {
                continue;
            }

            const int cache_block = slot / cache_block_size;
            const int cache_token = slot - cache_block * cache_block_size;
            const int value_base =
                cache_base_offset + cache_block * cache_block_stride
                + cache_token * 576;
            const int scale_base =
                cache_base_offset + cache_block * cache_block_stride
                + cache_block_size * 576 + cache_token * 8;

            float64 kv_chunks[8];
            #pragma unroll (4)
            for (int vector_index = 0;
                 vector_index < 4;
                 ++vector_index) {
                int5 kv_coords = {
                    vector_index * 128, token, 0, 0, 0};
                const bfloat128 kv_bf16 =
                    v_bf16_ld_tnsr_b(kv_coords, kv);
                const float128 kv_f32 =
                    convert_bfloat128_to_float128(
                        kv_bf16, SW_LINEAR);
                kv_chunks[vector_index * 2] = kv_f32.v1;
                kv_chunks[vector_index * 2 + 1] = kv_f32.v2;
            }

            #pragma unroll (7)
            for (int chunk = 0; chunk < 7; ++chunk) {
                float64 absmax =
                    v_f32_reduce_max(v_f32_abs_b(kv_chunks[chunk]));
                absmax = v_f32_shuffle_b(
                    absmax, broadcast_lane_zero, 0, absmax);
                const float64 scale_raw =
                    v_f32_max_b(absmax, 1.0e-4f) / 448.0f;
                const uint64 scale_bits = *((uint64*)&scale_raw);
                const uint64 encoded_scale =
                    ((scale_bits + 0x7fffff) >> 23) & 0xff;
                const uint64 inverse_scale_bits =
                    (254 - encoded_scale) << 23;
                const float64 inverse_scale =
                    *((float64*)&inverse_scale_bits);
                const float64 quantized = v_f32_min_b(
                    v_f32_max_b(
                        kv_chunks[chunk] * inverse_scale,
                        -448.0f),
                    448.0f);
                const uchar256 encoded =
                    dsv4_qkv_encode_e4m3fn_f32(quantized);
                int5 value_coords = {
                    value_base + chunk * 64, 0, 0, 0, 0};
                v_u8_st_tnsr_partial(
                    value_coords,
                    cache_storage_u8,
                    encoded,
                    63,
                    0);

                int5 scale_coords = {
                    scale_base + chunk, 0, 0, 0, 0};
                uint256 wide_scale_codes = {0};
                wide_scale_codes.v1 = encoded_scale;
                const uchar256 scale_codes =
                    v_convert_u32_to_u8_all_b(wide_scale_codes);
                v_u8_st_tnsr_partial(
                    scale_coords,
                    cache_storage_u8,
                    scale_codes,
                    0,
                    0);
            }

            const float64 roped =
                dsv4_qkv_apply_pairwise_rope_f32(
                    kv_chunks[7], cos_sin_cache, position);
            float128 rope_wide = {0};
            rope_wide.v1 = roped;
            const bfloat128 rope_bf16 =
                v_convert_f32_to_bf16_all_b(
                    rope_wide, SW_RHNE | SW_LINEAR);
            const uchar256 rope_raw = *((uchar256*)&rope_bf16);
            int5 rope_coords = {value_base + 448, 0, 0, 0, 0};
            v_u8_st_tnsr_partial(
                rope_coords, cache_storage_u8, rope_raw, 127, 0);
            int5 padding_scale_coords = {
                scale_base + 7, 0, 0, 0, 0};
            s_u8_st_g(
                gen_addr(padding_scale_coords, cache_storage_u8),
                (unsigned char)0);
        }
    }
}
#endif
