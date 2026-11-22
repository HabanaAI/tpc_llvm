/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

#define DSV4_MHC_HIDDEN_SIZE 4096
#define DSV4_MHC_STREAMS 4
#define DSV4_MHC_BF16_VECTOR_WIDTH 128
#define DSV4_MHC_VECTOR_COUNT \
    (DSV4_MHC_HIDDEN_SIZE / DSV4_MHC_BF16_VECTOR_WIDTH)
#define DSV4_MHC_EPS 1.0e-6f
#define DSV4_MHC_POST_ALPHA 2.0f
#define DSV4_MHC_SINKHORN_ITERS 20

static inline float64 dsv4_mhc_exp(float64 value)
{
    return v_exp_cephes_f32(value);
}

static inline float64 dsv4_mhc_sigmoid(float64 value)
{
    return v_reciprocal_f32(1.0f + v_exp_cephes_f32(-value));
}

// Consumes the native MME gate projection, performs all gate activations and
// Sinkhorn iterations locally, and emits the next layer input in one TPC op.
void main(
    tensor residual,
    tensor raw_mixes,
    tensor rrms,
    tensor hc_scale,
    tensor hc_base,
    tensor post_mix_out,
    tensor comb_mix_out,
    tensor layer_input_out)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;

    for (int token = index_start[0]; token < index_end[0]; ++token) {
        int5 rrms_coords = {0, token, 0, 0, 0};
        const float rrms_value = s_f32_ld_g(gen_addr(rrms_coords, rrms));
        float scales[3];
        for (int group = 0; group < 3; ++group) {
            int5 scale_coords = {group, 0, 0, 0, 0};
            scales[group] = s_f32_ld_g(gen_addr(scale_coords, hc_scale));
        }

        float64 pre_mix[DSV4_MHC_STREAMS];
        for (int stream = 0; stream < DSV4_MHC_STREAMS; ++stream) {
            int5 pre_mix_coords = {stream, token, 0, 0, 0};
            int5 post_mix_coords = {
                DSV4_MHC_STREAMS + stream, token, 0, 0, 0};
            int5 pre_base_coords = {stream, 0, 0, 0, 0};
            int5 post_base_coords = {
                DSV4_MHC_STREAMS + stream, 0, 0, 0, 0};
            float64 pre_logit =
                s_f32_ld_g(gen_addr(pre_mix_coords, raw_mixes));
            pre_logit =
                pre_logit * rrms_value * scales[0]
                + s_f32_ld_g(gen_addr(pre_base_coords, hc_base));
            float64 post_logit =
                s_f32_ld_g(gen_addr(post_mix_coords, raw_mixes));
            post_logit =
                post_logit * rrms_value * scales[1]
                + s_f32_ld_g(gen_addr(post_base_coords, hc_base));
            pre_mix[stream] = dsv4_mhc_sigmoid(pre_logit) + DSV4_MHC_EPS;
            const float64 post_mix =
                dsv4_mhc_sigmoid(post_logit) * DSV4_MHC_POST_ALPHA;
            int5 post_output_coords = {0, stream, token, 0, 0};
            v_f32_st_tnsr_partial(
                post_output_coords, post_mix_out, post_mix, 0, 0);
        }

        float64 comb_mix[DSV4_MHC_STREAMS][DSV4_MHC_STREAMS];
        for (int source = 0; source < DSV4_MHC_STREAMS; ++source) {
            float64 row_max = -3.402823466e+38f;
            for (int target = 0; target < DSV4_MHC_STREAMS; ++target) {
                const int index =
                    2 * DSV4_MHC_STREAMS
                    + source * DSV4_MHC_STREAMS + target;
                int5 mix_coords = {index, token, 0, 0, 0};
                int5 base_coords = {index, 0, 0, 0, 0};
                const float64 value =
                    s_f32_ld_g(gen_addr(mix_coords, raw_mixes))
                    * rrms_value * scales[2]
                    + s_f32_ld_g(gen_addr(base_coords, hc_base));
                comb_mix[source][target] = value;
                row_max = v_f32_max_b(value, row_max);
            }
            float64 row_sum = 0.0f;
            for (int target = 0; target < DSV4_MHC_STREAMS; ++target) {
                const float64 value = dsv4_mhc_exp(
                    comb_mix[source][target] - row_max);
                comb_mix[source][target] = value;
                row_sum += value;
            }
            const float64 inverse_row_sum = v_reciprocal_f32(row_sum);
            for (int target = 0; target < DSV4_MHC_STREAMS; ++target) {
                comb_mix[source][target] =
                    comb_mix[source][target] * inverse_row_sum + DSV4_MHC_EPS;
            }
        }

        for (int target = 0; target < DSV4_MHC_STREAMS; ++target) {
            float64 column_sum = DSV4_MHC_EPS;
            for (int source = 0; source < DSV4_MHC_STREAMS; ++source) {
                column_sum += comb_mix[source][target];
            }
            const float64 inverse_column_sum =
                v_reciprocal_f32(column_sum);
            for (int source = 0; source < DSV4_MHC_STREAMS; ++source) {
                comb_mix[source][target] *= inverse_column_sum;
            }
        }
        for (int iteration = 1;
             iteration < DSV4_MHC_SINKHORN_ITERS;
             ++iteration) {
            for (int source = 0; source < DSV4_MHC_STREAMS; ++source) {
                float64 row_sum = DSV4_MHC_EPS;
                for (int target = 0; target < DSV4_MHC_STREAMS; ++target) {
                    row_sum += comb_mix[source][target];
                }
                const float64 inverse_row_sum =
                    v_reciprocal_f32(row_sum);
                for (int target = 0; target < DSV4_MHC_STREAMS; ++target) {
                    comb_mix[source][target] *= inverse_row_sum;
                }
            }
            for (int target = 0; target < DSV4_MHC_STREAMS; ++target) {
                float64 column_sum = DSV4_MHC_EPS;
                for (int source = 0; source < DSV4_MHC_STREAMS; ++source) {
                    column_sum += comb_mix[source][target];
                }
                const float64 inverse_column_sum =
                    v_reciprocal_f32(column_sum);
                for (int source = 0; source < DSV4_MHC_STREAMS; ++source) {
                    comb_mix[source][target] *= inverse_column_sum;
                }
            }
        }

        for (int source = 0; source < DSV4_MHC_STREAMS; ++source) {
            for (int target = 0; target < DSV4_MHC_STREAMS; ++target) {
                const int flat_index =
                    source * DSV4_MHC_STREAMS + target;
                int5 output_coords = {0, flat_index, token, 0, 0};
                v_f32_st_tnsr_partial(
                    output_coords,
                    comb_mix_out,
                    comb_mix[source][target],
                    0,
                    0);
            }
        }

        for (int chunk = 0; chunk < DSV4_MHC_VECTOR_COUNT; ++chunk) {
            const int feature = chunk * DSV4_MHC_BF16_VECTOR_WIDTH;
            float128 layer_input = {0};
            for (int stream = 0; stream < DSV4_MHC_STREAMS; ++stream) {
                int5 residual_coords = {feature, stream, token, 0, 0};
                const float128 values = v_convert_bf16_to_f32_all_b(
                    v_bf16_ld_tnsr_b(residual_coords, residual));
                layer_input.v1 = v_f32_mac_b(
                    values.v1, pre_mix[stream], layer_input.v1);
                layer_input.v2 = v_f32_mac_b(
                    values.v2, pre_mix[stream], layer_input.v2);
            }
            const bfloat128 rounded =
                v_convert_f32_to_bf16_all_b(layer_input, SW_RHNE);
            int5 output_coords = {feature, token, 0, 0, 0};
            v_bf16_st_tnsr(output_coords, layer_input_out, rounded);
        }
    }
}
