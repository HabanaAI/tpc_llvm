// SPDX-License-Identifier: Apache-2.0
#define DSV41_DECODED_KV_WRITE 1
#include "deepseek_v41_swa_pack.h"

// Quantize once, publish the canonical circular paged row, and retain the
// same quantized value in the bounded decoded working set.
void main(tensor cache, tensor value, tensor packed_position,
          tensor decoded_position, tensor decoded, tensor completion,
          int offset) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    // The paged C1 producer and its decoded mirror share the same 256-row
    // circular namespace.  Consume the logical position directly so every
    // layer does not materialize a separate div/mod tensor before this
    // writer.  Ring indices passed by older callers remain valid because
    // masking 0..255 is an identity operation.
    const int packed_logical = s_i32_ld_g(gen_addr((int5){0}, packed_position));
    const int decoded_logical = s_i32_ld_g(gen_addr((int5){0}, decoded_position));
    const int packed_row = packed_logical & 255;
    const int decoded_row = decoded_logical & 255;
    const bool valid = packed_logical >= 0 && decoded_logical >= 0 &&
                       packed_row < get_dim_size(cache, 1) &&
                       offset + decoded_row < get_dim_size(decoded, 1);
    for (int group = begin[0]; group < end[0]; ++group) {
        if (valid)
            swa_pack_group(value, cache, group, 0, packed_row,
                           decoded, offset + decoded_row);
        s_i32_st_g(gen_addr((int5){group,0,0,0,0}, completion),
                   valid ? decoded_row : -1);
    }
}
