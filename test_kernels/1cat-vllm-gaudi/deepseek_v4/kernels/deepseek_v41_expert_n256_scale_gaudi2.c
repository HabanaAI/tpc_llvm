// SPDX-License-Identifier: Apache-2.0
void main(tensor product, tensor ids, tensor activation_scale, tensor channel, tensor output)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int experts = get_dim_size(channel, 2);
    const int rows = get_dim_size(activation_scale, 1);
    for (int slot = start[1]; slot < end[1]; ++slot) {
        const int expert = s_i32_ld_g(gen_addr((int5){slot}, ids));
        const bool valid = expert >= 0 && expert < experts;
        const float sx = s_f32_ld_g(gen_addr((int5){0, rows == 1 ? 0 : slot}, activation_scale));
        for (int block = start[0]; block < end[0]; ++block) {
            const int n = block * 64;
            const float64 acc = v_f32_ld_tnsr_b((int5){n, 0, slot}, product);
            const uint64 scale_bits = v_u32_ld_tnsr_b(
                (int5){n % 256, n / 256, expert}, channel,
                SW_UNPACK | SW_UNPCK_16_TO_32, (uint64){0}, valid) << 16;
            const float64 scaled = v_f32_mul_b(v_f32_mul_b(acc, *((float64*)&scale_bits)), sx);
            const float128 pair = {scaled, (float64){0}};
            const bfloat128 rounded = convert_float128_to_bfloat128(pair, SW_RHNE | SW_LINEAR);
            v_bf16_st_tnsr_partial((int5){n, 0, slot}, output, rounded, 63, 0);
        }
    }
}
