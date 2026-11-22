// SPDX-License-Identifier: Apache-2.0
void main(tensor product, tensor inverse_denominator, tensor output) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int head = start[0]; head < end[0]; ++head) {
        const float inverse = s_f32_ld_g(gen_addr((int5){0, head, 0, 0, 0}, inverse_denominator));
        for (int tile = 0; tile < 4; ++tile) {
            int5 at = {tile * 128, head, 0, 0, 0};
            float128 value;
            value.v1 = v_f32_ld_tnsr_b(at, product) * inverse;
            at[0] += 64;
            value.v2 = v_f32_ld_tnsr_b(at, product) * inverse;
            at[0] -= 64;
            v_bf16_st_tnsr(at, output, convert_float128_to_bfloat128(value, SW_LINEAR));
        }
    }
}
