// SPDX-License-Identifier: Apache-2.0
#include "deepseek_v41_swa_pack.h"
void main(tensor value, tensor output) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    for (int row = begin[1]; row < end[1]; ++row)
        for (int group = begin[0]; group < end[0]; ++group)
            swa_pack_group(value, output, group, row, row);
}
