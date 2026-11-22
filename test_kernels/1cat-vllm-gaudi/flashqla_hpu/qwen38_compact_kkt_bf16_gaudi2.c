void main(tensor compact_dot, tensor grouped_beta, tensor lmat)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const ushort128 lanes = V_LANE_ID_16;
    const bfloat128 zeros = 0.0f;
    const bfloat128 ones = 1.0f;

    int5 dot_coords = {0, 0, 0, 0, 0};
    int5 beta_coords = {0, 0, 0, 0, 0};
    int5 output_coords = {0, 0, 0, 0, 0};

    for (int outer = start[3]; outer < end[3]; ++outer)
    {
        dot_coords[3] = outer;
        beta_coords[3] = outer;
        output_coords[4] = outer;
        for (int head = start[2]; head < end[2]; ++head)
        {
            dot_coords[2] = head;
            beta_coords[2] = head;
            output_coords[3] = head;
            for (int repeat = start[1]; repeat < end[1]; ++repeat)
            {
                beta_coords[1] = repeat;
                output_coords[2] = repeat;
                #pragma loop_unroll(4)
                for (int row = start[0]; row < end[0]; ++row)
                {
                    dot_coords[1] = row;
                    beta_coords[0] = row;
                    output_coords[1] = row;

                    const bfloat128 dot_row =
                        v_bf16_ld_tnsr_b(dot_coords, compact_dot);
                    const bf16 beta_row =
                        s_bf16_ld_g(gen_addr(beta_coords, grouped_beta));
                    bfloat128 result = dot_row * (float)beta_row;

                    const bool128 strict_lower = v_u16_cmp_less_b(
                        lanes, (ushort128)(unsigned short)row);
                    const bool128 diagonal = v_u16_cmp_eq_b(
                        lanes, (ushort128)(unsigned short)row);
                    result = v_bf16_mov_vb(result, 0, zeros, strict_lower);
                    result = v_bf16_mov_vb(ones, 0, result, diagonal);
                    v_bf16_st_tnsr_partial(
                        output_coords, lmat, result, 63, 0);
                }
            }
        }
    }
}
