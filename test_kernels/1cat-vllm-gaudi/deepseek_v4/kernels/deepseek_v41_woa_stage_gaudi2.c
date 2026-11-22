// SPDX-License-Identifier: Apache-2.0
// Preserve prepared bytes while exposing a tile producer to the SRAM slicer.
void main(tensor input, tensor output) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int group = start[2]; group < end[2]; ++group) {
        for (int row = start[1]; row < end[1]; ++row) {
            for (int block = start[0]; block < end[0]; ++block) {
                #pragma unroll
                for (int k = 0; k < 16; ++k) {
                    const int5 at = {block * 256, row * 16 + k, group, 0, 0};
                    v_f8_st_tnsr(at, output, v_f8_ld_tnsr_b(at, input));
                }
            }
        }
    }
}
