// SPDX-License-Identifier: Apache-2.0
/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

// Fused Qwen3.8 DFlash2 edge scoring and greedy path selection. The portable
// graph builds all 16x16 predecessor/successor scores before walking one row.
// This kernel follows only the selected predecessor and evaluates 16 scores
// per step, reducing codebook reads and dot products by about 16x.
float64_pair_t bf16_to_f32_linear_score_select(bfloat128 input)
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
    tensor predecessor_table,
    tensor successor_table,
    tensor candidate_ids,
    tensor unary_logits,
    tensor hidden,
    tensor anchor_token_ids,
    tensor output)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
    const int vocab_size = get_dim_size(predecessor_table, 1);
    const int num_steps = get_dim_size(candidate_ids, 1);
    const int top_k = get_dim_size(candidate_ids, 0);

    for (int batch = index_start[0]; batch < index_end[0]; ++batch) {
        int5 anchor_coords = {batch, 0, 0, 0, 0};
        int predecessor_token =
            s_i32_ld_g(gen_addr(anchor_coords, anchor_token_ids));

        for (int step = 0; step < num_steps; ++step) {
            int predecessor_id = (int)predecessor_token;
            predecessor_id = predecessor_id >= 0 && predecessor_id < vocab_size ?
                predecessor_id : 0;

            int5 hidden_coords = {0, step, batch, 0, 0};
            int5 predecessor_coords = {0, predecessor_id, 0, 0, 0};
            const bfloat128 hidden_0 = v_bf16_ld_tnsr_b(hidden_coords, hidden);
            hidden_coords[0] = 128;
            const bfloat128 hidden_1 = v_bf16_ld_tnsr_b(hidden_coords, hidden);
            const bfloat128 predecessor_0 =
                v_bf16_ld_tnsr_b(predecessor_coords, predecessor_table);
            predecessor_coords[0] = 128;
            const bfloat128 predecessor_1 =
                v_bf16_ld_tnsr_b(predecessor_coords, predecessor_table);

            const bfloat128 weighted_0 = v_bf16_mul_b(predecessor_0, hidden_0);
            const bfloat128 weighted_1 = v_bf16_mul_b(predecessor_1, hidden_1);
            const float64_pair_t weighted_f32_0 =
                bf16_to_f32_linear_score_select(weighted_0);
            const float64_pair_t weighted_f32_1 =
                bf16_to_f32_linear_score_select(weighted_1);

            float best_score = -3.402823466e+38F;
            int best_token = 0;
            for (int candidate = 0; candidate < top_k; ++candidate) {
                int5 candidate_coords = {candidate, step, batch, 0, 0};
                const int candidate_token =
                    s_i32_ld_g(gen_addr(candidate_coords, candidate_ids));
                const int candidate_id = candidate_token;
                float score = -3.402823466e+38F;
                if (candidate_id >= 0 && candidate_id < vocab_size) {
                    int5 successor_coords = {0, candidate_id, 0, 0, 0};
                    const bfloat128 successor_0 =
                        v_bf16_ld_tnsr_b(successor_coords, successor_table);
                    successor_coords[0] = 128;
                    const bfloat128 successor_1 =
                        v_bf16_ld_tnsr_b(successor_coords, successor_table);
                    const float64_pair_t successor_f32_0 =
                        bf16_to_f32_linear_score_select(successor_0);
                    const float64_pair_t successor_f32_1 =
                        bf16_to_f32_linear_score_select(successor_1);

                    float64 dot = weighted_f32_0.v1 * successor_f32_0.v1;
                    dot = v_f32_mac_b(
                        weighted_f32_0.v2, successor_f32_0.v2, dot);
                    dot = v_f32_mac_b(
                        weighted_f32_1.v1, successor_f32_1.v1, dot);
                    dot = v_f32_mac_b(
                        weighted_f32_1.v2, successor_f32_1.v2, dot);
                    dot = v_f32_reduce_add(dot);

                    // TPC vectors cannot be scalar-subscripted. Reduction
                    // broadcasts the sum, so round-trip lane zero through
                    // core-local SLM and match einsum's BF16 output rounding.
                    v_f32_st_l_v(0, dot);
                    const float dot_f32 = s_f32_ld_l(0);
                    const bf16 dot_bf16 = s_convert_f32_to_bf16(dot_f32);
                    int5 unary_coords = {candidate, step, batch, 0, 0};
                    score = (float)dot_bf16 +
                        s_f32_ld_g(gen_addr(unary_coords, unary_logits));
                }

                if (candidate == 0 || score > best_score) {
                    best_score = score;
                    best_token = candidate_token;
                }
            }

            int5 output_coords = {step, batch, 0, 0, 0};
            s_i32_st_g(gen_addr(output_coords, output), best_token);
            predecessor_token = best_token;
        }
    }
}
