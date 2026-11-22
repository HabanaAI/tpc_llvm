// SPDX-License-Identifier: Apache-2.0
// Consume the unscaled FP8 MME accumulator directly. Retain the shared C1
// BF16 projection boundary in registers, then reuse its Q/KV normalization.
// This is a generic C1-C6 capability, independent of the tensor-parallel size.
#pragma clang fp contract(off)
#include "../include/fp8_projection_bf16_inline.h"
#define DSV41_QNORM_FUNCTION 1
#define DSV41_QNORM_PUBLISH 1
#define DSV41_QNORM_INPUT_WIDTH 1280
#define DSV41_QNORM_EXTRA_INPUTS tensor channel, float sx,
#define DSV41_QNORM_LOAD_VALUE(at) projection_row(input, channel, sx, (at), 0)
#include "../../deepseek_v4/kernels/deepseek_v41_qnorm_quant_gaudi2.c"
#define DSV41_KV_NORM_FUNCTION 1
#define DSV41_KV_NORM_EXTRA_INPUTS tensor channel, float sx,
#define DSV41_KV_NORM_LOAD_VALUE(at) projection_row(input, channel, sx, (at), 1280)
#include "../../deepseek_v4/kernels/deepseek_v41_kv_norm_rope_bf16_gaudi2.c"

void main(tensor product, tensor channel, tensor activation_scale, tensor qnorm, tensor kvnorm,
          tensor positions, tensor phase, tensor quantized, tensor scales, tensor normalized,
          tensor rotated, float epsilon) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    for (int row = begin[1]; row < end[1]; ++row) {
        const float sx = s_f32_ld_g(gen_addr((int5){0, row}, activation_scale));
        for (int point = begin[0]; point < end[0]; ++point) {
            if (point == 0) {
                qnorm_quant_row(product, qnorm, channel, sx, quantized, scales,
                                normalized, epsilon, 1.0f / 1280.0f,
                                (int5){row}, (int5){row + 1});
            } else {
                kv_norm_rope_row(product, kvnorm, channel, sx, positions, phase,
                                 rotated, epsilon, 1.0f / 512.0f,
                                 (int5){row}, (int5){row + 1});
            }
        }
    }
}
