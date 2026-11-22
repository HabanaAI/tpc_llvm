// SPDX-License-Identifier: Apache-2.0
#define DSV41_DECODED_KV_WRITE 1
#include "deepseek_v41_fp4_pack.h"
void main(tensor main_cache, tensor index_cache, tensor main_value, tensor index_value,
          tensor position, tensor decoded, tensor completion) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int row = s_i32_ld_g(gen_addr((int5){0}, position));
    const bool valid = row >= 0 && row < get_dim_size(main_cache,1) && row < get_dim_size(index_cache,1);
    for (int group = begin[0]; group < end[0]; ++group) {
        if (valid) {
            if (group < 4) fp4_pack_group(index_value,index_cache,group,0,row,32,decoded,0,row);
            else fp4_pack_group(main_value,main_cache,group-4,0,row,16,decoded,1,row);
        }
        s_i32_st_g(gen_addr((int5){group,0,0,0,0},completion),valid ? row : -1);
    }
}
