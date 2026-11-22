// SPDX-License-Identifier: Apache-2.0
// Repeated-max selection follows the upstream DeepSeek top-6 contract.
// Input scores retain the model's FP32 softplus/sqrt computation.
void main(tensor input, tensor text_bias, tensor image_bias, tensor image_mask,
          tensor output_ids, tensor output_weights) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int64 lanes = (int64)V_LANE_ID_32;
    for (int token = start[0]; token < end[0]; ++token) {
        const int5 image_at = {token, 0, 0, 0, 0};
        const char is_image = s_i8_ld_g(gen_addr(image_at, image_mask));
        float64 scores[6], choices[6];
        float64 packed_weights = 0;
        int64 packed_ids = 0;
        bool64 eligible[6];
        #pragma unroll
        for (int c = 0; c < 6; ++c) {
            const int5 at = {c * 64, token, 0, 0, 0};
            const int5 bias_at = {c * 64, 0, 0, 0, 0};
            scores[c] = v_f32_ld_tnsr_b(at, input);
            const float64 bias = is_image ? v_f32_ld_tnsr_b(bias_at, image_bias) : v_f32_ld_tnsr_b(bias_at, text_bias);
            choices[c] = scores[c] + bias;
            eligible[c] = (bool64)1;
        }
        float64 sum = 0;
        #pragma unroll
        for (int rank = 0; rank < 6; ++rank) {
            // Reduce each lane's six experts together with its expert ID.
            // Descending chunk order plus >= gives the smaller ID on ties;
            // predication excludes an already-selected -Inf expert as well.
            int64_float64_pair_t local;
            local.v1 = 2147483647;
            local.v2 = as_float64((int64)0xff800000);
            float64 local_score = 0;
            #pragma unroll
            for (int c = 5; c >= 0; --c) {
                local_score = v_f32_sel_geq_f32_vb(choices[c], local.v2, scores[c], local_score,
                                                   0, local_score, eligible[c], 0);
                local = v_i32_sel2_geq_f32_vb(choices[c], local.v2, lanes + c * 64, local.v1,
                                              0, local, eligible[c], 0);
            }
            const float64 best = v_f32_reduce_max(local.v2);
            int64 winner = v_i32_sel_eq_f32_b(local.v2, best, local.v1, 2147483647);
            winner = v_i32_reduce_min(winner);
            const bool64 selected_lane = v_i32_cmp_eq_b(local.v1, winner);
            const float64 selected = v_f32_mov_vb(local_score, 0, 0, selected_lane, 0);
            #pragma unroll
            for (int c = 0; c < 6; ++c)
                eligible[c] = eligible[c] & v_i32_cmp_neq_b(lanes + c * 64, winner);
            const float64 selected_weight = v_f32_reduce_add(selected);
            const bool64 output_lane = v_i32_cmp_eq_b(lanes, rank);
            packed_weights = v_f32_mov_vb(selected_weight, 0, packed_weights, output_lane, 0);
            packed_ids = v_i32_mov_vb(winner, 0, packed_ids, output_lane, 0);
            sum += selected_weight;
        }
        const float64 denominator = sum + 1.0e-20f;
        float64 inverse = v_reciprocal_f32(denominator);
        inverse = inverse * (2.0f - denominator * inverse);
        const int5 at = {0, token, 0, 0, 0};
        v_i32_st_tnsr_partial(at, output_ids, packed_ids, 5, 0);
        v_f32_st_tnsr_partial(at, output_weights, packed_weights * inverse * 1.5f, 5, 0);
    }
}
