// SPDX-License-Identifier: Apache-2.0
// Each index point produces 128 elements. Both packed halves remain in HBM
// layout; activation and multiplication are executed in this one TPC kernel.
void main(tensor input, tensor output)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = get_dim_size(output, 0);
    for (int row = start[1]; row < end[1]; ++row) {
        #pragma loop_unroll(4) pipelined taken
        for (int tile = start[0]; tile < end[0]; ++tile) {
            int5 coord = {tile * 128, row, 0, 0, 0};
            const float64_pair_t gate = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(coord, input));
            coord[0] += width;
            const bfloat128 value = v_bf16_ld_tnsr_b(coord, input);
            float64_pair_t activated;
            activated.v1 = gate.v1 * v_sigmoid_f32(gate.v1);
            activated.v2 = gate.v2 * v_sigmoid_f32(gate.v2);
            // Match the BF16 activation boundary of the established HPU op.
            const bfloat128 silu = v_convert_f32_to_bf16_all_b(activated);
            coord[0] -= width;
            v_bf16_st_tnsr(coord, output, silu * value);
        }
    }
}
