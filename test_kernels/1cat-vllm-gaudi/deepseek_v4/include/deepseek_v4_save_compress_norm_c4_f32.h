/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

#ifndef DSV4_COMPRESS_NORM_BF16_CONSTANTS
#define DSV4_COMPRESS_NORM_BF16_CONSTANTS 0
#endif

#ifndef DSV4_COMPRESS_INPUTS_BF16
#define DSV4_COMPRESS_INPUTS_BF16 DSV4_COMPRESS_NORM_BF16_CONSTANTS
#endif

#ifndef DSV4_COMPRESS_NORM_BF16
#define DSV4_COMPRESS_NORM_BF16 DSV4_COMPRESS_NORM_BF16_CONSTANTS
#endif

#ifndef DSV4_COMPRESS_VECTOR_ROPE
#define DSV4_COMPRESS_VECTOR_ROPE DSV4_COMPRESS_NORM_BF16_CONSTANTS
#endif

#ifndef DSV4_COMPRESS_ORDERED_COMPLETION
#define DSV4_COMPRESS_ORDERED_COMPLETION 0
#endif

#if DSV4_COMPRESS_INPUTS_BF16
float64 dsv4_load_bf16_chunk_as_f32(
    tensor input,
    int feature,
    int token)
{
    int5 coords = {feature, token, 0, 0, 0};
    const bfloat128 input_bf16 =
        v_bf16_ld_tnsr_partial_b(coords, input, 63, 0);
    return convert_bfloat128_to_float128(
        input_bf16, SW_LINEAR).v1;
}
#endif

float64 dsv4_round_to_bf16_f32(float64 values)
{
    float128 wide = {0};
    wide.v1 = values;
    const bfloat128 rounded =
        v_convert_f32_to_bf16_all_b(wide, SW_RHNE);
    return v_convert_bf16_to_f32_all_b(rounded).v1;
}

uchar256 dsv4_encode_e4m3fn_f32(float64 values)
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
    // Exponent-shifting the half-value is exact for normal FP8 values and
    // exposes E4M3FN's finite exponent-15 range (codes 120..126).
    return v_u8_sel_geq_u8_b(
        half_magnitude, 8, shifted, direct_raw);
}

float64 dsv4_apply_pairwise_rope_f32(
    float64 values,
    tensor cos_sin_cache,
    int position)
{
#if DSV4_COMPRESS_VECTOR_ROPE
    int5 cache_coords = {0, position, 0, 0, 0};
    const float64 cos_sin =
        v_f32_ld_tnsr_b(cache_coords, cos_sin_cache);
    const uint64 lanes = V_LANE_ID_32;
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
    uint256 wide_directions = {0};
    wide_directions.v1 =
        (group_lanes & 0xfffffffe) + current_group + 0x80;
    wide_directions.v2 = wide_directions.v1;
    wide_directions.v3 = wide_directions.v1;
    wide_directions.v4 = wide_directions.v1;
    const uchar256 real_directions =
        v_convert_u32_to_u8_all_b(wide_directions);
    wide_directions.v1 = (group_lanes | 1) + current_group + 0x80;
    wide_directions.v2 = wide_directions.v1;
    wide_directions.v3 = wide_directions.v1;
    wide_directions.v4 = wide_directions.v1;
    const uchar256 imag_directions =
        v_convert_u32_to_u8_all_b(wide_directions);
    wide_directions.v1 = pair_offsets + alternate_group + 0x80;
    wide_directions.v2 = wide_directions.v1;
    wide_directions.v3 = wide_directions.v1;
    wide_directions.v4 = wide_directions.v1;
    const uchar256 trig_directions =
        v_convert_u32_to_u8_all_b(wide_directions);

    const float64 real = v_f32_shuffle_b(
        values, real_directions, 0, 0.0f);
    const float64 imag = v_f32_shuffle_b(
        values, imag_directions, 0, 0.0f);
    const float64 cos_values = v_f32_shuffle_b(
        cos_groups, trig_directions, 0, 0.0f);
    const float64 sin_values = v_f32_shuffle_b(
        sin_groups, trig_directions, 0, 0.0f);
    const float64 roped_real =
        real * cos_values - imag * sin_values;
    const float64 roped_imag =
        imag * cos_values + real * sin_values;
    return v_f32_sel_eq_u32_b(
        lanes & 1, 1, roped_imag, roped_real);
#else
    float64 roped = 0.0f;
    const uint64 lanes = V_LANE_ID_32;
    #pragma unroll (32)
    for (int pair = 0; pair < 32; ++pair) {
        int5 cos_coords = {pair, position, 0, 0, 0};
        int5 sin_coords = {pair + 32, position, 0, 0, 0};
        const float cos_value =
            s_f32_ld_g(gen_addr(cos_coords, cos_sin_cache));
        const float sin_value =
            s_f32_ld_g(gen_addr(sin_coords, cos_sin_cache));
        const float64 real =
            v_broadcast_element_f32(values, pair * 2);
        const float64 imag =
            v_broadcast_element_f32(values, pair * 2 + 1);
        const float64 roped_real =
            real * cos_value - imag * sin_value;
        const float64 roped_imag =
            imag * cos_value + real * sin_value;
        roped = v_f32_sel_eq_u32_b(
            lanes, pair * 2, roped_real, roped);
        roped = v_f32_sel_eq_u32_b(
            lanes, pair * 2 + 1, roped_imag, roped);
    }
    return roped;
#endif
}

// Decode-specialized compressor. One program owns one token so state update,
// compression, normalization, RoPE and quantization form one launch. C4 and
// C128 are selected from the APE row count.
void main(
    tensor storage_u8,
    tensor state_geometry,
    tensor kv_cache_geometry,
    tensor kv,
    tensor score,
    tensor ape,
    tensor positions,
    tensor slot_mapping,
    tensor token_to_req_indices,
    tensor block_table,
    tensor rms_norm_weight,
    tensor rms_norm_eps,
    tensor cos_sin_cache,
    tensor kv_slot_mapping,
#if DSV4_COMPRESS_ORDERED_COMPLETION
    tensor completion_input,
#endif
    tensor mutation_token)
{
    const uchar256 broadcast_lane_zero = 0x80;
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;

    int5 geometry_coords = {0, 0, 0, 0, 0};
    const int base_offset =
        s_i32_ld_g(gen_addr(geometry_coords, state_geometry));
    geometry_coords[0] = 1;
    const int block_stride =
        s_i32_ld_g(gen_addr(geometry_coords, state_geometry));
    geometry_coords[0] = 2;
    const int token_stride =
        s_i32_ld_g(gen_addr(geometry_coords, state_geometry));
    geometry_coords[0] = 3;
    const int block_size =
        s_i32_ld_g(gen_addr(geometry_coords, state_geometry));
    geometry_coords[0] = 4;
    const int block_count =
        s_i32_ld_g(gen_addr(geometry_coords, state_geometry));

    int5 cache_geometry_coords = {0, 0, 0, 0, 0};
    const int cache_base_offset =
        s_i32_ld_g(gen_addr(cache_geometry_coords, kv_cache_geometry));
    cache_geometry_coords[0] = 1;
    const int cache_block_stride =
        s_i32_ld_g(gen_addr(cache_geometry_coords, kv_cache_geometry));
    cache_geometry_coords[0] = 2;
    const int cache_block_size =
        s_i32_ld_g(gen_addr(cache_geometry_coords, kv_cache_geometry));
    cache_geometry_coords[0] = 3;
    const int cache_block_count =
        s_i32_ld_g(gen_addr(cache_geometry_coords, kv_cache_geometry));

    int5 eps_coords = {0, 0, 0, 0, 0};
    const float eps = s_f32_ld_g(gen_addr(eps_coords, rms_norm_eps));
    const int state_width = get_dim_size(kv, 0);
    const int head_dim = get_dim_size(rms_norm_weight, 0);
    const int compress_ratio = get_dim_size(ape, 1);
    const int coff = state_width / head_dim;
    const int window_size = coff * compress_ratio;
    const int state_chunks = state_width / 64;
    const int head_chunks = head_dim / 64;
    const int block_table_width = get_dim_size(block_table, 0);
    const int slot_count = block_count * block_size;

    for (int token = index_start[0]; token < index_end[0]; ++token) {
        int5 token_coords = {token, 0, 0, 0, 0};
        const int position =
            s_i32_ld_g(gen_addr(token_coords, positions));
        const int slot =
            s_i32_ld_g(gen_addr(token_coords, slot_mapping));
        const int request =
            s_i32_ld_g(gen_addr(token_coords, token_to_req_indices));
        const int kv_slot =
            s_i32_ld_g(gen_addr(token_coords, kv_slot_mapping));
        const bool valid_slot =
            slot >= 0 && slot < slot_count && position >= 0;

        if (valid_slot) {
            const int block = slot / block_size;
            const int token_in_block = slot - block * block_size;
            const int destination =
                base_offset + block * block_stride
                + token_in_block * token_stride;
            const int ape_row =
                position - (position / compress_ratio) * compress_ratio;
            for (int chunk = 0; chunk < state_chunks; ++chunk) {
                const int feature = chunk * 64;
#if DSV4_COMPRESS_INPUTS_BF16
                const float64 kv_value =
                    dsv4_load_bf16_chunk_as_f32(kv, feature, token);
                const float64 score_value =
                    dsv4_load_bf16_chunk_as_f32(score, feature, token);
#else
                int5 source_coords = {feature, token, 0, 0, 0};
                const float64 kv_value =
                    v_f32_ld_tnsr_b(source_coords, kv);
                const float64 score_value =
                    v_f32_ld_tnsr_b(source_coords, score);
#endif
                int5 ape_coords = {feature, ape_row, 0, 0, 0};
                const float64 ape_value =
                    v_f32_ld_tnsr_b(ape_coords, ape);
                int5 state_coords = {
                    destination + feature * 4, 0, 0, 0, 0};
                const uchar256 kv_raw = *((uchar256*)&kv_value);
                v_u8_st_tnsr(state_coords, storage_u8, kv_raw);
                state_coords[0] += state_width * 4;
                const float64 score_ape = score_value + ape_value;
                const uchar256 score_raw = *((uchar256*)&score_ape);
                v_u8_st_tnsr(state_coords, storage_u8, score_raw);
            }
        }

        const bool boundary =
            valid_slot && request >= 0 &&
            ((position + 1) % compress_ratio == 0) &&
            kv_slot >= 0 && kv_slot < cache_block_count * cache_block_size;
        int5 mutation_coords = {token, 0, 0, 0, 0};
#if !DSV4_COMPRESS_ORDERED_COMPLETION
        s_u8_st_g(
            gen_addr(mutation_coords, mutation_token),
            (unsigned char)boundary);
#endif
        if (!boundary) {
#if DSV4_COMPRESS_ORDERED_COMPLETION
            const int completion_value = s_i32_ld_g(
                gen_addr(mutation_coords, completion_input));
            s_i32_st_g(
                gen_addr(mutation_coords, mutation_token),
                completion_value);
#endif
            continue;
        }

        float64 compressed_chunks[8] = {0};
        float64 total_sumsq = 0.0f;
        for (int chunk = 0; chunk < head_chunks; ++chunk) {
            const int feature = chunk * 64;
            float64 running_max = -3.402823466e+38f;
            float64 running_sum = 0.0f;
            float64 running_product = 0.0f;

            if (boundary) {
                const int window_start = position - window_size + 1;
                for (int row = 0; row < window_size; ++row) {
                    const int state_position = window_start + row;
                    if (state_position < 0) {
                        continue;
                    }

                    float64 value;
                    float64 score_value;
                    const int head_offset =
                        (row / compress_ratio) * head_dim;
                    if (row == window_size - 1) {
#if DSV4_COMPRESS_INPUTS_BF16
                        value = dsv4_load_bf16_chunk_as_f32(
                            kv, feature + head_offset, token);
                        score_value = dsv4_load_bf16_chunk_as_f32(
                            score, feature + head_offset, token);
#else
                        int5 source_coords = {
                            feature + head_offset, token, 0, 0, 0};
                        value = v_f32_ld_tnsr_b(source_coords, kv);
                        score_value =
                            v_f32_ld_tnsr_b(source_coords, score);
#endif
                        const int ape_row = position -
                            (position / compress_ratio) * compress_ratio;
                        int5 ape_coords = {
                            feature + head_offset, ape_row, 0, 0, 0};
                        score_value += v_f32_ld_tnsr_b(ape_coords, ape);
                    } else {
                        const int logical_block =
                            state_position / block_size;
                        if (logical_block < 0 ||
                            logical_block >= block_table_width) {
                            continue;
                        }
                        int5 table_coords = {
                            logical_block, request, 0, 0, 0};
                        const int physical_block = s_i32_ld_g(
                            gen_addr(table_coords, block_table));
                        if (physical_block < 0 ||
                            physical_block >= block_count) {
                            continue;
                        }
                        const int position_in_block =
                            state_position - logical_block * block_size;
                        const int source =
                            base_offset + physical_block * block_stride
                            + position_in_block * token_stride
                            + (head_offset + feature) * 4;
                        int5 state_coords = {source, 0, 0, 0, 0};
                        const uchar256 value_raw =
                            v_u8_ld_tnsr_b(state_coords, storage_u8);
                        value = *((float64*)&value_raw);
                        state_coords[0] += state_width * 4;
                        const uchar256 score_raw =
                            v_u8_ld_tnsr_b(state_coords, storage_u8);
                        score_value = *((float64*)&score_raw);
                    }

                    const float64 next_max =
                        v_f32_max_b(running_max, score_value);
                    const float64 previous_scale =
                        v_exp_cephes_f32(running_max - next_max);
                    const float64 weight =
                        v_exp_cephes_f32(score_value - next_max);
                    running_sum = running_sum * previous_scale + weight;
                    running_product =
                        running_product * previous_scale + value * weight;
                    running_max = next_max;
                }
            }

            float64 compressed = 0.0f;
            if (boundary) {
                compressed = running_product * v_reciprocal_f32(running_sum);
            }
            compressed_chunks[chunk] = compressed;
            float64 chunk_sumsq =
                v_f32_reduce_add(compressed * compressed);
            chunk_sumsq = v_f32_shuffle_b(
                chunk_sumsq,
                broadcast_lane_zero,
                0,
                chunk_sumsq);
            total_sumsq += chunk_sumsq;
        }

        const float64 rrms = v_rsqrt_f32(
            total_sumsq / (float)head_dim + eps);
#if DSV4_COMPRESS_NORM_BF16
        for (int chunk_pair = 0;
             chunk_pair < head_chunks / 2;
             ++chunk_pair) {
            const int feature = chunk_pair * 128;
            int5 weight_coords = {feature, 0, 0, 0, 0};
            const bfloat128 norm_weight_bf16 =
                v_bf16_ld_tnsr_b(weight_coords, rms_norm_weight);
            const float128 norm_weight =
                convert_bfloat128_to_float128(
                    norm_weight_bf16, SW_LINEAR);
            compressed_chunks[chunk_pair * 2] =
                compressed_chunks[chunk_pair * 2]
                * rrms * norm_weight.v1;
            compressed_chunks[chunk_pair * 2 + 1] =
                compressed_chunks[chunk_pair * 2 + 1]
                * rrms * norm_weight.v2;
        }
#else
        for (int chunk = 0; chunk < head_chunks; ++chunk) {
            const int feature = chunk * 64;
            int5 weight_coords = {feature, 0, 0, 0, 0};
            const float64 norm_weight =
                v_f32_ld_tnsr_b(weight_coords, rms_norm_weight);
            compressed_chunks[chunk] =
                compressed_chunks[chunk] * rrms * norm_weight;
        }
#endif

        const int compressed_position =
            (position / compress_ratio) * compress_ratio;
        const int cache_block = kv_slot / cache_block_size;
        const int cache_token = kv_slot - cache_block * cache_block_size;
        if (head_dim == 512) {
            const int value_base =
                cache_base_offset + cache_block * cache_block_stride
                + cache_token * 576;
            const int scale_base =
                cache_base_offset + cache_block * cache_block_stride
                + cache_block_size * 576 + cache_token * 8;
            #pragma unroll (7)
            for (int chunk = 0; chunk < 7; ++chunk) {
                const float64 rounded =
                    dsv4_round_to_bf16_f32(compressed_chunks[chunk]);
                float64 absmax =
                    v_f32_reduce_max(v_f32_abs_b(rounded));
                absmax = v_f32_shuffle_b(
                    absmax, broadcast_lane_zero, 0, absmax);
                const float64 scale_raw =
                    v_f32_max_b(absmax, 1.0e-4f) / 448.0f;
                const uint64 scale_bits =
                    *((uint64*)&scale_raw);
                const uint64 encoded_scale =
                    ((scale_bits + 0x7fffff) >> 23) & 0xff;
                const uint64 inverse_scale_bits =
                    (254 - encoded_scale) << 23;
                const float64 inverse_scale =
                    *((float64*)&inverse_scale_bits);
                const float64 quantized = v_f32_min_b(
                    v_f32_max_b(
                        rounded * inverse_scale, -448.0f),
                    448.0f);
                const uchar256 encoded =
                    dsv4_encode_e4m3fn_f32(quantized);
                int5 value_coords = {
                    value_base + chunk * 64, token, 0, 0, 0};
                value_coords[1] = 0;
                v_u8_st_tnsr_partial(
                    value_coords,
                    storage_u8,
                    encoded,
                    63,
                    0);
                int5 scale_coords = {
                    scale_base + chunk, token, 0, 0, 0};
                scale_coords[1] = 0;
                uint256 wide_scale_codes = {0};
                wide_scale_codes.v1 = encoded_scale;
                const uchar256 scale_codes =
                    v_convert_u32_to_u8_all_b(wide_scale_codes);
                v_u8_st_tnsr_partial(
                    scale_coords,
                    storage_u8,
                    scale_codes,
                    0,
                    0);
            }

            const float64 roped = dsv4_apply_pairwise_rope_f32(
                compressed_chunks[7],
                cos_sin_cache,
                compressed_position);
            float128 rope_wide = {0};
            rope_wide.v1 = roped;
            const bfloat128 rope_bf16 =
                v_convert_f32_to_bf16_all_b(
                    rope_wide, SW_RHNE | SW_LINEAR);
            const uchar256 rope_raw = *((uchar256*)&rope_bf16);
            int5 rope_coords = {
                value_base + 448, token, 0, 0, 0};
            rope_coords[1] = 0;
            v_u8_st_tnsr_partial(
                rope_coords, storage_u8, rope_raw, 127, 0);
            int5 padding_scale_coords = {
                scale_base + 7, token, 0, 0, 0};
            padding_scale_coords[1] = 0;
            s_u8_st_g(
                gen_addr(padding_scale_coords, storage_u8),
                (unsigned char)0);
        } else {
            const int value_base =
                cache_base_offset + cache_block * cache_block_stride
                + cache_token * 128;
            const int scale_base =
                cache_base_offset + cache_block * cache_block_stride
                + cache_block_size * 128 + cache_token * 4;
            float64 rounded_chunks[2] = {0};
            rounded_chunks[0] =
                dsv4_round_to_bf16_f32(compressed_chunks[0]);
            rounded_chunks[1] = dsv4_round_to_bf16_f32(
                dsv4_apply_pairwise_rope_f32(
                    compressed_chunks[1],
                    cos_sin_cache,
                    compressed_position));
            float64 absmax = v_f32_max_b(
                v_f32_reduce_max(v_f32_abs_b(rounded_chunks[0])),
                v_f32_reduce_max(v_f32_abs_b(rounded_chunks[1])));
            absmax = v_f32_shuffle_b(
                absmax, broadcast_lane_zero, 0, absmax);
            const float64 scale_raw =
                v_f32_max_b(absmax, 1.0e-4f) / 448.0f;
            const uint64 scale_bits = *((uint64*)&scale_raw);
            const uint64 encoded_scale =
                ((scale_bits + 0x7fffff) >> 23) & 0xff;
            const uint64 scale_value_bits = encoded_scale << 23;
            const float64 scale = *((float64*)&scale_value_bits);
            const uint64 inverse_scale_bits =
                (254 - encoded_scale) << 23;
            const float64 inverse_scale =
                *((float64*)&inverse_scale_bits);
            #pragma unroll (2)
            for (int chunk = 0; chunk < 2; ++chunk) {
                const float64 quantized = v_f32_min_b(
                    v_f32_max_b(
                        rounded_chunks[chunk] * inverse_scale,
                        -448.0f),
                    448.0f);
                const uchar256 encoded =
                    dsv4_encode_e4m3fn_f32(quantized);
                int5 value_coords = {
                    value_base + chunk * 64, token, 0, 0, 0};
                value_coords[1] = 0;
                v_u8_st_tnsr_partial(
                    value_coords,
                    storage_u8,
                    encoded,
                    63,
                    0);
            }
            int5 scale_coords = {scale_base, token, 0, 0, 0};
            scale_coords[1] = 0;
            const uchar256 scale_raw_bytes =
                *((uchar256*)&scale);
            v_u8_st_tnsr_partial(
                scale_coords,
                storage_u8,
                scale_raw_bytes,
                3,
                0);
        }
#if DSV4_COMPRESS_ORDERED_COMPLETION
        const int completion_value = s_i32_ld_g(
            gen_addr(mutation_coords, completion_input));
        s_i32_st_g(
            gen_addr(mutation_coords, mutation_token),
            completion_value);
#endif
    }
}
