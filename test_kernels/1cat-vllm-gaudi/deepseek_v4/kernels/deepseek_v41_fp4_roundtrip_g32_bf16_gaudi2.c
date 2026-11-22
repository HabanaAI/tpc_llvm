// SPDX-License-Identifier: Apache-2.0
// Exact UE8M0/group-32 FP4 activation roundtrip.  The packed representation
// exists only in registers; only the final BF16 values are written.
#define DSV41_DECODED_KV_WRITE 1
#define DSV41_FP4_DECODE_ONLY 1
#define DSV41_FP4_PRESERVE_ZERO_SIGN 1
#include "deepseek_v41_fp4_pack.h"

void main(tensor value, tensor output) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    for (int row = begin[1]; row < end[1]; ++row)
        for (int group = begin[0]; group < end[0]; ++group)
            fp4_pack_group(value, output, group, row, row, 32,
                           output, 1, row);
}
