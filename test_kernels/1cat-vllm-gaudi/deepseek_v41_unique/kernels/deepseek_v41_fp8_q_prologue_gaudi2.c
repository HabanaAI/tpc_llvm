// SPDX-License-Identifier: Apache-2.0
#include "../include/fp8_projection_bf16_inline.h"
#define DSV41_QNORM_FUNCTION 1
#define DSV41_QNORM_PUBLISH 1
#define DSV41_QNORM_INPUT_WIDTH 1280
#define DSV41_QNORM_EXTRA_INPUTS tensor channel, float sx,
#define DSV41_QNORM_LOAD_VALUE(at) projection_row(input, channel, sx, (at), 0)
#include "../../deepseek_v4/kernels/deepseek_v41_qnorm_quant_gaudi2.c"
void main(tensor product, tensor channel, tensor activation_scale, tensor qnorm,
          tensor quantized, tensor scales, tensor normalized, float epsilon) {
    const int5 begin = get_index_space_offset(), end = begin + get_index_space_size();
    for (int row = begin[0]; row < end[0]; ++row) {
        const float sx = s_f32_ld_g(gen_addr((int5){0, row}, activation_scale));
        qnorm_quant_row(product, qnorm, channel, sx, quantized, scales, normalized,
                        epsilon, 1.0f / 1280.0f, (int5){row}, (int5){row + 1});
    }
}
