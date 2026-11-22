/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

// Combines FlashMLA split-KV FP32 states. Each split stores an online
// softmax state in its own local-max basis: (m_i, l_i, o_i). The sink is an
// additional zero-valued state, so the final normalization is exact in FP32.
void main(
    tensor partial_output,
    tensor partial_stats,
    tensor attn_sink,
    tensor output,
    tensor combine_stats)
{
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
    const int split_count = get_dim_size(partial_output, 2);

    for (int batch = index_start[1]; batch < index_end[1]; ++batch) {
        for (int head = index_start[0]; head < index_end[0]; ++head) {
            int5 sink_coords = {head, 0, 0, 0, 0};
            const float sink_value =
                s_f32_ld_g(gen_addr(sink_coords, attn_sink));
            float global_max = sink_value;
            for (int split = 0; split < split_count; ++split) {
                int5 stats_coords = {0, head, split, batch, 0};
                const float partial_max = s_f32_ld_g(
                    gen_addr(stats_coords, partial_stats));
                global_max = partial_max > global_max
                    ? partial_max
                    : global_max;
            }

            float128 combined[4] = {0};
            float64 total_sum =
                v_exp_cephes_f32(sink_value - global_max);
            for (int split = 0; split < split_count; ++split) {
                int5 stats_coords = {0, head, split, batch, 0};
                const float partial_max = s_f32_ld_g(
                    gen_addr(stats_coords, partial_stats));
                stats_coords[0] = 1;
                const float partial_sum = s_f32_ld_g(
                    gen_addr(stats_coords, partial_stats));
                if (partial_sum <= 0.0f) {
                    continue;
                }
                const float64 split_scale =
                    v_exp_cephes_f32(partial_max - global_max);
                total_sum += split_scale * partial_sum;
                #pragma unroll (4)
                for (int chunk = 0; chunk < 4; ++chunk) {
                    int5 partial_coords = {
                        chunk * 128, head, split, batch, 0};
                    float128 partial;
                    partial.v1 = v_f32_ld_tnsr_b(
                        partial_coords, partial_output);
                    partial_coords[0] += 64;
                    partial.v2 = v_f32_ld_tnsr_b(
                        partial_coords, partial_output);
                    combined[chunk].v1 = v_f32_mac_b(
                        partial.v1,
                        split_scale,
                        combined[chunk].v1);
                    combined[chunk].v2 = v_f32_mac_b(
                        partial.v2,
                        split_scale,
                        combined[chunk].v2);
                }
            }

            const float64 inverse_sum = v_reciprocal_f32(total_sum);
            #pragma unroll (4)
            for (int chunk = 0; chunk < 4; ++chunk) {
                combined[chunk].v1 *= inverse_sum;
                combined[chunk].v2 *= inverse_sum;
                const bfloat128 result = v_convert_f32_to_bf16_all_b(
                    combined[chunk], SW_RHNE);
                int5 output_coords = {
                    chunk * 128, head, batch, 0, 0};
                v_bf16_st_tnsr(output_coords, output, result);
            }
            int5 result_stats_coords = {head, batch, 0, 0, 0};
            v_f32_st_tnsr_partial(
                result_stats_coords,
                combine_stats,
                total_sum,
                0,
                0);
        }
    }
}
