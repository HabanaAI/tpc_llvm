// SPDX-License-Identifier: Apache-2.0
// One bounded query tile owns selected BF16 KV. The output is a recipe-local
// SRAM candidate; no FP32 copy of KV is made for the PV MME operation.
void main(tensor cache, tensor indices, tensor lengths, tensor keys, tensor mask) {
    const int5 first = get_index_space_offset();
    const int5 end = first + get_index_space_size();
    const int capacity = get_dim_size(cache, 1);
    for (int token = first[1]; token < end[1]; ++token) {
        const int length = s_i32_ld_g(gen_addr((int5){token}, lengths));
        for (int row = first[0]; row < end[0]; ++row) {
            const int index = s_i32_ld_g(gen_addr((int5){row, token}, indices));
            const bool valid = row < length && index >= 0 && index < capacity;
            for (int chunk = 0; chunk < 4; ++chunk) {
                const bfloat128 value = v_bf16_ld_tnsr_b(
                    (int5){chunk * 128, s_i32_max(index, 0)}, cache, 0, (bfloat128)0, valid);
                v_bf16_st_tnsr((int5){chunk * 128, row, token}, keys, value);
            }
            s_f32_st_g(gen_addr((int5){row, token}, mask), valid ? 1.0f : 0.0f);
        }
    }
}
