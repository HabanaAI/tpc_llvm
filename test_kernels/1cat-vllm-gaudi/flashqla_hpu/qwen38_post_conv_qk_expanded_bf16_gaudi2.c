// Qwen3.8-27B GDN expanded BF16 post-convolution Q/K preparation.
//
// Preserve the 48-head layout selected by Gaudi2's batched GEMMs, but cast
// immediately after FP32 normalization so the large repeated output is never
// materialized as FP32.
void main(tensor packed_qkv, tensor q_out, tensor k_out)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const uchar256 broadcast_lane_zero = 0x80;
    const int qk_heads = 16;
    const int head_dim = 128;
    const int key_offset = qk_heads * head_dim;

    int5 packed_coords = {0, 0, 0, 0, 0};
    int5 output_coords = {0, 0, 0, 0, 0};

    for (int token = start[0]; token < end[0]; ++token)
    {
        packed_coords[1] = token;
        output_coords[2] = token;

        #pragma loop_unroll(4) pipelined taken
        for (int head = 0; head < qk_heads; ++head)
        {
            packed_coords[0] = head * head_dim;
            const bfloat128 q_bf16 =
                v_bf16_ld_tnsr_b(packed_coords, packed_qkv);
            packed_coords[0] = key_offset + head * head_dim;
            const bfloat128 k_bf16 =
                v_bf16_ld_tnsr_b(packed_coords, packed_qkv);

            const float64_pair_t q_square =
                v_bf16_mul_acc32_b(q_bf16, q_bf16);
            float64 q_norm = q_square.v1 + q_square.v2;
            q_norm = v_f32_reduce_add(q_norm);
            q_norm = v_f32_shuffle_b(
                q_norm,
                broadcast_lane_zero,
                0,
                q_norm);
            q_norm = v_rsqrt_fast_f32(q_norm + 1e-6f);

            const float64_pair_t k_square =
                v_bf16_mul_acc32_b(k_bf16, k_bf16);
            float64 k_norm = k_square.v1 + k_square.v2;
            k_norm = v_f32_reduce_add(k_norm);
            k_norm = v_f32_shuffle_b(
                k_norm,
                broadcast_lane_zero,
                0,
                k_norm);
            k_norm = v_rsqrt_fast_f32(k_norm + 1e-6f);

            const float64_pair_t q_f32 =
                v_convert_bf16_to_f32_all_b(q_bf16);
            const float64_pair_t k_f32 =
                v_convert_bf16_to_f32_all_b(k_bf16);
            float64_pair_t q_normalized;
            q_normalized.v1 = q_f32.v1 * q_norm;
            q_normalized.v2 = q_f32.v2 * q_norm;
            float64_pair_t k_normalized;
            k_normalized.v1 = k_f32.v1 * k_norm;
            k_normalized.v2 = k_f32.v2 * k_norm;

            const bfloat128 q_result =
                v_convert_f32_to_bf16_all_b(q_normalized);
            const bfloat128 k_result =
                v_convert_f32_to_bf16_all_b(k_normalized);
            output_coords[0] = 0;
            const int output_head = head * 3;
            output_coords[1] = output_head;
            v_bf16_st_tnsr(output_coords, q_out, q_result);
            v_bf16_st_tnsr(output_coords, k_out, k_result);
            output_coords[1] = output_head + 1;
            v_bf16_st_tnsr(output_coords, q_out, q_result);
            v_bf16_st_tnsr(output_coords, k_out, k_result);
            output_coords[1] = output_head + 2;
            v_bf16_st_tnsr(output_coords, q_out, q_result);
            v_bf16_st_tnsr(output_coords, k_out, k_result);
        }
    }
}
