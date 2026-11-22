// SPDX-License-Identifier: Apache-2.0
#include "deepseek_v41_attention_interleave.h"

void main(tensor value, tensor positions, tensor phase_table, tensor output) {
    const int5 begin = get_index_space_offset(), end = begin + get_index_space_size();
    for (int token = begin[2]; token < end[2]; ++token) {
        const int position = s_i32_ld_g(gen_addr((int5){token,0,0,0,0}, positions));
        const float64 phase = v_f32_ld_tnsr_b((int5){0,position,0,0,0}, phase_table);
        for (int group = begin[1]; group < end[1]; ++group) {
            for (int block = begin[0]; block < end[0]; ++block) {
                const int5 at = {block*128,group,token,0,0};
                bfloat128 result = v_bf16_ld_tnsr_b(at, value);
                if (block >= 28) {
                    float128 x = convert_bfloat128_to_float128(result, SW_LINEAR);
                    const int pair = (block - 28) * 8;
                    x.v1 = interleaved_rope(x.v1, phase, pair, 0);
                    x.v2 = interleaved_rope(x.v2, phase, pair + 4, 0);
                    result = convert_float128_to_bfloat128(x, SW_RHNE | SW_LINEAR);
                }
                v_bf16_st_tnsr(at, output, result);
            }
        }
    }
}
