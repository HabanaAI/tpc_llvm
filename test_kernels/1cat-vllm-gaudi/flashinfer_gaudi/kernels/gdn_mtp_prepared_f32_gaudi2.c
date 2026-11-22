// SPDX-License-Identifier: Apache-2.0
// Full-query MTP core. Q is normalized/scaled, K normalized, and V FP32.
// The caller resolves the accepted state before this pure, two-output op.
#ifndef MTP_PREPARED_ROWS
#define MTP_PREPARED_ROWS 4
#endif

void main(tensor initial_state, tensor prepared_qkv, tensor decay, tensor beta,
          tensor output, tensor checkpoints)
{
    const int rows = MTP_PREPARED_ROWS;
    const uchar256 broadcast_lane_zero = 0x80;
    const uint64 lanes = read_lane_id_4b_b();
    const int5 begin = get_index_space_offset();
    const int5 end = get_index_space_size() + begin;

    for (int batch = begin[3]; batch < end[3]; ++batch) {
        for (int head = begin[2]; head < end[2]; ++head) {
            const int key_head = head / 3;
            for (int tile = begin[1]; tile < end[1]; ++tile) {
                float64 state_lo[MTP_PREPARED_ROWS];
                float64 state_hi[MTP_PREPARED_ROWS];
                #pragma unroll
                for (int row = 0; row < rows; ++row) {
                    int5 coords = {0, tile * rows + row, head, batch, 0};
                    state_lo[row] = v_f32_ld_tnsr_b(coords, initial_state);
                    coords[0] = 64;
                    state_hi[row] = v_f32_ld_tnsr_b(coords, initial_state);
                }
                for (int token = 0; token < 8; ++token) {
                    int5 packed_coords = {key_head * 128, token, batch, 0, 0};
                    const float64 q_lo = v_f32_ld_tnsr_b(packed_coords, prepared_qkv);
                    packed_coords[0] += 64;
                    const float64 q_hi = v_f32_ld_tnsr_b(packed_coords, prepared_qkv);
                    packed_coords[0] = 2048 + key_head * 128;
                    const float64 k_lo = v_f32_ld_tnsr_b(packed_coords, prepared_qkv);
                    packed_coords[0] += 64;
                    const float64 k_hi = v_f32_ld_tnsr_b(packed_coords, prepared_qkv);
                    int5 scalar_coords = {head, token, batch, 0, 0};
                    const float64 decay_values = v_f32_ld_tnsr_b(scalar_coords, decay);
                    const float64 decay_value = v_broadcast_element_f32(decay_values, 0);
                    const float64 beta_values = v_f32_ld_tnsr_b(scalar_coords, beta);
                    const float64 beta_value = v_broadcast_element_f32(beta_values, 0);
                    packed_coords[0] = 4096 + head * 128 + tile * rows;
                    const float64 values = v_f32_ld_tnsr_b(packed_coords, prepared_qkv);
                    float64 packed_output = 0.0f;

                    #pragma unroll
                    for (int row = 0; row < rows; ++row) {
                        const float64 value = v_broadcast_element_f32(values, row);
                        state_lo[row] *= decay_value;
                        state_hi[row] *= decay_value;
                        float64 projection = state_lo[row] * k_lo;
                        projection = v_f32_mac_b(state_hi[row], k_hi, projection);
                        projection = v_f32_reduce_add(projection);
                        projection = v_f32_shuffle_b(projection, broadcast_lane_zero, 0, projection);
                        const float64 delta = (value - projection) * beta_value;
                        state_lo[row] = v_f32_mac_b(k_lo, delta, state_lo[row]);
                        state_hi[row] = v_f32_mac_b(k_hi, delta, state_hi[row]);
                        float64 out_value = state_lo[row] * q_lo;
                        out_value = v_f32_mac_b(state_hi[row], q_hi, out_value);
                        out_value = v_f32_reduce_add(out_value);
                        out_value = v_broadcast_element_f32(out_value, 0);
                        const bool64 row_lane = v_u32_cmp_eq_b(lanes, (uint64)row);
                        packed_output = v_f32_mov_vb(out_value, 0, packed_output, row_lane);

                        int5 coords = {0, tile * rows + row, head, token, batch};
                        v_f32_st_tnsr(coords, checkpoints, state_lo[row]);
                        coords[0] = 64;
                        v_f32_st_tnsr(coords, checkpoints, state_hi[row]);
                    }
                    int5 out_coords = {tile * rows, head, token, batch, 0};
                    v_f32_st_tnsr_partial(out_coords, output, packed_output, rows - 1, 0);
                }
            }
        }
    }
}
