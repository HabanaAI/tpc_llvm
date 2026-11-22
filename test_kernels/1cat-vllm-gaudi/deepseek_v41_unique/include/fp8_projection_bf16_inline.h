// SPDX-License-Identifier: Apache-2.0
#pragma once
#pragma clang fp contract(off)
static inline bfloat128 projection_row(tensor product, tensor channel, float sx,
                                       int5 at, int offset) {
    at[0] += offset;
    const int5 sw = {at[0], 0};
    float128 result;
    result.v1 = v_f32_ld_tnsr_b(at, product) * v_f32_ld_tnsr_b(sw, channel) * sx;
    at[0] += 64;
    result.v2 = v_f32_ld_tnsr_b(at, product) * v_f32_ld_tnsr_b(sw + (int5){64}, channel) * sx;
    return convert_float128_to_bfloat128(result, SW_RHNE | SW_LINEAR);
}
