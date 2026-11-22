// SPDX-License-Identifier: Apache-2.0
// One launch, two independent TPC work points per row: control RRMS/gates
// and FFN norm/quant. The producer is an explicit BF16 hi/lo MME projection.
// Retain the original control norm reduction tree and FFN BF16 boundaries.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"
#define HC_EPS 1.0e-6f
#define SINKHORN_ITERS 20
static inline float64_pair_t mhc_bf16_to_f32_linear(bfloat128 input) {
    bfloat128_pair_t unpacked;
    unpacked.v1 = v_bf16_unpack_b(
        input,
        ((e_group_0) << 8) | ((e_every_second_element) << 9) |
            ((e_lower_half_group) << 10),
        unpacked.v1);
    unpacked.v2 = v_bf16_unpack_b(
        input,
        ((e_group_1) << 8) | ((e_every_second_element) << 9) |
            ((e_lower_half_group) << 10),
        unpacked.v2);

    const bfloat128 first_groups = unpacked.v1;
    unpacked.v1 = v_bf16_mov_dual_group_b(
        unpacked.v2, 0xFFFFFFFF, 0, 1, MkWr(1, 1), unpacked.v1);
    unpacked.v1 = v_bf16_mov_dual_group_b(
        first_groups, 0xFFFFFFFF, 1, 2, MkWr(1, 1), unpacked.v1);
    unpacked.v1 = v_bf16_mov_dual_group_b(
        unpacked.v2, 0xFFFFFFFF, 1, 3, MkWr(1, 1), unpacked.v1);
    unpacked.v2 = v_bf16_mov_dual_group_b(
        first_groups, 0xFFFFFFFF, 2, 0, MkWr(1, 1), unpacked.v2);
    unpacked.v2 = v_bf16_mov_dual_group_b(
        unpacked.v2, 0xFFFFFFFF, 2, 1, MkWr(1, 1), unpacked.v2);
    unpacked.v2 = v_bf16_mov_dual_group_b(
        first_groups, 0xFFFFFFFF, 3, 2, MkWr(1, 1), unpacked.v2);

    const float128 first = v_convert_bf16_to_f32_all_b(unpacked.v1);
    const float128 second = v_convert_bf16_to_f32_all_b(unpacked.v2);
    return (float64_pair_t){first.v1, second.v1};
}

static inline float64 sigmoid_f32(float64 value)
{
    return v_reciprocal_f32(1.0f + v_exp_cephes_f32(-value));
}

static inline uchar256 matrix_direction(uint64 index)
{
    const uint64 selected = (index & 7) | ((index & 8) << 2) | 0x80;
    uint256 packed;
    packed.v1 = selected;
    packed.v2 = selected;
    packed.v3 = selected;
    packed.v4 = selected;
    return v_convert_u32_to_u8_all_b(packed);
}

static inline float64 round_bf16(float64 value) {
    float64_pair_t pair;
    pair.v1 = value;
    pair.v2 = value;
    return v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(pair)).v1;
}

static inline float64 reciprocal_without_lookup(float64 value) {
    float64 estimate = as_float64((int64)0x7ef311c3 - as_int64(value));
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    return v_f32_sel_eq_f32_b(value, as_float64((int64)0x7f800000), 0.0f, estimate);
}

static inline float64 row_max_without_lookup(float64 value) {
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(
        value, 0xffffffff, 1, 0, 3, 2, MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(
        value, 0xffffffff, 2, 3, 0, 1, MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_group_b(value, 0xffffffff, 63, 0));
    float64 result = 0;
    #pragma loop_unroll(8)
    for (int lane = 0; lane < 8; ++lane) {
        result = v_f32_max_b(result,
            v_f32_shuffle_b(value, (uchar256)(0x80 | lane), 0, value));
    }
    return result;
}

static inline float64 load_mix(tensor mixes, int feature, int token, int last) {
    return v_f32_ld_tnsr_partial_b((int5){feature, token, 0, 0, 0}, mixes, last, 0)
         + v_f32_ld_tnsr_partial_b((int5){feature + 24, token, 0, 0, 0}, mixes, last, 0);
}
static inline void compute_gates(tensor raw_mixes, tensor residual,
                                tensor hc_scale, tensor hc_base,
                                tensor gates_out, int token, float epsilon) {
    const uint64 lane = read_lane_id_4b_b();
    const uchar256 r0 = matrix_direction(lane & 0xfffffffc);
    const uchar256 r1 = matrix_direction((lane & 0xfffffffc) | 1);
    const uchar256 r2 = matrix_direction((lane & 0xfffffffc) | 2);
    const uchar256 r3 = matrix_direction((lane & 0xfffffffc) | 3);
    const uchar256 c0 = matrix_direction(lane & 3);
    const uchar256 c1 = matrix_direction((lane & 3) | 4);
    const uchar256 c2 = matrix_direction((lane & 3) | 8);
    const uchar256 c3 = matrix_direction((lane & 3) | 12);

        float64_pair_t squares = {0};
        for (int k = 0; k < 20480; k += 128) {
            const float64_pair_t x = mhc_bf16_to_f32_linear(
                v_bf16_ld_tnsr_b((int5){k, token, 0, 0, 0}, residual));
            squares.v1 = v_f32_mac_b(x.v1, x.v1, squares.v1);
            squares.v2 = v_f32_mac_b(x.v2, x.v2, squares.v2);
        }
        const float64 inverse_rms = positive_rsqrt(
            v_f32_reduce_add(squares.v1 + squares.v2) * (1.0f / 20480.0f) + epsilon);
        const float64 pre_scale = s_f32_ld_g(gen_addr((int5){0}, hc_scale));
        const float64 post_scale = s_f32_ld_g(gen_addr((int5){1, 0, 0, 0, 0}, hc_scale));
        const float64 comb_scale = s_f32_ld_g(gen_addr((int5){2, 0, 0, 0, 0}, hc_scale));

        const float64 pre_raw = load_mix(raw_mixes, 0, token, 3);
        const float64 pre_base = v_f32_ld_tnsr_partial_b((int5){0}, hc_base, 3, 0);
        const float64 pre = sigmoid_f32(pre_raw * inverse_rms * pre_scale + pre_base) + HC_EPS;
        v_f32_st_tnsr_partial((int5){0, token, 0, 0, 0}, gates_out, pre, 3, 0);

        const float64 post_raw = load_mix(raw_mixes, 4, token, 3);
        const float64 post_base = v_f32_ld_tnsr_partial_b(
            (int5){4, 0, 0, 0, 0}, hc_base, 3, 0);
        const float64 post = sigmoid_f32(post_raw * inverse_rms * post_scale + post_base) * 2.0f;
        v_f32_st_tnsr_partial((int5){4, token, 0, 0, 0}, gates_out, post, 3, 0);

        const float64 raw = load_mix(raw_mixes, 8, token, 15);
        const float64 base = v_f32_ld_tnsr_partial_b(
            (int5){8, 0, 0, 0, 0}, hc_base, 15, 0);
        float64 values = raw * inverse_rms * comb_scale + base;
        float64 maximum = -3.402823466e+38f;
        maximum = v_f32_max_b(v_f32_shuffle_b(values, r0, 0, 0.0f), maximum);
        maximum = v_f32_max_b(v_f32_shuffle_b(values, r1, 0, 0.0f), maximum);
        maximum = v_f32_max_b(v_f32_shuffle_b(values, r2, 0, 0.0f), maximum);
        maximum = v_f32_max_b(v_f32_shuffle_b(values, r3, 0, 0.0f), maximum);
        values = v_exp_cephes_f32(values - maximum);
        float64 sum = 0.0f;
        sum += v_f32_shuffle_b(values, r0, 0, 0.0f);
        sum += v_f32_shuffle_b(values, r1, 0, 0.0f);
        sum += v_f32_shuffle_b(values, r2, 0, 0.0f);
        sum += v_f32_shuffle_b(values, r3, 0, 0.0f);
        values = values / sum + HC_EPS;

        for (int iteration = 0; iteration < SINKHORN_ITERS; ++iteration) {
            if (iteration != 0) {
                const float64 a = v_f32_shuffle_b(values, r0, 0, 0.0f)
                                + v_f32_shuffle_b(values, r1, 0, 0.0f);
                const float64 b = v_f32_shuffle_b(values, r2, 0, 0.0f)
                                + v_f32_shuffle_b(values, r3, 0, 0.0f);
                values /= (a + b) + HC_EPS;
            }
            float64 column_sum = 0.0f;
            column_sum += v_f32_shuffle_b(values, c0, 0, 0.0f);
            column_sum += v_f32_shuffle_b(values, c1, 0, 0.0f);
            column_sum += v_f32_shuffle_b(values, c2, 0, 0.0f);
            column_sum += v_f32_shuffle_b(values, c3, 0, 0.0f);
            values /= column_sum + HC_EPS;
        }
        v_f32_st_tnsr_partial((int5){8, token, 0, 0, 0}, gates_out, values, 15, 0);
}
static inline void compute_norm(tensor input, tensor weight, tensor normalized,
                               tensor quantized, tensor scales, int row,
                               float epsilon, float inverse_width) {
    const int tiles = get_dim_size(input, 0) / 128;
    bfloat128 cached[40];


        float128 squares = {0};
        for (int tile = 0; tile < tiles; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
            const bfloat128 value = v_bf16_ld_tnsr_b(at, input);
            cached[tile] = value;
            squares = v_bf16_mac_acc32_b(value, value, squares,
                                         (e_no_negation) << 1);
        }
        const float64 rrms = positive_rsqrt(
            row_sum(squares.v1 + squares.v2) * inverse_width + epsilon);

        float64 maximum = 0;
        for (int tile = 0; tile < tiles; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
            const int5 wt = {tile * 128, 0, 0, 0, 0};
            const float128 w = v_convert_bf16_to_f32_all_b(
                v_bf16_ld_tnsr_b(wt, weight));
            float128 value = v_convert_bf16_to_f32_all_b(cached[tile]);
            value.v1 = (value.v1 * rrms) * w.v1;
            value.v2 = (value.v2 * rrms) * w.v2;
            const bfloat128 rounded = v_convert_f32_to_bf16_all_b(value);
            cached[tile] = rounded;
            v_bf16_st_tnsr(at, normalized, rounded);
            const float128 expanded = v_convert_bf16_to_f32_all_b(rounded);
            maximum = v_f32_max_b(maximum, v_f32_abs_b(expanded.v1));
            maximum = v_f32_max_b(maximum, v_f32_abs_b(expanded.v2));
        }

        maximum = row_max_without_lookup(maximum);
        const float64 raw_scale = round_bf16(
            maximum * (float)(bf16)(1.0f / 240.0f));
        const float64 scale = round_bf16(
            raw_scale + (float)(bf16)(1.0e-8f / 240.0f));
        const float64 inverse = round_bf16(reciprocal_without_lookup(scale));
        const int5 scale_at = {0, row, 0, 0, 0};
        v_f32_st_tnsr(scale_at, scales, scale);

        for (int tile = 0; tile < tiles; ++tile) {
            const int5 at = {tile * 128, row, 0, 0, 0};
            const float64_pair_t value =
                v_convert_bf16_to_f32_all_b(cached[tile]);
            minifloat256 packed = 0;
            packed = v_convert_f32_to_f8_b(
                round_bf16(value.v1 * inverse), 0, SW_CLIP_FP, packed);
            packed = v_convert_f32_to_f8_b(
                round_bf16(value.v2 * inverse), 2, SW_CLIP_FP, packed);
            const minifloat256 sparse = packed;
            packed = v_f8_pack_b(sparse, SW_GROUP_0 | SW_STRIDE_2,
                                 (minifloat256)0);
            packed = v_f8_pack_b(sparse, SW_GROUP_1 | SW_STRIDE_2, packed);
            packed = v_f8_mov_dual_group_pack_b(
                packed, SW_PACK21, (minifloat256)0);
            v_f8_st_tnsr_partial(at, quantized, packed, 127, 0);
        }
}
void main(tensor mixes, tensor residual, tensor collapsed, tensor norm_weight,
          tensor hc_scale, tensor hc_base, tensor gates, tensor normalized,
          tensor quantized, tensor scales, float epsilon, float inverse_width) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    for (int row = begin[1]; row < end[1]; ++row) {
        for (int part = begin[0]; part < end[0]; ++part) {
            if (part == 0) compute_gates(mixes, residual, hc_scale, hc_base, gates, row, epsilon);
            else compute_norm(collapsed, norm_weight, normalized, quantized, scales,
                              row, epsilon, inverse_width);
        }
    }
}
