// SPDX-License-Identifier: Apache-2.0
// The complete K reduction is FP32; combine both scales before BF16 emission.
void main(tensor product, tensor weight_scale, tensor activation_scale, tensor output) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int row = start[1]; row < end[1]; ++row) {
        const int5 sx_at = {0, row, 0, 0, 0};
        const float sx = s_f32_ld_g(gen_addr(sx_at, activation_scale));
        for (int block = start[0]; block < end[0]; ++block) {
            float128 result;
            int5 at = {block * 128, row, 0, 0, 0};
            int5 sw = {block * 128, 0, 0, 0, 0};
            result.v1 = v_f32_ld_tnsr_b(at, product) * v_f32_ld_tnsr_b(sw, weight_scale) * sx;
            at[0] += 64; sw[0] += 64;
            result.v2 = v_f32_ld_tnsr_b(at, product) * v_f32_ld_tnsr_b(sw, weight_scale) * sx;
            at[0] -= 64;
            const int remaining = get_dim_size(output, 0) - block * 128;
            v_bf16_st_tnsr_partial(at, output, convert_float128_to_bfloat128(result, SW_RHNE | SW_LINEAR),
                                  remaining < 128 ? remaining - 1 : 127, 0);
        }
    }
}
