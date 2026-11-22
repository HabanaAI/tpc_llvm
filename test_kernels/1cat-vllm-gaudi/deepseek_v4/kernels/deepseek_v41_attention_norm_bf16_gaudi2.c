// SPDX-License-Identifier: Apache-2.0
// Reuse FlashInfer-Gaudi row reduction/rsqrt, preserving V4.1's FP32 weight
// product and final BF16 boundary rather than the vendor BF16 product.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"
#ifndef DSV41_INPUT_NORM_FULL_CACHE
#define DSV41_INPUT_NORM_FULL_CACHE 0
#endif
#if DSV41_INPUT_NORM_FULL_CACHE
#define DSV41_NORM_TILES(F) \
    F(0) F(1) F(2) F(3) F(4) F(5) F(6) F(7) F(8) F(9) \
    F(10) F(11) F(12) F(13) F(14) F(15) F(16) F(17) F(18) F(19) \
    F(20) F(21) F(22) F(23) F(24) F(25) F(26) F(27) F(28) F(29) \
    F(30) F(31) F(32) F(33) F(34) F(35) F(36) F(37) F(38) F(39)
#endif
void main(tensor input, tensor weight, tensor output, float epsilon, float inverse_width) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
#if !DSV41_INPUT_NORM_FULL_CACHE
    const int tiles = get_dim_size(input, 0) / 128;
    // 40 tiles covers the model-width final norm (5120) while retaining the
    // existing 512/1280 attention rows.  The bound is compile-time so C1 and
    // batched rows share one kernel contract without a batch specialization.
    bfloat128 cached[40];
#endif
    for (int row = begin[0]; row < end[0]; ++row) {
        float128 squares = {0};
#if DSV41_INPUT_NORM_FULL_CACHE
#define CACHE(T) \
        const bfloat128 cached##T = v_bf16_ld_tnsr_b((int5){T * 128, row}, input); \
        squares = v_bf16_mac_acc32_b(cached##T, cached##T, squares, (e_no_negation) << 1);
        DSV41_NORM_TILES(CACHE)
#undef CACHE
#else
        for (int tile = 0; tile < tiles; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
            const bfloat128 value = v_bf16_ld_tnsr_b(at, input);
            cached[tile] = value;
            squares = v_bf16_mac_acc32_b(value, value, squares, (e_no_negation) << 1);
        }
#endif
        const float64 rrms = positive_rsqrt(row_sum(squares.v1 + squares.v2) * inverse_width + epsilon);
#if DSV41_INPUT_NORM_FULL_CACHE
#define EMIT(T) do { \
        const float128 w = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b((int5){T * 128, 0}, weight)); \
        float128 value = v_convert_bf16_to_f32_all_b(cached##T); \
        value.v1 = (value.v1 * rrms) * w.v1; \
        value.v2 = (value.v2 * rrms) * w.v2; \
        v_bf16_st_tnsr((int5){T * 128, row}, output, v_convert_f32_to_bf16_all_b(value)); \
    } while (0);
        DSV41_NORM_TILES(EMIT)
#undef EMIT
#else
        for (int tile = 0; tile < tiles; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
            const int5 wt = {tile * 128, 0, 0, 0, 0};
            const float128 w = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wt, weight));
            float128 value = v_convert_bf16_to_f32_all_b(cached[tile]);
            value.v1 = (value.v1 * rrms) * w.v1;
            value.v2 = (value.v2 * rrms) * w.v2;
            v_bf16_st_tnsr(at, output, v_convert_f32_to_bf16_all_b(value));
        }
#endif
    }
}
