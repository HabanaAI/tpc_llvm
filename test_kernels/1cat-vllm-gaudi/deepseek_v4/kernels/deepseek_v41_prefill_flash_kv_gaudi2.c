// SPDX-License-Identifier: Apache-2.0
// Emit selected rows, invalid-row zeros and the sink into their final layout.
void main(tensor cache, tensor indices, tensor lengths, tensor output) {
    const int5 first = get_index_space_offset();
    const int5 end = first + get_index_space_size();
    const int columns = get_dim_size(indices, 0);
    const int rows = get_dim_size(cache, 1);
    for (int token = first[1]; token < end[1]; ++token) {
        const int5 length_at = {token, 0, 0, 0, 0};
        const int length = s_i32_ld_g(gen_addr(length_at, lengths));
        for (int column = first[0] * 16; column < end[0] * 16 && column <= columns; ++column) {
            int row = -1;
            if (column < columns) {
                const int5 index_at = {column, token, 0, 0, 0};
                row = s_i32_ld_g(gen_addr(index_at, indices));
            }
            const bool valid = column < length && row >= 0 && row < rows;
            for (int block = 0; block < 4; ++block) {
                const int5 source = {block * 128, s_i32_max(row, 0), 0, 0, 0};
                const bfloat128 value = v_bf16_ld_tnsr_b(source, cache, 0, 0, valid);
                const int5 destination = {block * 128, column, token, 0, 0};
                v_bf16_st_tnsr(destination, output, value);
            }
        }
    }
}
