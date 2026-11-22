void main(
    tensor a0,
    tensor scores,
    tensor g_even,
    tensor g_odd,
    tensor beta,
    tensor ag,
    tensor attention)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const uint64 lanes = read_lane_id_4b_b();
    const uint64 even_columns = lanes * 2;
    const uint64 odd_columns = even_columns + 1;
    const float64 f32_zeros = 0.0f;

    int5 matrix_coords = {0, 0, 0, 0, 0};
    int5 gate_coords = {0, 0, 0, 0, 0};
    int5 beta_coords = {0, 0, 0, 0, 0};

    for (int outer = start[2]; outer < end[2]; ++outer)
    {
        matrix_coords[3] = outer;
        gate_coords[2] = outer;
        beta_coords[2] = outer;
        for (int head = start[1]; head < end[1]; ++head)
        {
            matrix_coords[2] = head;
            gate_coords[1] = head;
            gate_coords[0] = 0;
            beta_coords[1] = head;
            beta_coords[0] = 0;

            const float64 g_even_columns =
                v_f32_ld_tnsr_b(gate_coords, g_even);
            const float64 g_odd_columns =
                v_f32_ld_tnsr_b(gate_coords, g_odd);
            const bfloat128 beta_bf16 =
                v_bf16_ld_tnsr_b(beta_coords, beta);
            const float64_pair_t beta_f32 =
                v_convert_bf16_to_f32_all_b(beta_bf16);

            for (int row = start[0]; row < end[0]; ++row)
            {
                matrix_coords[1] = row;
                const bfloat128 a0_bf16 =
                    v_bf16_ld_tnsr_b(matrix_coords, a0);
                const bfloat128 score_bf16 =
                    v_bf16_ld_tnsr_b(matrix_coords, scores);
                const float64_pair_t a0_f32 =
                    v_convert_bf16_to_f32_all_b(a0_bf16);
                const float64_pair_t score_f32 =
                    v_convert_bf16_to_f32_all_b(score_bf16);

                gate_coords[0] = row >> 1;
                const float g_i = (row & 1)
                    ? s_f32_ld_g(gen_addr(gate_coords, g_odd))
                    : s_f32_ld_g(gen_addr(gate_coords, g_even));
                const bool64 lower_even = v_u32_cmp_leq_b(
                    even_columns, (uint64)row);
                const bool64 lower_odd = v_u32_cmp_leq_b(
                    odd_columns, (uint64)row);

                float64_pair_t decay_f32;
                decay_f32.v1 = v_exp_cephes_f32(
                    v_f32_mov_vb(
                        g_i - g_even_columns,
                        0,
                        f32_zeros,
                        lower_even));
                decay_f32.v2 = v_exp_cephes_f32(
                    v_f32_mov_vb(
                        g_i - g_odd_columns,
                        0,
                        f32_zeros,
                        lower_odd));
                float64_pair_t ag_f32;
                ag_f32.v1 = a0_f32.v1 * decay_f32.v1 * beta_f32.v1;
                ag_f32.v2 = a0_f32.v2 * decay_f32.v2 * beta_f32.v2;
                float64_pair_t attention_f32;
                attention_f32.v1 = score_f32.v1 * decay_f32.v1;
                attention_f32.v2 = score_f32.v2 * decay_f32.v2;
                ag_f32.v1 = v_f32_mov_vb(
                    ag_f32.v1, 0, f32_zeros, lower_even);
                ag_f32.v2 = v_f32_mov_vb(
                    ag_f32.v2, 0, f32_zeros, lower_odd);
                attention_f32.v1 = v_f32_mov_vb(
                    attention_f32.v1, 0, f32_zeros, lower_even);
                attention_f32.v2 = v_f32_mov_vb(
                    attention_f32.v2, 0, f32_zeros, lower_odd);
                const bfloat128 ag_row =
                    v_convert_f32_to_bf16_all_b(ag_f32);
                const bfloat128 attention_row =
                    v_convert_f32_to_bf16_all_b(attention_f32);

                v_bf16_st_tnsr_partial(
                    matrix_coords, ag, ag_row, 63, 0);
                v_bf16_st_tnsr_partial(
                    matrix_coords, attention, attention_row, 63, 0);
            }
        }
    }
}
