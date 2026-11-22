// SPDX-License-Identifier: Apache-2.0
// One index point owns a row. Cache BF16 weighted values between the two passes.
#ifndef FLASHINFER_NORM_CACHE_TILES
#define FLASHINFER_NORM_CACHE_TILES 136
#endif
static inline float64 bf16_round(float64 value) {
    float64_pair_t pair;
    pair.v1 = value;
    pair.v2 = value;
    return v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(pair)).v1;
}

#include "norm_math_gaudi2.h"

static inline float64 positive_reciprocal(float64 value) {
    float64 estimate = as_float64((int64)0x7ef311c3 - as_int64(value));
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    return v_f32_sel_eq_f32_b(value, as_float64((int64)0x7f800000), 0.0f, estimate);
}

static inline float64 row_max(float64 value) {
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(value, 0xffffffff, 1, 0, 3, 2, MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(value, 0xffffffff, 2, 3, 0, 1, MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_group_b(value, 0xffffffff, 63, 0));
    float64 result = 0;
    #pragma loop_unroll(8)
    for (int lane = 0; lane < 8; ++lane) {
        const uchar256 broadcast = 0x80 | lane;
        result = v_f32_max_b(result, v_f32_shuffle_b(value, broadcast, 0, value));
    }
    return result;
}

#ifdef FOLD_DEQUANT
#define LOAD_SUMMED(COORD, WCOORD, OUT_SUM) do { \
    const float64_pair_t raw_f_ = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b((COORD), input)); \
    const float64_pair_t sw_f_ = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b((WCOORD), swb)); \
    float64_pair_t dq_f_; \
    dq_f_.v1 = raw_f_.v1 * (sw_f_.v1 * row_a); \
    dq_f_.v2 = raw_f_.v2 * (sw_f_.v2 * row_a); \
    (OUT_SUM) = v_convert_f32_to_bf16_all_b(dq_f_) + v_bf16_ld_tnsr_b((COORD), residual); \
} while (0)
#else
#define LOAD_SUMMED(COORD, WCOORD, OUT_SUM) do { \
    (OUT_SUM) = v_bf16_ld_tnsr_b((COORD), input) + v_bf16_ld_tnsr_b((COORD), residual); \
} while (0)
#endif

#define READ_TILE(TILE, SQUARES, MAXIMUM) do { \
    const int column = (TILE) * 128; \
    const int5 coord = {column, row, 0, 0, 0}; \
    const int5 weight_coord = {column, 0, 0, 0, 0}; \
    bfloat128 summed; \
    LOAD_SUMMED(coord, weight_coord, summed); \
    v_bf16_st_tnsr(coord, residual_out, summed); \
    (SQUARES) = v_bf16_mac_acc32_b(summed, summed, (SQUARES), (e_no_negation) << 1); \
    const bfloat128 weighted = summed * v_bf16_ld_tnsr_b(weight_coord, weight); \
    cached[TILE] = weighted; \
    (MAXIMUM) = v_bf16_max_b((MAXIMUM), v_bf16_abs_b(weighted)); \
} while (0)

#if defined(FOLD_DEQUANT)
void main(tensor input, tensor sa, tensor swb, tensor residual, tensor weight,
          tensor quantized, tensor scales, tensor residual_out,
          float epsilon, float inverse_width, float inverse_range)
#elif defined(NO_NORMALIZED_OUT)
void main(tensor input, tensor residual, tensor weight,
          tensor quantized, tensor scales, tensor residual_out,
          float epsilon, float inverse_width, float inverse_range)
#else
void main(tensor input, tensor residual, tensor weight, tensor quantized,
          tensor scales, tensor normalized, tensor residual_out, float epsilon, float inverse_width, float inverse_range)
#endif
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int width = get_dim_size(input, 0);
    const int tiles = width / 128;
    const int bulk_tiles = tiles & ~3;
    // Avoid lookup intrinsics so this row cache fits the ordinary large-VLM budget.
    bfloat128 cached[FLASHINFER_NORM_CACHE_TILES];
    for (int row = start[0]; row < end[0]; ++row) {
#ifdef FOLD_DEQUANT
        const int5 sa_coord = {0, row, 0, 0, 0};
        const float64 row_a = v_broadcast_element_f32(v_f32_ld_tnsr_b(sa_coord, sa), 0);
#endif
        float128 squares0 = {0}, squares1 = {0}, squares2 = {0}, squares3 = {0};
        bfloat128 maximum0 = 0, maximum1 = 0, maximum2 = 0, maximum3 = 0;
        // The compiler cannot auto-unroll a loop-carried vector maximum.
        // Independent accumulators expose parallel loads and shorten sum chains.
        for (int tile = 0; tile < bulk_tiles; tile += 4) {
            READ_TILE(tile, squares0, maximum0);
            READ_TILE(tile + 1, squares1, maximum1);
            READ_TILE(tile + 2, squares2, maximum2);
            READ_TILE(tile + 3, squares3, maximum3);
        }
        for (int tile = bulk_tiles; tile < tiles; ++tile) {
            READ_TILE(tile, squares0, maximum0);
        }
        const float64 sum_lo = (squares0.v1 + squares1.v1) + (squares2.v1 + squares3.v1);
        const float64 sum_hi = (squares0.v2 + squares1.v2) + (squares2.v2 + squares3.v2);
        const float64 rrms = positive_rsqrt(row_sum(sum_lo + sum_hi) * inverse_width + epsilon);
        const float64_pair_t max_f = v_convert_bf16_to_f32_all_b(
            v_bf16_max_b(v_bf16_max_b(maximum0, maximum1), v_bf16_max_b(maximum2, maximum3)));
        float64 maximum = v_f32_max_b(max_f.v1, max_f.v2);
        // BF16 rounding and positive multiplication are monotone. Therefore
        // max(abs(BF16(weighted * rrms))) equals BF16(max(abs(weighted)) * rrms).
        // Compute both reductions while reading inputs; no separate norm/max pass.
        maximum = bf16_round(row_max(maximum) * rrms);
        const float64 scale = bf16_round(bf16_round(maximum + 1.0e-8f) * inverse_range);
        const float64 inverse = bf16_round(positive_reciprocal(scale));
        float64_pair_t inverse_pair;
        inverse_pair.v1 = inverse;
        inverse_pair.v2 = inverse;
        const bfloat128 inverse_bf16 = v_convert_f32_to_bf16_all_b(inverse_pair);
        const int5 scale_coord = {0, row, 0, 0, 0};
        v_f32_st_tnsr(scale_coord, scales, scale);
        const int pairs = (width / 128) & ~1;
        #pragma loop_unroll(2)
        for (int tile = 0; tile < pairs; tile += 2) {
            float64_pair_t v0 = v_convert_bf16_to_f32_all_b(cached[tile]);
            float64_pair_t v1 = v_convert_bf16_to_f32_all_b(cached[tile + 1]);
            v0.v1 = v0.v1 * rrms; v0.v2 = v0.v2 * rrms;
            v1.v1 = v1.v1 * rrms; v1.v2 = v1.v2 * rrms;
            const bfloat128 n0 = v_convert_f32_to_bf16_all_b(v0);
            const bfloat128 n1 = v_convert_f32_to_bf16_all_b(v1);
            const int5 c0 = {tile * 128, row, 0, 0, 0};
#ifndef NO_NORMALIZED_OUT
            const int5 c1 = {(tile + 1) * 128, row, 0, 0, 0};
            v_bf16_st_tnsr(c0, normalized, n0);
            v_bf16_st_tnsr(c1, normalized, n1);
#endif
            const minifloat256 sp0 = v_convert_bf16_to_f8_b(n0 * inverse_bf16, 0, SW_CLIP_FP, (minifloat256)0);
            minifloat256 pk0 = v_f8_pack_b(sp0, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            pk0 = v_f8_pack_b(sp0, SW_GROUP_1 | SW_STRIDE_2, pk0);
            pk0 = v_f8_mov_dual_group_pack_b(pk0, SW_PACK21, (minifloat256)0);
            const minifloat256 sp1 = v_convert_bf16_to_f8_b(n1 * inverse_bf16, 0, SW_CLIP_FP, (minifloat256)0);
            minifloat256 pk1 = v_f8_pack_b(sp1, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            pk1 = v_f8_pack_b(sp1, SW_GROUP_1 | SW_STRIDE_2, pk1);
            pk1 = v_f8_mov_dual_group_pack_b(pk1, SW_PACK21, (minifloat256)0);
            pk0 = v_f8_mov_dual_group_b(pk1, 0xffffffff, 0, 2, MkWr(1, 1), pk0);
            pk0 = v_f8_mov_dual_group_b(pk1, 0xffffffff, 1, 3, MkWr(1, 1), pk0);
            v_f8_st_tnsr(c0, quantized, pk0);
        }
        for (int tile = pairs; tile < width / 128; ++tile) {
            float64_pair_t value = v_convert_bf16_to_f32_all_b(cached[tile]);
            value.v1 = value.v1 * rrms;
            value.v2 = value.v2 * rrms;
            const bfloat128 normed = v_convert_f32_to_bf16_all_b(value);
            const int5 coord = {tile * 128, row, 0, 0, 0};
#ifndef NO_NORMALIZED_OUT
            v_bf16_st_tnsr(coord, normalized, normed);
#endif
            const bfloat128 scaled = normed * inverse_bf16;
            const minifloat256 sparse = v_convert_bf16_to_f8_b(scaled, 0, SW_CLIP_FP, (minifloat256)0);
            minifloat256 packed = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2, (minifloat256)0);
            packed = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, packed);
            packed = v_f8_mov_dual_group_pack_b(packed, SW_PACK21, (minifloat256)0);
            v_f8_st_tnsr_partial(coord, quantized, packed, 127, 0);
        }
    }
}
