void main(tensor dot, tensor gate, tensor beta, tensor kkt)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const ushort128 lanes = V_LANE_ID_16;
    const bfloat128 zeros = 0.0f;
    const bfloat128 ones = 1.0f;

    int5 matrix_coords = {0, 0, 0, 0, 0};
    int5 vector_coords = {0, 0, 0, 0, 0};

    for (int matrix = start[1]; matrix < end[1]; ++matrix)
    {
        matrix_coords[2] = matrix;
        vector_coords[1] = matrix;
        vector_coords[0] = 0;
        const bfloat128 gate_columns =
            v_bf16_ld_tnsr_b(vector_coords, gate);
        const float64_pair_t gate_columns_f32 =
            v_convert_bf16_to_f32_all_b(gate_columns);

        #pragma loop_unroll(4) pipelined taken
        for (int row = start[0]; row < end[0]; ++row)
        {
            matrix_coords[1] = row;
            bfloat128 dot_row = v_bf16_ld_tnsr_b(matrix_coords, dot);

            vector_coords[0] = row;
            const float gate_row =
                (float)s_bf16_ld_g(gen_addr(vector_coords, gate));
            const bf16 beta_row = s_bf16_ld_g(gen_addr(vector_coords, beta));
            const bfloat128 beta_vector = (float)beta_row;

            float64_pair_t decay_f32;
            decay_f32.v1 = v_exp_cephes_f32(
                gate_row - gate_columns_f32.v1);
            decay_f32.v2 = v_exp_cephes_f32(
                gate_row - gate_columns_f32.v2);
            const bfloat128 decay =
                v_convert_f32_to_bf16_all_b(decay_f32);

            const bfloat128 coefficient = decay * beta_vector;
            bfloat128 result = dot_row * coefficient;
            const bool128 strict_lower = v_u16_cmp_less_b(
                lanes, (ushort128)(unsigned short)row);
            const bool128 diagonal = v_u16_cmp_eq_b(
                lanes, (ushort128)(unsigned short)row);
            result = v_bf16_mov_vb(result, 0, zeros, strict_lower);
            result = v_bf16_mov_vb(ones, 0, result, diagonal);
            v_bf16_st_tnsr_partial(matrix_coords, kkt, result, 63, 0);
        }
    }
}
