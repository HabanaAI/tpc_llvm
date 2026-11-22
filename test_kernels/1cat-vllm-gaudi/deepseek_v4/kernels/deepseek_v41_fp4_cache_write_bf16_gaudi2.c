// SPDX-License-Identifier: Apache-2.0
#include "deepseek_v41_fp4_pack.h"
#ifndef DSV41_FP4_WRITE_ROWS
#define DSV41_FP4_WRITE_ROWS 0
#endif
void main(tensor main_cache, tensor index_cache, tensor main_value, tensor index_value,
          tensor position, tensor completion) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
#if DSV41_FP4_WRITE_ROWS
    for (int token = begin[1]; token < end[1]; ++token) {
    const int row = s_i32_ld_g(gen_addr((int5){token}, position));
#else
    const int row = s_i32_ld_g(gen_addr((int5){0}, position));
#endif
    const bool valid = row >= 0 && row < get_dim_size(main_cache, 1) && row < get_dim_size(index_cache, 1);
    for (int group = begin[0]; group < end[0]; ++group) {
        if (valid) {
#if DSV41_FP4_WRITE_ROWS
            if (group < 4) fp4_pack_group(index_value, index_cache, group, token, row, 32);
            else fp4_pack_group(main_value, main_cache, group - 4, token, row, 16);
#else
            if (group < 4) fp4_pack_group(index_value, index_cache, group, 0, row, 32);
            else fp4_pack_group(main_value, main_cache, group - 4, 0, row, 16);
#endif
        }
#if DSV41_FP4_WRITE_ROWS
        s_i32_st_g(gen_addr((int5){group, token, 0, 0, 0}, completion), valid ? row : -1);
#else
        s_i32_st_g(gen_addr((int5){group, 0, 0, 0, 0}, completion), valid ? row : -1);
#endif
    }
#if DSV41_FP4_WRITE_ROWS
    }
#endif
}
