// SPDX-License-Identifier: Apache-2.0
// Independent heads share vector instructions; key traversal stays ordered.
void main(tensor scores, tensor indices, tensor sink, tensor lengths,
          tensor coefficients, tensor normalization, int sequence_length)
{
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int width = get_dim_size(indices, 0);
    const int5 zero = {0};
    const float64 sinks = v_f32_ld_tnsr_partial_b(zero, sink, 31, 0);
    for (int token = begin[2]; token < end[2]; ++token) {
        const int5 lc = {token, 0, 0, 0, 0};
        int count = s_i32_ld_g(gen_addr(lc, lengths));
        count = count < 0 ? 0 : count > width ? width : count;
        float64 maximum = -3.402823466e+38f;
        float64 sum = 0.0f;
        for (int position = 0; position < count; ++position) {
            const int5 ic = {position, token, 0, 0, 0};
            const int row = s_i32_ld_g(gen_addr(ic, indices));
            if (row < 0 || row >= sequence_length) continue;
            const int5 sc = {0, position, token, 0, 0};
            const float64 score = v_f32_ld_tnsr_partial_b(sc, scores, 31, 0);
            const float64 next = v_f32_max_b(maximum, score);
            const float64 previous = v_exp_cephes_f32(maximum - next);
            const float64 weight = v_exp_cephes_f32(score - next);
            sum = sum * previous + weight;
            const int5 pc = {0, 0, position, token, 0};
            const int5 wc = {0, 1, position, token, 0};
            v_f32_st_tnsr_partial(pc, coefficients, previous, 31, 0);
            v_f32_st_tnsr_partial(wc, coefficients, weight, 31, 0);
            maximum = next;
        }
        const float64 final_max = v_f32_max_b(maximum, sinks);
        const float64 data_scale = v_exp_cephes_f32(maximum - final_max);
        const float64 sink_weight = v_exp_cephes_f32(sinks - final_max);
        sum = sum * data_scale + sink_weight;
        const float64 result = data_scale * v_reciprocal_f32(sum);
        const int5 nc = {0, token, 0, 0, 0};
        v_f32_st_tnsr_partial(nc, normalization, result, 31, 0);
    }
}
