void main(tensor lower, tensor inverse)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const uint64 lanes = read_lane_id_4b_b();
    const float64 zeros = 0.0f;
    const float64 ones = 1.0f;
    float64 inverse_rows[16];

    for (int outer = start[2]; outer < end[2]; ++outer)
    {
        for (int head = start[1]; head < end[1]; ++head)
        {
            for (int block = start[0]; block < end[0]; ++block)
            {
                int5 coords = {0, 0, block, head, outer};

#pragma unroll(16)
                for (int row = 0; row < 16; ++row)
                {
                    bool64 diagonal = v_u32_cmp_eq_b(lanes, (uint64)row);
                    float64 inverse_row = v_f32_mov_vb(
                        ones, 0, zeros, diagonal);

#pragma unroll(16)
                    for (int k = 0; k < row; ++k)
                    {
                        coords[0] = k;
                        coords[1] = row;
                        const float coefficient = s_f32_ld_g(
                            gen_addr(coords, lower));
                        inverse_row = v_f32_mac_b(
                            inverse_rows[k],
                            -coefficient,
                            inverse_row);
                    }

                    inverse_rows[row] = inverse_row;
                    coords[0] = 0;
                    coords[1] = row;
                    v_f32_st_tnsr_partial(
                        coords, inverse, inverse_row, 15, 0);
                }
            }
        }
    }
}
