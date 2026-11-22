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

static inline float64 dsv4_mhc_broadcast_lane_zero(float64 value)
{
    const uchar256 broadcast_lane_zero = 0x80;
    return v_f32_shuffle_b(
        value, broadcast_lane_zero, 0, value);
}

// Fuses MHC post, the BF16 semantic cast, FP32 materialization for the MME
// gate projection, and RMS scale reduction. One index-space point owns one
// token; the expensive 24x16384 projection intentionally remains on MME.
void main(
    tensor x,
    tensor residual,
    tensor post_layer_mix,
    tensor comb_res_mix,
    tensor residual_out,
    tensor residual_f32_out,
    tensor rrms_out)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;

    for (int token = index_start[0]; token < index_end[0]; ++token) {
        float post_weights[DSV4_MHC_STREAMS];
        float comb_weights[DSV4_MHC_STREAMS][DSV4_MHC_STREAMS];
        for (int target = 0; target < DSV4_MHC_STREAMS; ++target) {
            int5 post_coords = {0, target, token, 0, 0};
            post_weights[target] =
                s_f32_ld_g(gen_addr(post_coords, post_layer_mix));
            for (int source = 0; source < DSV4_MHC_STREAMS; ++source) {
                int5 comb_coords = {target, source, token, 0, 0};
                comb_weights[source][target] =
                    s_f32_ld_g(gen_addr(comb_coords, comb_res_mix));
            }
        }

        float64 sumsq_lanes = 0.0f;
        for (int chunk = 0; chunk < DSV4_MHC_VECTOR_COUNT; ++chunk) {
            const int feature = chunk * DSV4_MHC_BF16_VECTOR_WIDTH;
            int5 x_coords = {feature, token, 0, 0, 0};
            const bfloat128 x_bf16 = v_bf16_ld_tnsr_b(x_coords, x);
            const float128 x_f32 = v_convert_bf16_to_f32_all_b(x_bf16);

            float128 residual_f32[DSV4_MHC_STREAMS];
            for (int source = 0; source < DSV4_MHC_STREAMS; ++source) {
                int5 residual_coords = {feature, source, token, 0, 0};
                residual_f32[source] = v_convert_bf16_to_f32_all_b(
                    v_bf16_ld_tnsr_b(residual_coords, residual));
            }

            for (int target = 0; target < DSV4_MHC_STREAMS; ++target) {
                float128 updated = {0};
                updated.v1 = x_f32.v1 * post_weights[target];
                updated.v2 = x_f32.v2 * post_weights[target];
                for (int source = 0; source < DSV4_MHC_STREAMS; ++source) {
                    updated.v1 = v_f32_mac_b(
                        residual_f32[source].v1,
                        comb_weights[source][target],
                        updated.v1);
                    updated.v2 = v_f32_mac_b(
                        residual_f32[source].v2,
                        comb_weights[source][target],
                        updated.v2);
                }

                const bfloat128 rounded =
                    v_convert_f32_to_bf16_all_b(updated, SW_RHNE);
                int5 residual_out_coords = {
                    feature, target, token, 0, 0};
                v_bf16_st_tnsr(
                    residual_out_coords, residual_out, rounded);

                const float128 rounded_f32 =
                    convert_bfloat128_to_float128(rounded, SW_LINEAR);
                const int flat_feature =
                    target * DSV4_MHC_HIDDEN_SIZE + feature;
                int5 flat_coords = {flat_feature, token, 0, 0, 0};
                v_f32_st_tnsr(
                    flat_coords, residual_f32_out, rounded_f32.v1);
                flat_coords[0] += 64;
                v_f32_st_tnsr(
                    flat_coords, residual_f32_out, rounded_f32.v2);

                sumsq_lanes = v_f32_mac_b(
                    rounded_f32.v1, rounded_f32.v1, sumsq_lanes);
                sumsq_lanes = v_f32_mac_b(
                    rounded_f32.v2, rounded_f32.v2, sumsq_lanes);
            }
        }

        const float64 reduced_sumsq = v_f32_reduce_add(sumsq_lanes);
        const float64 rrms = v_rsqrt_f32(
            dsv4_mhc_broadcast_lane_zero(reduced_sumsq)
            / (float) (DSV4_MHC_STREAMS * DSV4_MHC_HIDDEN_SIZE)
            + DSV4_MHC_EPS);
        int5 rrms_coords = {0, token, 0, 0, 0};
        v_f32_st_tnsr_partial(rrms_coords, rrms_out, rrms, 0, 0);
    }
}
