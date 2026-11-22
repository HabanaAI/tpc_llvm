// SPDX-License-Identifier: Apache-2.0
#define DSV41_DECODED_KV_WRITE 1
#include "deepseek_v41_swa_pack.h"
void main(tensor cache, tensor value, tensor position, tensor decoded, tensor completion, int offset) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int row = s_i32_ld_g(gen_addr((int5){0}, position));
    const bool valid = row >= 0 && row < get_dim_size(cache, 1);
    for (int group = begin[0]; group < end[0]; ++group) {
        if (valid) swa_pack_group(value, cache, group, 0, row, decoded, offset + row);
        s_i32_st_g(gen_addr((int5){group,0,0,0,0},completion), valid ? row : -1);
    }
}
