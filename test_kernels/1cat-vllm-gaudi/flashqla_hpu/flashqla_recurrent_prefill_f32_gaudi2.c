#define ROWS_PER_WORK_ITEM 16

void main(
    tensor q,
    tensor k,
    tensor v,
    tensor decay,
    tensor beta,
    tensor initial_state,
    tensor output,
    tensor final_state)
{
    const uchar256 broadcast_lane_zero = 0x80;
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int tokens = get_dim_size(q, 2);
    const float scale = 0.08838834764831845f;

    float64 state_lo[ROWS_PER_WORK_ITEM];
    float64 state_hi[ROWS_PER_WORK_ITEM];

    for (int head = start[1]; head < end[1]; ++head)
    {
        for (int row_group = start[0]; row_group < end[0]; ++row_group)
        {
            const int row_base = row_group * ROWS_PER_WORK_ITEM;
            int5 state_coords = {0, row_base, head, 0, 0};

#pragma unroll(ROWS_PER_WORK_ITEM)
            for (int row_slot = 0; row_slot < ROWS_PER_WORK_ITEM; ++row_slot)
            {
                state_coords[0] = 0;
                state_coords[1] = row_base + row_slot;
                state_lo[row_slot] = v_f32_ld_tnsr_b(
                    state_coords, initial_state);
                state_coords[0] = 64;
                state_hi[row_slot] = v_f32_ld_tnsr_b(
                    state_coords, initial_state);
            }

            int5 qk_coords = {0, head, 0, 0, 0};
            int5 scalar_coords = {head, 0, 0, 0, 0};
            int5 value_coords = {row_base, head, 0, 0, 0};
            int5 output_coords = {row_base, head, 0, 0, 0};

            for (int token = 0; token < tokens; ++token)
            {
                qk_coords[0] = 0;
                qk_coords[2] = token;
                float64 q_lo = v_f32_ld_tnsr_b(qk_coords, q);
                float64 k_lo = v_f32_ld_tnsr_b(qk_coords, k);
                qk_coords[0] = 64;
                float64 q_hi = v_f32_ld_tnsr_b(qk_coords, q);
                float64 k_hi = v_f32_ld_tnsr_b(qk_coords, k);

                scalar_coords[1] = token;
                const float decay_value = s_f32_ld_g(
                    gen_addr(scalar_coords, decay));
                const float beta_value = s_f32_ld_g(
                    gen_addr(scalar_coords, beta));
                value_coords[2] = token;
                output_coords[2] = token;

#pragma unroll(ROWS_PER_WORK_ITEM)
                for (int row_slot = 0;
                     row_slot < ROWS_PER_WORK_ITEM;
                     ++row_slot)
                {
                    const int value_row = row_base + row_slot;
                    value_coords[0] = value_row;
                    const float v_value = s_f32_ld_g(
                        gen_addr(value_coords, v));

                    state_lo[row_slot] *= decay_value;
                    state_hi[row_slot] *= decay_value;

                    float64 projection = state_lo[row_slot] * k_lo;
                    projection = v_f32_mac_b(
                        state_hi[row_slot], k_hi, projection);
                    projection = v_f32_reduce_add(projection);
                    projection = v_f32_shuffle_b(
                        projection,
                        broadcast_lane_zero,
                        0,
                        projection);
                    const float64 v_new =
                        (v_value - projection) * beta_value;

                    state_lo[row_slot] = v_f32_mac_b(
                        k_lo, v_new, state_lo[row_slot]);
                    state_hi[row_slot] = v_f32_mac_b(
                        k_hi, v_new, state_hi[row_slot]);

                    float64 out_value = state_lo[row_slot] * q_lo;
                    out_value = v_f32_mac_b(
                        state_hi[row_slot], q_hi, out_value);
                    out_value = v_f32_reduce_add(out_value) * scale;
                    output_coords[0] = value_row;
                    v_f32_st_tnsr_partial(
                        output_coords, output, out_value, 0, 0);
                }
            }

#pragma unroll(ROWS_PER_WORK_ITEM)
            for (int row_slot = 0; row_slot < ROWS_PER_WORK_ITEM; ++row_slot)
            {
                state_coords[0] = 0;
                state_coords[1] = row_base + row_slot;
                v_f32_st_tnsr(
                    state_coords, final_state, state_lo[row_slot]);
                state_coords[0] = 64;
                v_f32_st_tnsr(
                    state_coords, final_state, state_hi[row_slot]);
            }
        }
    }
}
