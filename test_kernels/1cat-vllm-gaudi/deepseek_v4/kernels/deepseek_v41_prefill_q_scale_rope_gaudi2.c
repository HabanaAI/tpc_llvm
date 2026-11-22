// SPDX-License-Identifier: Apache-2.0
// Scale the MME product, retain its BF16 boundary, and rotate only the tail.
#pragma clang fp contract(off)
#define DSV4_QNORM_HELPERS_ONLY 1
#include "deepseek_v4_qnorm_rope_kv_pack_bf16.h"

void main(tensor product, tensor weight_scale, tensor activation_scale,
          tensor positions, tensor table, tensor output) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int token = start[1]; token < end[1]; ++token) {
        const float sx = s_f32_ld_g(gen_addr((int5){0, token, 0, 0, 0}, activation_scale));
        const int position = s_i32_ld_g(gen_addr((int5){token, 0, 0, 0, 0}, positions));
        for (int head = start[0]; head < end[0]; ++head) {
            for (int block = 0; block < 4; ++block) {
                int5 at = {head * 512 + block * 128, token, 0, 0, 0};
                int5 wt = {at[0], 0, 0, 0, 0};
                float128 scaled;
#ifdef DSV41_Q_RAW_BF16
                const float128 raw = convert_bfloat128_to_float128(v_bf16_ld_tnsr_b(at, product), SW_LINEAR);
                scaled.v1 = (raw.v1 * v_f32_ld_tnsr_b(wt, weight_scale)) * sx;
                at[0] += 64;
                wt[0] += 64;
                scaled.v2 = (raw.v2 * v_f32_ld_tnsr_b(wt, weight_scale)) * sx;
#else
                scaled.v1 = (v_f32_ld_tnsr_b(at, product) * v_f32_ld_tnsr_b(wt, weight_scale)) * sx;
                at[0] += 64;
                wt[0] += 64;
                scaled.v2 = (v_f32_ld_tnsr_b(at, product) * v_f32_ld_tnsr_b(wt, weight_scale)) * sx;
#endif
                const bfloat128 rounded = convert_float128_to_bfloat128(scaled, SW_RHNE | SW_LINEAR);
                at[0] -= 64;
                if (block != 3) {
                    v_bf16_st_tnsr(at, output, rounded);
                } else {
                    v_bf16_st_tnsr_partial(at, output, rounded, 63, 0);
                    const float128 expanded = convert_bfloat128_to_float128(rounded, SW_LINEAR);
                    float128 rotated = {0};
                    rotated.v1 = dsv4_qkv_apply_pairwise_rope_f32(expanded.v2, table, position);
                    at[0] += 64;
                    v_bf16_st_tnsr_partial(at, output,
                        convert_float128_to_bfloat128(rotated, SW_RHNE | SW_LINEAR), 63, 0);
                }
            }
        }
    }
}
