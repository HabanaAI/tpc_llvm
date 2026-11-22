// SPDX-License-Identifier: Apache-2.0
// Main 92c82b97 MLA gather, adapted to a bounded selected-row cache and C1-C6.
void main(tensor cache, tensor indices, tensor lengths, tensor keys,
          tensor values, tensor mask) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int capacity = get_dim_size(cache, 1);
    for (int token = start[1]; token < end[1]; ++token) {
        const int length = s_i32_ld_g(gen_addr((int5){token}, lengths));
        for (int row = start[0]; row < end[0]; ++row) {
            const int index = s_i32_ld_g(gen_addr((int5){row, token}, indices));
            const bool valid = row < length && index >= 0 && index < capacity;
            for (int chunk = 0; chunk < 4; ++chunk) {
                const int5 at = {chunk * 128, row, token};
                const bfloat128 value = v_bf16_ld_tnsr_b(
                    (int5){chunk * 128, index}, cache, 0, (bfloat128)0, valid);
                v_bf16_st_tnsr(at, keys, value);
                const float128 wide = convert_bfloat128_to_float128(value, SW_LINEAR);
                v_f32_st_tnsr(at, values, wide.v1);
                int5 hi = at; hi[0] += 64;
                v_f32_st_tnsr(hi, values, wide.v2);
            }
            s_f32_st_g(gen_addr((int5){row, token}, mask), valid ? 1.0f : 0.0f);
        }
    }
}
