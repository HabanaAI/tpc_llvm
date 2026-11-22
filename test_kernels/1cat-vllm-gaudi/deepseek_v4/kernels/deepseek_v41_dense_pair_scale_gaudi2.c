// SPDX-License-Identifier: Apache-2.0
// Scale the two Engram batch-GEMM results in one TPC launch.  The MME
// accumulator is FP32; combine its per-token and per-output-channel scales
// before restoring the existing BF16 consumer boundary.
void main(tensor product, tensor weight_scale, tensor activation_scale,
          tensor output) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int batch = start[2]; batch < end[2]; ++batch) {
        for (int row = start[1]; row < end[1]; ++row) {
            const int5 sx_at = {0, row, batch, 0, 0};
            const float sx = s_f32_ld_g(gen_addr(sx_at, activation_scale));
            for (int block = start[0]; block < end[0]; ++block) {
                float128 result;
                int5 at = {block * 128, row, batch, 0, 0};
                int5 sw = {block * 128, 0, batch, 0, 0};
                result.v1 = v_f32_ld_tnsr_b(at, product)
                            * v_f32_ld_tnsr_b(sw, weight_scale) * sx;
                at[0] += 64;
                sw[0] += 64;
                result.v2 = v_f32_ld_tnsr_b(at, product)
                            * v_f32_ld_tnsr_b(sw, weight_scale) * sx;
                at[0] -= 64;
                v_bf16_st_tnsr(
                    at, output,
                    convert_float128_to_bfloat128(result,
                                                   SW_RHNE | SW_LINEAR));
            }
        }
    }
}
