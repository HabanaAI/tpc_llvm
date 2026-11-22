// SPDX-License-Identifier: Apache-2.0
#include "../include/fp8_projection_bf16_inline.h"
#define DSV41_KV_NORM_FUNCTION 1
#define DSV41_KV_NORM_EXTRA_INPUTS tensor channel, float sx,
#define DSV41_KV_NORM_LOAD_VALUE(at) projection_row(input, channel, sx, (at), 1280)
#include "../../deepseek_v4/kernels/deepseek_v41_kv_norm_rope_bf16_gaudi2.c"
void main(tensor product, tensor channel, tensor activation_scale, tensor kvnorm,
          tensor positions, tensor phase, tensor rotated, float epsilon) {
    const int5 begin = get_index_space_offset(), end = begin + get_index_space_size();
    for (int row = begin[0]; row < end[0]; ++row) {
        const float sx = s_f32_ld_g(gen_addr((int5){0, row}, activation_scale));
        kv_norm_rope_row(product, kvnorm, channel, sx, positions, phase, rotated,
                         epsilon, 1.0f / 512.0f, (int5){row}, (int5){row + 1});
    }
}
