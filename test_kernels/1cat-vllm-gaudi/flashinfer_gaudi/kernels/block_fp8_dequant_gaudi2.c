// SPDX-License-Identifier: Apache-2.0
// The model contract rounds the scale to BF16 BEFORE multiplication.
void main(tensor weight, tensor scale, tensor output) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int width = get_dim_size(weight, 0);
    for (int block_n = begin[1]; block_n < end[1]; ++block_n) {
        for (int block_k = begin[0]; block_k < end[0]; ++block_k) {
            int5 scale_coord = {block_k * 2, block_n, 0, 0, 0};
            const bfloat128 multiplier = (bf16)s_f32_ld_g(gen_addr(scale_coord, scale));
            scale_coord[0] += 1;
            const bool second = block_k * 256 + 128 < width;
            const bfloat128 multiplier2 = (bf16)s_f32_ld_g(gen_addr(scale_coord, scale), 0, 0, second);
            #pragma loop_unroll(4) pipelined taken
            for (int row = 0; row < 128; ++row) {
                int5 coord = {block_k * 256, block_n * 128 + row, 0, 0, 0};
                // Expand bytes in the load pipe, directly into the conversion
                // lanes. This avoids the generic linear conversion's shuffles.
                const minifloat256 packed = v_f8_ld_tnsr_b(coord, weight, SW_UNPACK | SW_UNPCK_8_TO_16);
                const bfloat128 values = v_convert_f8_to_bf16_b(packed);
                v_bf16_st_tnsr(coord, output, values * multiplier);
                coord[0] += 128;
                const minifloat256 packed2 = v_f8_ld_tnsr_b(coord, weight, SW_UNPACK | SW_UNPCK_8_TO_16, 0, second);
                const bfloat128 values2 = v_convert_f8_to_bf16_b(packed2);
                v_bf16_st_tnsr(coord, output, values2 * multiplier2, 0, second);
            }
        }
    }
}
