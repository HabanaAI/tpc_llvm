// SPDX-License-Identifier: Apache-2.0
// Late consumer keeps the BF16 per-route boundary and original route order.
// FP32 multiplication associates channel*sx before multiplying the accumulator;
// this numerical change requires the official/C1 and teacher-forced gates.
void main(tensor product, tensor factors, tensor output) {
    const int5 start = get_index_space_offset(), end = start + get_index_space_size();
    for (int token = start[1]; token < end[1]; ++token) {
        for (int block = start[0]; block < end[0]; ++block) {
            const int n = block * 128;
            float128 accumulated = {0};
            #pragma loop_unroll(6)
            for (int slot = 0; slot < 6; ++slot) {
                const int row = token * 6 + slot;
                float128 scaled;
                scaled.v1 = v_f32_ld_tnsr_b((int5){n, 0, row}, product)
                            * v_f32_ld_tnsr_b((int5){n, 0, row}, factors);
                scaled.v2 = v_f32_ld_tnsr_b((int5){n + 64, 0, row}, product)
                            * v_f32_ld_tnsr_b((int5){n + 64, 0, row}, factors);
                const float128 rounded = v_convert_bf16_to_f32_all_b(
                    convert_float128_to_bfloat128(scaled, SW_RHNE | SW_LINEAR));
                accumulated.v1 += rounded.v1;
                accumulated.v2 += rounded.v2;
            }
            v_bf16_st_tnsr((int5){n, 0, token}, output,
                          v_convert_f32_to_bf16_all_b(accumulated, SW_RHNE));
        }
    }
}
