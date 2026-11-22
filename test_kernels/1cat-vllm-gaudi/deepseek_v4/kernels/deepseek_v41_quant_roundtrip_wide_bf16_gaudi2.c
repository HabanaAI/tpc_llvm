// SPDX-License-Identifier: Apache-2.0
#include "deepseek_v41_group32_roundtrip.h"

void main(tensor input, tensor output) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int width = get_dim_size(input, 0);
    // Full vectors use ordinary loads so the compiler can overlap independent
    // codec iterations. Only the owning core handles the partial final vector.
    const int full = width / 128;
    const int full_end = s_i32_min(end[0], full);
    for (int row = begin[1]; row < end[1]; ++row) {
        #pragma loop_unroll(4)
        for (int tile = begin[0]; tile < full_end; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
            const bfloat128 packed = v_bf16_ld_tnsr_b(at, input);
            const bfloat128 result = v41_group32_roundtrip_bf16(packed);
            v_bf16_st_tnsr(at, output, result);
        }
        if (width % 128 && begin[0] <= full && full < end[0]) {
            const int5 at = {full * 128, row, 0, 0, 0};
            const int last = width % 128 - 1;
            const bfloat128 packed = v_bf16_ld_tnsr_partial_b(at, input, last, 0);
            const bfloat128 result = v41_group32_roundtrip_bf16(packed);
            v_bf16_st_tnsr_partial(at, output, result, last, 0);
        }
    }
}
