// SPDX-License-Identifier: Apache-2.0
/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

// Eight-token Qwen GDN verification for Gaudi2. Each index-space point
// processes 64 value rows for one value head.
// Recurrent state is loaded from the accepted checkpoint and every live token
// writes a new checkpoint, matching FlashInfer's MTP rollback contract.
float64_pair_t bf16_to_f32_linear_mtp(bfloat128 input)
{
    bfloat128_pair_t unpacked;
    unpacked.v1 = v_bf16_unpack_b(
        input,
        ((e_group_0) << 8) | ((e_every_second_element) << 9) |
            ((e_lower_half_group) << 10),
        unpacked.v1);
    unpacked.v2 = v_bf16_unpack_b(
        input,
        ((e_group_1) << 8) | ((e_every_second_element) << 9) |
            ((e_lower_half_group) << 10),
        unpacked.v2);

    const bfloat128 first_groups = unpacked.v1;
    unpacked.v1 = v_bf16_mov_dual_group_b(
        unpacked.v2, 0xFFFFFFFF, 0, 1, MkWr(1, 1), unpacked.v1);
    unpacked.v1 = v_bf16_mov_dual_group_b(
        first_groups, 0xFFFFFFFF, 1, 2, MkWr(1, 1), unpacked.v1);
    unpacked.v1 = v_bf16_mov_dual_group_b(
        unpacked.v2, 0xFFFFFFFF, 1, 3, MkWr(1, 1), unpacked.v1);

    unpacked.v2 = v_bf16_mov_dual_group_b(
        first_groups, 0xFFFFFFFF, 2, 0, MkWr(1, 1), unpacked.v2);
    unpacked.v2 = v_bf16_mov_dual_group_b(
        unpacked.v2, 0xFFFFFFFF, 2, 1, MkWr(1, 1), unpacked.v2);
    unpacked.v2 = v_bf16_mov_dual_group_b(
        first_groups, 0xFFFFFFFF, 3, 2, MkWr(1, 1), unpacked.v2);

    const float64_pair_t first = v_convert_bf16_to_f32_all_b(unpacked.v1);
    const float64_pair_t second = v_convert_bf16_to_f32_all_b(unpacked.v2);
    float64_pair_t result;
    result.v1 = first.v1;
    result.v2 = second.v1;
    return result;
}

void main(
    tensor state_cache,
    tensor packed_qkv,
    tensor decay,
    tensor beta,
    tensor state_indices,
    tensor num_accepted_tokens,
    tensor query_lengths,
    tensor output)
{
    const uchar256 broadcast_lane_zero = 0x80;
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
    const int key_dim = 128;
    const int value_dim = 128;
    const int value_heads = get_dim_size(state_cache, 2);
    const int state_slots = get_dim_size(state_cache, 3);
    const int packed_width = get_dim_size(packed_qkv, 0);
    const int num_tokens = get_dim_size(packed_qkv, 1);
    const int key_heads = (packed_width - value_heads * value_dim) / (2 * key_dim);
    const int head_repeat = value_heads / key_heads;
    const int key_offset = key_heads * key_dim;
    const int value_offset = 2 * key_offset;
    const float q_scale = 0.08838834764831845f;

    for (int batch = index_start[3]; batch < index_end[3]; ++batch) {
        int5 batch_coords = {batch, 0, 0, 0, 0};
        int accepted = s_i32_ld_g(gen_addr(batch_coords, num_accepted_tokens));
        accepted = accepted < 1 ? 1 : accepted;
        accepted = accepted > num_tokens ? num_tokens : accepted;
        const int query_length = s_i32_ld_g(gen_addr(batch_coords, query_lengths));

        int5 initial_index_coords = {accepted - 1, batch, 0, 0, 0};
        const int initial_source_slot = s_i32_ld_g(gen_addr(initial_index_coords, state_indices));
        const bool valid_initial_source = initial_source_slot >= 0 && initial_source_slot < state_slots;

        for (int value_head = index_start[2]; value_head < index_end[2]; ++value_head) {
            const int key_head = value_head / head_repeat;
            const int value_start = index_start[1] * 64;
            const int value_end = index_end[1] * 64;
            int source_slot = initial_source_slot;
            bool valid_source = valid_initial_source;

            for (int token = 0; token < num_tokens; ++token) {
                int5 destination_coords = {token, batch, 0, 0, 0};
                const int destination_slot = s_i32_ld_g(gen_addr(destination_coords, state_indices));
                const bool active = token < query_length && valid_source &&
                    destination_slot >= 0 && destination_slot < state_slots;

                if (!active) {
                    const bfloat128 zero = 0.0f;
                    for (int value_row = value_start; value_row < value_end; ++value_row) {
                        int5 output_coords = {value_row, value_head, token, batch, 0};
                        v_bf16_st_tnsr_partial(output_coords, output, zero, 0, 0);
                    }
                    continue;
                }

                int5 packed_coords = {key_head * key_dim, token, batch, 0, 0};
                const bfloat128 q_bf16 = v_bf16_ld_tnsr_b(packed_coords, packed_qkv);
                packed_coords[0] = key_offset + key_head * key_dim;
                const bfloat128 k_bf16 = v_bf16_ld_tnsr_b(packed_coords, packed_qkv);
                const float64_pair_t q_f32 = bf16_to_f32_linear_mtp(q_bf16);
                const float64_pair_t k_f32 = bf16_to_f32_linear_mtp(k_bf16);
                float64 q_lo = q_f32.v1;
                float64 q_hi = q_f32.v2;
                float64 k_lo = k_f32.v1;
                float64 k_hi = k_f32.v2;

                float64 q_norm = v_f32_mac_b(q_lo, q_lo, q_hi * q_hi);
                q_norm = v_f32_reduce_add(q_norm);
                q_norm = v_f32_shuffle_b(q_norm, broadcast_lane_zero, 0, q_norm);
                q_norm = v_rsqrt_f32(q_norm + 1e-6f) * q_scale;
                q_lo *= q_norm;
                q_hi *= q_norm;

                float64 k_norm = v_f32_mac_b(k_lo, k_lo, k_hi * k_hi);
                k_norm = v_f32_reduce_add(k_norm);
                k_norm = v_f32_shuffle_b(k_norm, broadcast_lane_zero, 0, k_norm);
                k_norm = v_rsqrt_f32(k_norm + 1e-6f);
                k_lo *= k_norm;
                k_hi *= k_norm;

                int5 scalar_coords = {value_head, token, batch, 0, 0};
                const float64 decay_value = s_f32_ld_g(gen_addr(scalar_coords, decay));
                const bf16 beta_bf16 = s_bf16_ld_g(gen_addr(scalar_coords, beta));
                const float64 beta_value = (float)beta_bf16;

                for (int value_row = value_start; value_row < value_end; ++value_row) {
                    int5 state_coords = {0, value_row, value_head, source_slot, 0};
                    float64 state_lo = v_f32_ld_tnsr_b(state_coords, state_cache);
                    state_coords[0] = 64;
                    float64 state_hi = v_f32_ld_tnsr_b(state_coords, state_cache);

                    int5 value_coords = {
                        value_offset + value_head * value_dim + value_row,
                        token,
                        batch,
                        0,
                        0};
                    const bf16 value_bf16 = s_bf16_ld_g(gen_addr(value_coords, packed_qkv));
                    const float64 v_value = (float)value_bf16;

                    state_lo *= decay_value;
                    state_hi *= decay_value;

                    float64 projection = state_lo * k_lo;
                    projection = v_f32_mac_b(state_hi, k_hi, projection);
                    projection = v_f32_reduce_add(projection);
                    projection = v_f32_shuffle_b(projection, broadcast_lane_zero, 0, projection);
                    const float64 delta = (v_value - projection) * beta_value;

                    state_lo = v_f32_mac_b(k_lo, delta, state_lo);
                    state_hi = v_f32_mac_b(k_hi, delta, state_hi);

                    float64 out_value = state_lo * q_lo;
                    out_value = v_f32_mac_b(state_hi, q_hi, out_value);
                    out_value = v_f32_reduce_add(out_value);

                    state_coords[3] = destination_slot;
                    state_coords[0] = 0;
                    v_f32_st_tnsr(state_coords, state_cache, state_lo);
                    state_coords[0] = 64;
                    v_f32_st_tnsr(state_coords, state_cache, state_hi);

                    int5 output_coords = {value_row, value_head, token, batch, 0};
                    float64_pair_t output_f32;
                    output_f32.v1 = out_value;
                    output_f32.v2 = out_value;
                    const bfloat128 output_bf16 = v_convert_f32_to_bf16_all_b(output_f32);
                    v_bf16_st_tnsr_partial(output_coords, output, output_bf16, 0, 0);
                }

                source_slot = destination_slot;
                valid_source = 1;
            }
        }
    }
}
