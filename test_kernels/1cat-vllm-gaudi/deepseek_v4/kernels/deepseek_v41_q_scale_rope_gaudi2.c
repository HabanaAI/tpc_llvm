// SPDX-License-Identifier: Apache-2.0
// Consume a complete head. Preserve the BF16 boundary before adjacent-pair RoPE.
#pragma clang fp contract(off)
#define DSV4_QNORM_HELPERS_ONLY 1
#define DSV4_ROPE_SECOND_TERM_FMA 1
#include "deepseek_v4_qnorm_rope_kv_pack_bf16.h"
#ifndef DSV41_Q_BF16_PRODUCT
#define DSV41_Q_BF16_PRODUCT 0
#endif
#ifndef DSV41_Q_SCALE_TILED
#define DSV41_Q_SCALE_TILED 0
#endif
#if DSV41_Q_BF16_PRODUCT
#define Q_PRODUCT(AT) v_f32_ld_tnsr_b(AT,product)
void main(tensor product,tensor positions,tensor table,tensor output) {
#else
#define Q_PRODUCT(AT) (v_f32_ld_tnsr_b(AT,product) * v_f32_ld_tnsr_b((int5){(AT)[0]},weight_scale) * sx)
void main(tensor product, tensor weight_scale, tensor activation_scale,
          tensor positions, tensor table, tensor output) {
#endif
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int row = start[1]; row < end[1]; ++row) {
#if !DSV41_Q_BF16_PRODUCT
        const float sx = s_f32_ld_g(gen_addr((int5){0, row, 0, 0, 0}, activation_scale));
#endif
        const int position = s_i32_ld_g(gen_addr((int5){row, 0, 0, 0, 0}, positions));
#if DSV41_Q_SCALE_TILED
        for (int tile = start[0]; tile < end[0]; ++tile) {
            const int head = tile / 4;
            const int block = tile & 3;
            if (block < 3) {
                int5 at = {tile * 128, row, 0, 0, 0};
                float128 scaled;
                scaled.v1 = Q_PRODUCT(at);
                at[0] += 64;
                scaled.v2 = Q_PRODUCT(at);
                at[0] -= 64;
                v_bf16_st_tnsr(at, output, convert_float128_to_bfloat128(scaled, SW_RHNE | SW_LINEAR));
                continue;
            }
#else
        for (int head = start[0]; head < end[0]; ++head) {
            for (int block = 0; block < 3; ++block) {
                int5 at = {head * 512 + block * 128, row, 0, 0, 0};
                float128 scaled;
                scaled.v1 = Q_PRODUCT(at);
                at[0] += 64;
                scaled.v2 = Q_PRODUCT(at);
                at[0] -= 64;
                v_bf16_st_tnsr(at, output, convert_float128_to_bfloat128(scaled, SW_RHNE | SW_LINEAR));
            }
#endif
            int5 at = {head * 512 + 384, row, 0, 0, 0};
            float128 scaled = {0};
            scaled.v1 = Q_PRODUCT(at);
            v_bf16_st_tnsr_partial(at, output, convert_float128_to_bfloat128(scaled, SW_RHNE | SW_LINEAR), 63, 0);
            at[0] += 64;
            scaled.v1 = Q_PRODUCT(at);
            const bfloat128 rounded = convert_float128_to_bfloat128(scaled, SW_RHNE | SW_LINEAR);
            const float64 expanded = convert_bfloat128_to_float128(rounded, SW_LINEAR).v1;
            scaled.v1 = dsv4_qkv_apply_pairwise_rope_f32(expanded, table, position);
            v_bf16_st_tnsr_partial(at, output, convert_float128_to_bfloat128(scaled, SW_RHNE | SW_LINEAR), 63, 0);
        }
    }
}
