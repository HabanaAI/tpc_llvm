// The inverse of unit lower-triangular 16x16 f32 blocks by forward
// substitution, as in the chunked (WY) form of gated delta rule attention: row
// i of the inverse is e_i - sum_{k<i} L[i][k] (row k of the inverse). 64 blocks
// of 16 tokens (1024 tokens) for each of 6 heads, one block per member. Fully
// unrolled: 120 scalar global loads of coefficients, MACs with a scalar
// operand, the rows of the inverse kept in registers, 16-lane partial stores.
// Only the strictly lower triangle of the input is read.
// From 1cat-vllm-gaudi (https://github.com/1CatAI/1cat-vllm-gaudi, commit e2266e0e),
// csrc/flashqla_hpu/flashqla_unit_lower_inverse16_f32_gaudi2.c; Apache-2.0 (the project's LICENSE file).
//
// BENCH: arch gaudi2
// BENCH: in  f32 16 16 64 6 1 range=-0.25,0.25
// BENCH: out f32 16 16 64 6 1
// BENCH: index-space 64 6 1

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
