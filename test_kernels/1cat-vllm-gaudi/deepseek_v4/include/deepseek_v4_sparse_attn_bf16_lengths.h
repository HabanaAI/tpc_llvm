/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

#define DSV4_HEAD_DIM 512
#define DSV4_BF16_VECTOR_WIDTH 128
#define DSV4_VECTOR_COUNT (DSV4_HEAD_DIM / DSV4_BF16_VECTOR_WIDTH)

#ifdef DSV41_PACKED_ATTN_EXP
// Both inputs are broadcasts. Compute the same Cephes function once with
// each argument in alternating lanes, then broadcast the two results within
// each hardware shuffle group (shuffle cannot cross arbitrary vector halves).
// Unlike exp(-abs(a-b)), this also preserves non-finite input behavior.
static inline float128 packed_exp_pair(float64 first, float64 second)
{
    const uint64 parity = v_u32_and_b(read_lane_id_4b_b(), 1);
    const bool64 odd = v_u32_cmp_eq_b(parity, 1);
    const float64 packed = v_f32_mov_vb(second, 0, first, odd);
    const float64 result = v_exp_cephes_f32(packed);
    float128 pair;
    pair.v1 = v_f32_shuffle_b(result, (uchar256)0x80, 0, result);
    pair.v2 = v_f32_shuffle_b(result, (uchar256)0x81, 0, result);
    return pair;
}
#endif

// FlashMLA-style sparse prefill. The caller supplies the valid prefix length
// for each row so aligned index padding is never scanned by the TPC program.
void main(
    tensor q,
    tensor kv,
#ifdef DSV41_DECODED_KV_READ
    tensor main_cache,
#endif
    tensor indices,
    tensor attn_sink,
    tensor softmax_scale,
    tensor topk_lengths,
#ifdef DSV41_DECODED_KV_READ
    tensor swa_completion,
    tensor main_completion,
#endif
    tensor output,
    tensor max_logits,
    tensor softmax_lse
#ifdef DSV41_DECODED_KV_READ
    , int swa_offset, int main_rows
#endif
    )
{
    const uchar256 broadcast_lane_zero = 0x80;
    const int5 index_start = get_index_space_offset();
    const int5 index_end = get_index_space_size() + index_start;
#ifdef DSV41_DECODED_KV_READ
    const int sequence_length = 512 + main_rows;
    const bool ready = s_i32_ld_g(gen_addr((int5){0}, swa_completion)) >= 0 &&
                       s_i32_ld_g(gen_addr((int5){0}, main_completion)) >= 0;
#else
    const int sequence_length = get_dim_size(kv, 1);
#endif
    const int topk_width = get_dim_size(indices, 0);

    int5 scale_coords = {0, 0, 0, 0, 0};
    const float scale_value =
        s_f32_ld_g(gen_addr(scale_coords, softmax_scale));

    for (int batch = index_start[2]; batch < index_end[2]; ++batch) {
        int5 length_coords = {batch, 0, 0, 0, 0};
        int valid_width =
            s_i32_ld_g(gen_addr(length_coords, topk_lengths));
#ifdef DSV41_DECODED_KV_READ
        if (!ready) valid_width = 0;
#endif
        if (valid_width < 0) {
            valid_width = 0;
        }
        if (valid_width > topk_width) {
            valid_width = topk_width;
        }

        for (int head = index_start[1]; head < index_end[1]; ++head) {
            bfloat128 q_vectors[DSV4_VECTOR_COUNT];
            float128 output_accum[DSV4_VECTOR_COUNT] = {0};

            #pragma unroll (DSV4_VECTOR_COUNT)
            for (int chunk = 0; chunk < DSV4_VECTOR_COUNT; ++chunk) {
                int5 q_coords = {
                    chunk * DSV4_BF16_VECTOR_WIDTH, head, batch, 0, 0};
                q_vectors[chunk] = v_bf16_ld_tnsr_b(q_coords, q);
            }

            float64 running_max = -3.402823466e+38f;
            float64 running_sum = 0.0f;

            for (int position = 0; position < valid_width; ++position) {
                int5 index_coords = {position, batch, 0, 0, 0};
                const int kv_index =
                    s_i32_ld_g(gen_addr(index_coords, indices));
                if (kv_index < 0 || kv_index >= sequence_length) {
                    continue;
                }

                bfloat128 kv_vectors[DSV4_VECTOR_COUNT];
                float128 dot_accum = {0};
                #pragma unroll (DSV4_VECTOR_COUNT)
                for (int chunk = 0; chunk < DSV4_VECTOR_COUNT; ++chunk) {
                    int5 kv_coords = {
                        chunk * DSV4_BF16_VECTOR_WIDTH, kv_index, 0, 0, 0};
#ifdef DSV41_DECODED_KV_READ
                    if (kv_index < 512) {
                        kv_coords[1] = swa_offset + kv_index;
                        kv_vectors[chunk] = v_bf16_ld_tnsr_b(kv_coords, kv);
                    } else {
                        kv_coords[1] = kv_index - 512;
                        kv_vectors[chunk] = v_bf16_ld_tnsr_b(kv_coords, main_cache);
                    }
#else
                    kv_vectors[chunk] = v_bf16_ld_tnsr_b(kv_coords, kv);
#endif
                    dot_accum = v_bf16_mac_acc32_b(
                        q_vectors[chunk], kv_vectors[chunk], dot_accum,
                        (e_no_negation) << 1);
                }

                float64 dot_lanes = dot_accum.v1 + dot_accum.v2;
                dot_lanes = v_f32_reduce_add(dot_lanes);
                float64 score = v_f32_shuffle_b(
                    dot_lanes, broadcast_lane_zero, 0, dot_lanes);
                score *= scale_value;

                const float64 next_max = v_f32_max_b(running_max, score);
#ifdef DSV41_PACKED_ATTN_EXP
                const float128 exp_pair = packed_exp_pair(
                    running_max - next_max, score - next_max);
                const float64 previous_scale = exp_pair.v1;
                const float64 weight = exp_pair.v2;
#else
                const float64 previous_scale =
                    v_exp_cephes_f32(running_max - next_max);
                const float64 weight = v_exp_cephes_f32(score - next_max);
#endif
                running_sum = running_sum * previous_scale + weight;

                #pragma unroll (DSV4_VECTOR_COUNT)
                for (int chunk = 0; chunk < DSV4_VECTOR_COUNT; ++chunk) {
                    const float128 kv_f32 =
                        v_convert_bf16_to_f32_all_b(kv_vectors[chunk]);
                    output_accum[chunk].v1 = v_f32_mac_b(
                        kv_f32.v1,
                        weight,
                        output_accum[chunk].v1 * previous_scale);
                    output_accum[chunk].v2 = v_f32_mac_b(
                        kv_f32.v2,
                        weight,
                        output_accum[chunk].v2 * previous_scale);
                }
                running_max = next_max;
            }

            int5 sink_coords = {head, 0, 0, 0, 0};
            const float sink_value =
                s_f32_ld_g(gen_addr(sink_coords, attn_sink));
            const float64 sink_score = sink_value;
            const float64 final_max = v_f32_max_b(running_max, sink_score);
#ifdef DSV41_PACKED_ATTN_EXP
            const float128 final_exp = packed_exp_pair(
                running_max - final_max, sink_score - final_max);
            const float64 data_scale = final_exp.v1;
            const float64 sink_weight = final_exp.v2;
#else
            const float64 data_scale =
                v_exp_cephes_f32(running_max - final_max);
            const float64 sink_weight =
                v_exp_cephes_f32(sink_score - final_max);
#endif
            running_sum = running_sum * data_scale + sink_weight;
            const float64 inverse_sum = v_reciprocal_f32(running_sum);
            const float64 lse = final_max + v_log_f32(running_sum);

            #pragma unroll (DSV4_VECTOR_COUNT)
            for (int chunk = 0; chunk < DSV4_VECTOR_COUNT; ++chunk) {
                output_accum[chunk].v1 *= data_scale * inverse_sum;
                output_accum[chunk].v2 *= data_scale * inverse_sum;
                const bfloat128 result =
                    v_convert_f32_to_bf16_all_b(output_accum[chunk]);
                int5 output_coords = {
                    chunk * DSV4_BF16_VECTOR_WIDTH, head, batch, 0, 0};
                v_bf16_st_tnsr(output_coords, output, result);
            }

            int5 stats_coords = {head, batch, 0, 0, 0};
            v_f32_st_tnsr_partial(
                stats_coords, max_logits, final_max, 0, 0);
            v_f32_st_tnsr_partial(
                stats_coords, softmax_lse, lse, 0, 0);
        }
    }
}
