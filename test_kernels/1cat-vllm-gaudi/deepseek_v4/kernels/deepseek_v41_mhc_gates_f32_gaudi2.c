// SPDX-License-Identifier: Apache-2.0
// Keep each 4x4 mHC matrix in SIMD lanes. The scalar-per-entry implementation
// replicated each matrix entry across a full vector and spilled local vectors.
// Row and column shuffle reductions retain its FP32 addition order.
#define HC_EPS 1.0e-6f
#define SINKHORN_ITERS 20
#ifndef DSV41_MHC_MME_EPILOGUE
#define DSV41_MHC_MME_EPILOGUE 0
#endif
#ifndef DSV41_MHC_PARTIAL_STATISTICS
#define DSV41_MHC_PARTIAL_STATISTICS 0
#endif
#if DSV41_MHC_MME_EPILOGUE
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"
#endif

static inline float64 projection_part(tensor projection, int5 at, int last) {
    float64 value = v_f32_ld_tnsr_partial_b(at, projection, last, 0);
#if DSV41_MHC_MME_EPILOGUE
    if (get_dim_size(projection, 0) == 48) {
        at[0] += 24;
        value += v_f32_ld_tnsr_partial_b(at, projection, last, 0);
    }
#endif
    return value;
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

#ifdef DSV41_MHC_POSITIVE_DIV
// Sinkhorn row/column denominators are finite and strictly positive after epsilon.
// Two FP32 Newton corrections avoid generic reciprocal range/LUT machinery.
static inline float64 sinkhorn_div(float64 numerator,float64 denominator) {
    float64 inverse=as_float64((int64)0x7ef311c3-as_int64(denominator));
    inverse*=v_f32_mac_b(-denominator,inverse,2.0f);
    inverse*=v_f32_mac_b(-denominator,inverse,2.0f);
    return numerator*inverse;
}
#else
static inline float64 sinkhorn_div(float64 numerator,float64 denominator) { return numerator/denominator; }
#endif

void main(tensor raw_mixes, tensor rrms, tensor hc_scale, tensor hc_base,
          tensor gates_out)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const uint64 lane = read_lane_id_4b_b();
    const uchar256 r0 = matrix_direction(lane & 0xfffffffc);
    const uchar256 r1 = matrix_direction((lane & 0xfffffffc) | 1);
    const uchar256 r2 = matrix_direction((lane & 0xfffffffc) | 2);
    const uchar256 r3 = matrix_direction((lane & 0xfffffffc) | 3);
    const uchar256 c0 = matrix_direction(lane & 3);
    const uchar256 c1 = matrix_direction((lane & 3) | 4);
    const uchar256 c2 = matrix_direction((lane & 3) | 8);
    const uchar256 c3 = matrix_direction((lane & 3) | 12);
    for (int token = start[0]; token < end[0]; ++token) {
#if DSV41_MHC_MME_EPILOGUE
        // The input is already BF16. One scan supplies RRMS directly to
        // Sinkhorn, avoiding FP32 expansion/reduction and the hi/lo add node.
#if DSV41_MHC_PARTIAL_STATISTICS
        float64 mean = v_f32_reduce_add(v_f32_ld_tnsr_partial_b((int5){0, token}, rrms, 39, 0))
                       * (1.0f / 20480.0f);
#else
        float128 square = {0};
        for (int col = 0; col < 20480; col += 128) {
            const float128 x = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b((int5){col, token}, rrms));
            square.v1 += x.v1 * x.v1;
            square.v2 += x.v2 * x.v2;
        }
        float64 mean = v_f32_reduce_add(square.v1 + square.v2) * (1.0f / 20480.0f);
#endif
        mean = v_f32_shuffle_b(mean, (uchar256)0x80, 0, mean);
        const float64 inverse_rms = positive_rsqrt(mean + 1.0e-20f);
#else
        const float64 inverse_rms = s_f32_ld_g(
            gen_addr((int5){0, token, 0, 0, 0}, rrms));
#endif
        const float64 pre_scale = s_f32_ld_g(gen_addr((int5){0}, hc_scale));
        const float64 post_scale = s_f32_ld_g(gen_addr((int5){1, 0, 0, 0, 0}, hc_scale));
        const float64 comb_scale = s_f32_ld_g(gen_addr((int5){2, 0, 0, 0, 0}, hc_scale));

        const float64 pre_raw = projection_part(raw_mixes, (int5){0, token, 0, 0, 0}, 3);
        const float64 pre_base = v_f32_ld_tnsr_partial_b((int5){0}, hc_base, 3, 0);
        const float64 pre = sigmoid_f32(pre_raw * inverse_rms * pre_scale + pre_base) + HC_EPS;
        v_f32_st_tnsr_partial((int5){0, token, 0, 0, 0}, gates_out, pre, 3, 0);

        const float64 post_raw = projection_part(raw_mixes, (int5){4, token, 0, 0, 0}, 3);
        const float64 post_base = v_f32_ld_tnsr_partial_b(
            (int5){4, 0, 0, 0, 0}, hc_base, 3, 0);
        const float64 post = sigmoid_f32(post_raw * inverse_rms * post_scale + post_base) * 2.0f;
        v_f32_st_tnsr_partial((int5){4, token, 0, 0, 0}, gates_out, post, 3, 0);

        const float64 raw = projection_part(raw_mixes, (int5){8, token, 0, 0, 0}, 15);
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
        values = sinkhorn_div(values,sum) + HC_EPS;

        for (int iteration = 0; iteration < SINKHORN_ITERS; ++iteration) {
            if (iteration != 0) {
                const float64 a = v_f32_shuffle_b(values, r0, 0, 0.0f)
                                + v_f32_shuffle_b(values, r1, 0, 0.0f);
                const float64 b = v_f32_shuffle_b(values, r2, 0, 0.0f)
                                + v_f32_shuffle_b(values, r3, 0, 0.0f);
                values = sinkhorn_div(values,(a + b) + HC_EPS);
            }
            float64 column_sum = 0.0f;
            column_sum += v_f32_shuffle_b(values, c0, 0, 0.0f);
            column_sum += v_f32_shuffle_b(values, c1, 0, 0.0f);
            column_sum += v_f32_shuffle_b(values, c2, 0, 0.0f);
            column_sum += v_f32_shuffle_b(values, c3, 0, 0.0f);
            values = sinkhorn_div(values,column_sum + HC_EPS);
        }
        v_f32_st_tnsr_partial((int5){8, token, 0, 0, 0}, gates_out, values, 15, 0);
    }
}
