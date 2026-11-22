// SPDX-License-Identifier: Apache-2.0
#include "deepseek_v41_group32_roundtrip.h"

void main(tensor product, tensor weight_scale, tensor activation_scale, tensor output) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    for (int group = begin[2]; group < end[2]; ++group) {
        for (int token = begin[1]; token < end[1]; ++token) {
            const int5 sx_at = {0, token, group, 0, 0};
            const float sx = s_f32_ld_g(gen_addr(sx_at, activation_scale));
            #pragma loop_unroll(4)
            for (int tile = begin[0]; tile < end[0]; ++tile) {
                const int5 a0 = {tile * 128, token, group, 0, 0};
                const int5 a1 = {tile * 128 + 64, token, group, 0, 0};
                const int5 w0 = {tile * 128, 0, group, 0, 0};
                const int5 w1 = {tile * 128 + 64, 0, group, 0, 0};
                float128 scaled;
                scaled.v1 = (v_f32_ld_tnsr_b(a0, product) * v_f32_ld_tnsr_b(w0, weight_scale)) * sx;
                scaled.v2 = (v_f32_ld_tnsr_b(a1, product) * v_f32_ld_tnsr_b(w1, weight_scale)) * sx;
                const bfloat128 first = convert_float128_to_bfloat128(scaled, SW_RHNE | SW_LINEAR);
                const bfloat128 result = v41_group32_roundtrip_bf16(first);
                const int5 destination = {tile * 128, group, token, 0, 0};
                v_bf16_st_tnsr(destination, output, result);
            }
        }
    }
}
