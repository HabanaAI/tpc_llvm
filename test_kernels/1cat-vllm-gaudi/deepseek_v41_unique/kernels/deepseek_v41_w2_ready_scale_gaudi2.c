// SPDX-License-Identifier: Apache-2.0
// Prepare channel*activation scales independently of the W2 MME result.
void main(tensor ids, tensor activation_scale, tensor channel, tensor factors) {
    const int5 start = get_index_space_offset(), end = start + get_index_space_size();
    const int experts = get_dim_size(channel, 2);
    for (int row = start[1]; row < end[1]; ++row) {
        const int expert = s_i32_ld_g(gen_addr((int5){row}, ids));
        const bool valid = expert >= 0 && expert < experts;
        const float sx = s_f32_ld_g(gen_addr((int5){0, row}, activation_scale));
        for (int block = start[0]; block < end[0]; ++block) {
            const int n = block * 128;
            const bfloat128 packed = v_bf16_ld_tnsr_b((int5){n % 256, n / 256, expert},
                                                      channel, 0, (bfloat128)0, valid);
            const float128 scale = convert_bfloat128_to_float128(packed, SW_LINEAR);
            v_f32_st_tnsr((int5){n, 0, row}, factors, scale.v1 * sx);
            v_f32_st_tnsr((int5){n + 64, 0, row}, factors, scale.v2 * sx);
        }
    }
}
