// Qwen3.8-27B GDN post-convolution Q/K preparation for Gaudi2.
//
// Keep the value slice out of this TPC kernel. Synapse can copy value through
// DMA while TPCs normalize Q/K, overlapping memory-only and compute work.
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

    // BF16->FP32 conversion places even and odd source elements in separate
    // registers. Build the inverse maps once, then interleave with two
    // predicated SHUFFLEs for each contiguous 64-element output half.
    const uint64 lane_ids = (uint64)read_lane_id_4b_b();
    const uint64 source_lanes = lane_ids >> 1;
    const uchar256 low_interleave_map =
        (uchar256)(0x80808080 + source_lanes * 0x01010101);
    const uchar256 high_interleave_map =
        (uchar256)(0xa0a0a0a0 + source_lanes * 0x01010101);
    const bool64 odd_lanes = v_u32_cmp_eq_b(lane_ids & 1, 1);

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

            // ACC_F32 squares BF16 inputs without rounding the products and
            // avoids issuing two separate FP32 vector multiplies per head.
            const float64_pair_t q_square =
                v_bf16_mul_acc32_b(q_bf16, q_bf16);
            float64 q_norm = q_square.v1 + q_square.v2;
            q_norm = v_f32_reduce_add(q_norm);
            const float64_pair_t k_square =
                v_bf16_mul_acc32_b(k_bf16, k_bf16);
            q_norm = v_f32_shuffle_b(
                q_norm,
                broadcast_lane_zero,
                0,
                q_norm);
            q_norm = v_rsqrt_fast_f32(q_norm + 1e-6f);

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
            float64 q_low = v_f32_shuffle_b(
                q_normalized.v1, low_interleave_map, 0, 0);
            q_low = v_f32_shuffle_vb(
                q_normalized.v2,
                low_interleave_map,
                0,
                q_low,
                odd_lanes);
            float64 q_high = v_f32_shuffle_b(
                q_normalized.v1, high_interleave_map, 0, 0);
            q_high = v_f32_shuffle_vb(
                q_normalized.v2,
                high_interleave_map,
                0,
                q_high,
                odd_lanes);
            float64 k_low = v_f32_shuffle_b(
                k_normalized.v1, low_interleave_map, 0, 0);
            k_low = v_f32_shuffle_vb(
                k_normalized.v2,
                low_interleave_map,
                0,
                k_low,
                odd_lanes);
            float64 k_high = v_f32_shuffle_b(
                k_normalized.v1, high_interleave_map, 0, 0);
            k_high = v_f32_shuffle_vb(
                k_normalized.v2,
                high_interleave_map,
                0,
                k_high,
                odd_lanes);

            // SHUFFLE selects within 16-lane dual groups, yielding group order
            // [0, 2, 4, 6] and [1, 3, 5, 7]. Interleave those groups into the
            // two contiguous output halves with load-slot MOV instructions.
            float64 q_contiguous_low = v_f32_mov_dual_group_all_b(
                q_low,
                0xffffffff,
                0,
                0,
                1,
                0,
                MkWrA(3, 0, 3, 0),
                0);
            q_contiguous_low = v_f32_mov_dual_group_all_b(
                q_high,
                0xffffffff,
                0,
                0,
                0,
                1,
                MkWrA(0, 3, 0, 3),
                q_contiguous_low);
            float64 q_contiguous_high = v_f32_mov_dual_group_all_b(
                q_low,
                0xffffffff,
                2,
                0,
                3,
                0,
                MkWrA(3, 0, 3, 0),
                0);
            q_contiguous_high = v_f32_mov_dual_group_all_b(
                q_high,
                0xffffffff,
                0,
                2,
                0,
                3,
                MkWrA(0, 3, 0, 3),
                q_contiguous_high);
            float64 k_contiguous_low = v_f32_mov_dual_group_all_b(
                k_low,
                0xffffffff,
                0,
                0,
                1,
                0,
                MkWrA(3, 0, 3, 0),
                0);
            k_contiguous_low = v_f32_mov_dual_group_all_b(
                k_high,
                0xffffffff,
                0,
                0,
                0,
                1,
                MkWrA(0, 3, 0, 3),
                k_contiguous_low);
            float64 k_contiguous_high = v_f32_mov_dual_group_all_b(
                k_low,
                0xffffffff,
                2,
                0,
                3,
                0,
                MkWrA(3, 0, 3, 0),
                0);
            k_contiguous_high = v_f32_mov_dual_group_all_b(
                k_high,
                0xffffffff,
                0,
                2,
                0,
                3,
                MkWrA(0, 3, 0, 3),
                k_contiguous_high);

            const int output_head = head * 3;
            output_coords[1] = output_head;
            output_coords[0] = 0;
            v_f32_st_tnsr(output_coords, q_out, q_contiguous_low);
            v_f32_st_tnsr(output_coords, k_out, k_contiguous_low);
            output_coords[0] = 64;
            v_f32_st_tnsr(output_coords, q_out, q_contiguous_high);
            v_f32_st_tnsr(output_coords, k_out, k_contiguous_high);
            output_coords[1] = output_head + 1;
            output_coords[0] = 0;
            v_f32_st_tnsr(output_coords, q_out, q_contiguous_low);
            v_f32_st_tnsr(output_coords, k_out, k_contiguous_low);
            output_coords[0] = 64;
            v_f32_st_tnsr(output_coords, q_out, q_contiguous_high);
            v_f32_st_tnsr(output_coords, k_out, k_contiguous_high);
            output_coords[1] = output_head + 2;
            output_coords[0] = 0;
            v_f32_st_tnsr(output_coords, q_out, q_contiguous_low);
            v_f32_st_tnsr(output_coords, k_out, k_contiguous_low);
            output_coords[0] = 64;
            v_f32_st_tnsr(output_coords, q_out, q_contiguous_high);
            v_f32_st_tnsr(output_coords, k_out, k_contiguous_high);
        }
    }
}
