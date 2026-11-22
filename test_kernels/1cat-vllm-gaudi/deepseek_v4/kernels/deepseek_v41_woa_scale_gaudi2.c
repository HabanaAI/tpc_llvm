// SPDX-License-Identifier: Apache-2.0
// FP32 [G,T,N] -> BF16 [T,G,N]; the full K reduction completed in MME.
void main(tensor product, tensor weight_scale, tensor activation_scale, tensor output) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int group = start[2]; group < end[2]; ++group) {
        for (int token = start[1]; token < end[1]; ++token) {
            const int5 sx_at = {0, token, group, 0, 0};
            const float sx = s_f32_ld_g(gen_addr(sx_at, activation_scale));
            for (int block = start[0]; block < end[0]; ++block) {
                float128 result;
                int5 p = {block * 128, token, group, 0, 0};
                int5 s = {block * 128, 0, group, 0, 0};
                result.v1 = v_f32_ld_tnsr_b(p, product) * v_f32_ld_tnsr_b(s, weight_scale) * sx;
                p[0] += 64; s[0] += 64;
                result.v2 = v_f32_ld_tnsr_b(p, product) * v_f32_ld_tnsr_b(s, weight_scale) * sx;
                const bfloat128 value = convert_float128_to_bfloat128(result, SW_RHNE | SW_LINEAR);
                const int5 dest = {block * 128, group, token, 0, 0};
                v_bf16_st_tnsr(dest, output, value);
            }
        }
    }
}
