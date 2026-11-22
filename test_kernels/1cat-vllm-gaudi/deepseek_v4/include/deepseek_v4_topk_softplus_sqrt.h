/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

#define DSV4_ROUTER_EXPERTS 256
#define DSV4_ROUTER_TOPK 6
#define DSV4_ROUTER_VECTOR_WIDTH 64
#define DSV4_ROUTER_CHUNKS \
    (DSV4_ROUTER_EXPERTS / DSV4_ROUTER_VECTOR_WIDTH)
#define DSV4_ROUTER_SCALE 1.5f

static inline float64 dsv4_router_score(float64 logits)
{
    const float64 clamped = v_f32_min_b(
        v_f32_max_b(logits, -8.0f), 7.999999f);
    const float64 interval_position = (clamped + 8.0f) * 2.0f;
    const int64 interval = v_convert_f32_to_i32_b(
        interval_position, e_round_down << 16);
    const float64 interval_f32 = v_convert_i32_to_f32_b(
        interval, e_round_half_ne << 16);
    const float64 center = -7.75f + interval_f32 * 0.5f;
    const float64 delta = clamped - center;
    const uint64 lookup_index = (uint64)interval;
    const int function_id = 0x3;
    const float64 c0 = v_f32_lookup_1c(
        lookup_index, function_id, SW_LUT_PTR, (float64){0});
    const float64_pair_t c1c2 = v_f32_lookup_2c(
        lookup_index,
        function_id,
        SW_LUT_PTR,
        (float64_pair_t){0});
    const float64 lut_score =
        c0 + delta * (c1c2.v1 + delta * c1c2.v2);

    // softplus(x) differs from x by less than 3.4e-4 above 8.0. Use two
    // Newton refinements of a bit-level reciprocal-sqrt seed there so large
    // outliers do not collapse onto the final LUT interval.
    const float64 positive = v_f32_max_b(logits, 1.0e-20f);
    int64 inverse_bits =
        (int64)0x5f375a86 - ((*((int64*)&positive)) >> 1);
    float64 inverse_root = *((float64*)&inverse_bits);
    const float64 half_value = positive * 0.5f;
    inverse_root = inverse_root * (
        1.5f - half_value * inverse_root * inverse_root);
    inverse_root = inverse_root * (
        1.5f - half_value * inverse_root * inverse_root);
    const float64 large_score = positive * inverse_root;
    return v_f32_sel_grt_f32_b(logits, 8.0f, large_score, lut_score);
}

// Decode-specialized equivalent of vLLM's CUDA topk_softplus_sqrt kernel.
// Selection uses score+bias while the normalized MoE weights use the
// unbiased score, matching DeepSeek V4's noaux_tc router semantics.
void main(
    tensor gating_output,
    tensor correction_bias,
    tensor topk_weights,
    tensor topk_ids,
    tensor score_lut)
{
    set_lut_32(score_lut);
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
    const int64 lanes = (int64)V_LANE_ID_32;

    for (int token = index_start[0]; token < index_end[0]; ++token) {
        float64 scores[DSV4_ROUTER_CHUNKS];
        float64 choices[DSV4_ROUTER_CHUNKS];
        int64 expert_ids[DSV4_ROUTER_CHUNKS];

        #pragma unroll
        for (int chunk = 0; chunk < DSV4_ROUTER_CHUNKS; ++chunk) {
            const int offset = chunk * DSV4_ROUTER_VECTOR_WIDTH;
            int5 logits_coords = {offset, token, 0, 0, 0};
            int5 bias_coords = {offset, 0, 0, 0, 0};
            scores[chunk] = dsv4_router_score(
                v_f32_ld_tnsr_b(logits_coords, gating_output));
            choices[chunk] = scores[chunk]
                + v_f32_ld_tnsr_b(bias_coords, correction_bias);
            expert_ids[chunk] = lanes + offset;
        }

        float64 selected_weights[DSV4_ROUTER_TOPK];
        int64 selected_ids[DSV4_ROUTER_TOPK];
        float64 weight_sum = 0.0f;

        #pragma unroll
        for (int rank = 0; rank < DSV4_ROUTER_TOPK; ++rank) {
            float64 maximum = v_f32_reduce_max(choices[0]);
            #pragma unroll
            for (int chunk = 1; chunk < DSV4_ROUTER_CHUNKS; ++chunk) {
                maximum = v_f32_max_b(
                    maximum,
                    v_f32_reduce_max(choices[chunk]));
            }

            int64 selected_id = 2147483647;
            #pragma unroll
            for (int chunk = 0; chunk < DSV4_ROUTER_CHUNKS; ++chunk) {
                const bool64 is_max =
                    v_f32_cmp_eq_b(choices[chunk], maximum);
                const int64 candidates = v_i32_mov_vb(
                    expert_ids[chunk],
                    0,
                    (int64)2147483647,
                    is_max,
                    0);
                selected_id = v_i32_min_b(
                    selected_id,
                    v_i32_reduce_min(candidates));
            }

            float64 selected_weight = 0.0f;
            #pragma unroll
            for (int chunk = 0; chunk < DSV4_ROUTER_CHUNKS; ++chunk) {
                const bool64 selected =
                    v_i32_cmp_eq_b(expert_ids[chunk], selected_id);
                const float64 selected_lanes = v_f32_mov_vb(
                    scores[chunk], 0, 0.0f, selected, 0);
                selected_weight += v_f32_reduce_add(selected_lanes);
                choices[chunk] = v_f32_mov_vb(
                    -3.402823466e+38f,
                    0,
                    choices[chunk],
                    selected,
                    0);
            }

            selected_ids[rank] = selected_id;
            selected_weights[rank] = selected_weight;
            weight_sum += selected_weight;
        }

        const float64 inverse_sum = v_reciprocal_f32(
            v_f32_max_b(weight_sum, 1.0e-20f));
        #pragma unroll
        for (int rank = 0; rank < DSV4_ROUTER_TOPK; ++rank) {
            int5 output_coords = {0, rank, token, 0, 0};
            const float64 weight =
                selected_weights[rank] * inverse_sum * DSV4_ROUTER_SCALE;
            v_f32_st_tnsr_partial(
                output_coords, topk_weights, weight, 0, 0);
            v_i32_st_tnsr_partial(
                output_coords, topk_ids, selected_ids[rank], 0, 0);
        }
    }
}
