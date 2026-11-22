// SPDX-License-Identifier: Apache-2.0
// Each SIMD lane owns a head. All heads reuse a loaded KV scalar; every
// output still executes the original ordered multiply/FMA recurrence.
void main(tensor kv, tensor indices, tensor lengths, tensor coefficients,
          tensor normalization, tensor output)
{
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int width = get_dim_size(indices, 0);
    const int rows = get_dim_size(kv, 1);
    for (int token = begin[2]; token < end[2]; ++token) {
        const int5 lc = {token, 0, 0, 0, 0};
        int count = s_i32_ld_g(gen_addr(lc, lengths));
        count = count < 0 ? 0 : count > width ? width : count;
        const int5 nc = {0, token, 0, 0, 0};
        const float64 norm = v_f32_ld_tnsr_partial_b(nc, normalization, 31, 0);
        for (int tile = begin[0]; tile < end[0]; ++tile) {
            float64 accum[8] = {0};
            for (int position = 0; position < count; ++position) {
                const int5 ic = {position, token, 0, 0, 0};
                const int row = s_i32_ld_g(gen_addr(ic, indices));
                if (row < 0 || row >= rows) continue;
                const int5 pc = {0, 0, position, token, 0};
                const int5 wc = {0, 1, position, token, 0};
                const float64 previous = v_f32_ld_tnsr_partial_b(pc, coefficients, 31, 0);
                const float64 weight = v_f32_ld_tnsr_partial_b(wc, coefficients, 31, 0);
                #pragma unroll (8)
                for (int c = 0; c < 8; ++c) {
                    const int5 kc = {tile * 8 + c, row, 0, 0, 0};
                    const bf16 packed = s_bf16_ld_g(gen_addr(kc, kv));
                    const float value = s_convert_bf16_to_f32(packed);
                    accum[c] = v_f32_mac_b((float64)value, weight, accum[c] * previous);
                }
            }
            #pragma unroll (8)
            for (int c = 0; c < 8; ++c) {
                const int5 oc = {0, tile * 8 + c, token, 0, 0};
                v_f32_st_tnsr_partial(oc, output, accum[c] * norm, 31, 0);
            }
        }
    }
}
