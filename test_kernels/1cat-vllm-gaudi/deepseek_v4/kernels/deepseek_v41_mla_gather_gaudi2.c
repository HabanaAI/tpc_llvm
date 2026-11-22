// SPDX-License-Identifier: Apache-2.0
// One selected KV row serves every query head. Completion tensors preserve
// the incremental cache writer -> gather -> matrix consumer dependency.
void main(tensor swa, tensor main_kv, tensor indices, tensor lengths,
          tensor swa_done, tensor main_done, tensor keys,
#ifndef DSV41_MLA_SHARED_KV
          tensor values,
#endif
          tensor mask, int swa_offset, int main_rows, int prefix_rows) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const bool ready = s_i32_ld_g(gen_addr((int5){0}, swa_done)) >= 0 &&
                       s_i32_ld_g(gen_addr((int5){0}, main_done)) >= 0;
    const int length = s_i32_ld_g(gen_addr((int5){0}, lengths));
    for (int row = start[0]; row < end[0]; ++row) {
        const int index = s_i32_ld_g(gen_addr((int5){row, 0, 0, 0, 0}, indices));
        const bool valid = ready && row < length && index >= 0 && index < prefix_rows + main_rows;
        for (int chunk = 0; chunk < 4; ++chunk) {
            const int5 at = {chunk * 128, row, 0, 0, 0};
            bfloat128 value = 0;
            if (valid) {
                int5 source = {chunk * 128, index, 0, 0, 0};
                if (index < prefix_rows) {
                    source[1] += swa_offset;
                    value = v_bf16_ld_tnsr_b(source, swa);
                } else {
                    source[1] -= prefix_rows;
                    value = v_bf16_ld_tnsr_b(source, main_kv);
                }
            }
            v_bf16_st_tnsr(at, keys, value);
#ifndef DSV41_MLA_SHARED_KV
            const float128 wide = convert_bfloat128_to_float128(value, SW_LINEAR);
            v_f32_st_tnsr(at, values, wide.v1);
            int5 hi = at; hi[0] += 64;
            v_f32_st_tnsr(hi, values, wide.v2);
#endif
        }
        s_f32_st_g(gen_addr((int5){row, 0, 0, 0, 0}, mask), valid ? 1.0f : 0.0f);
    }
}
