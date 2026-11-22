// SPDX-License-Identifier: Apache-2.0
// Gather already-rounded route rows and preserve the original six-add order.
void main(tensor rows, tensor inverse, tensor output)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int token = start[1]; token < end[1]; ++token) {
        for (int block = start[0]; block < end[0]; ++block) {
            const int n = block * 128;
            const int first = s_i32_ld_g(gen_addr((int5){token * 6}, inverse));
            float128 sum = v_convert_bf16_to_f32_all_b(
                v_bf16_ld_tnsr_b((int5){n, 0, first}, rows));
            #pragma loop_unroll(5)
            for (int slot = 1; slot < 6; ++slot) {
                const int row = s_i32_ld_g(gen_addr((int5){token * 6 + slot}, inverse));
                const float128 value = v_convert_bf16_to_f32_all_b(
                    v_bf16_ld_tnsr_b((int5){n, 0, row}, rows));
                sum.v1 += value.v1;
                sum.v2 += value.v2;
            }
            v_bf16_st_tnsr((int5){n, token}, output,
                           v_convert_f32_to_bf16_all_b(sum, SW_RHNE));
        }
    }
}
