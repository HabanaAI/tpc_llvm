// SPDX-License-Identifier: Apache-2.0
#ifndef DSV41_PAGED_DECODED_ROWS
#define DSV41_PAGED_DECODED_ROWS 0
#endif
#define DSV41_DECODED_KV_WRITE 1
#include "deepseek_v41_fp4_pack.h"

// The packed row is scheduler/page-table addressed.  The decoded row is the
// bounded logical prefix row, so arbitrary scheduler block IDs do not expand
// the working set or leak one request's physical allocation into another.
void main(tensor main_cache, tensor index_cache, tensor main_value,
          tensor index_value, tensor packed_position, tensor decoded_position,
          tensor decoded, tensor completion) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
#if DSV41_PAGED_DECODED_ROWS
    for (int token = begin[1]; token < end[1]; ++token) {
#else
    const int token = 0;
#endif
    const int packed_row = s_i32_ld_g(gen_addr((int5){token}, packed_position));
    const int decoded_row = s_i32_ld_g(gen_addr((int5){token}, decoded_position));
#if DSV41_PAGED_DECODED_ROWS
    // Ratio-two queries can update one row twice. Publish only its final
    // value. Incomplete null-page rows are packed but never published
    // into the visible logical mirror, so packed and derived
    // caches cannot choose different winners from racing tensor stores.
    bool last = 1;
    for (int next = token + 1; next < get_dim_size(packed_position, 0); ++next)
        last = last && packed_row != s_i32_ld_g(gen_addr((int5){next}, packed_position));
#else
    const bool last = 1;
#endif
    const bool decoded_valid = decoded_row >= 0 && decoded_row < get_dim_size(decoded, 1);
    const bool valid = last && packed_row >= 0 && packed_row < get_dim_size(main_cache, 1) &&
                       packed_row < get_dim_size(index_cache, 1)
#if !DSV41_PAGED_DECODED_ROWS
                       && decoded_valid
#endif
                       ;
    for (int group = begin[0]; group < end[0]; ++group) {
        if (valid) {
            if (group < 4)
                fp4_pack_group(index_value, index_cache, group, token,
                               packed_row, 32, decoded, 0, decoded_row);
            else
                fp4_pack_group(main_value, main_cache, group - 4, token,
                               packed_row, 16, decoded, decoded_valid, decoded_row);
        }
        s_i32_st_g(gen_addr((int5){group,token,0,0,0}, completion),
                   valid ? decoded_row : -1);
    }
#if DSV41_PAGED_DECODED_ROWS
    }
#endif
}
