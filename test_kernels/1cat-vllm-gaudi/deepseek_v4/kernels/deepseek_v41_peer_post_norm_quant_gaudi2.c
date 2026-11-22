// SPDX-License-Identifier: Apache-2.0
// Fuse ordered TP sum, rounded mHC update/collapse and exact FFN norm/quant.
// Four workpoints recompute the row statistics and publish disjoint feature
// tiles; there is no cross-TPC barrier or intermediate device tensor.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"

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


typedef struct {
    float post[4];
    float comb[4][4];
    float pre[4];
} PostWeights;

static inline PostWeights load_post_weights(tensor post, tensor comb, tensor pre, int token) {
    PostWeights weights;
    #pragma unroll
    for (int target = 0; target < 4; ++target) {
        weights.post[target] = s_f32_ld_g(gen_addr((int5){target, token}, post));
        weights.pre[target] = s_f32_ld_g(gen_addr((int5){target, token}, pre));
        #pragma unroll
        for (int source = 0; source < 4; ++source)
            weights.comb[source][target] = s_f32_ld_g(
                gen_addr((int5){target, source, token}, comb));
    }
    return weights;
}

static inline bfloat128 post_tile(tensor value, tensor residual, PostWeights weights,
    tensor residual_out, tensor collapsed_out, int token, int block, bool publish) {
            const int feature = block * 128;
            int5 vc = {feature, token, 0, 0, 0};
            float128 x = v_convert_bf16_to_f32_all_b(
                v_bf16_ld_tnsr_b(vc, value));
            // A peer tensor is [features, tokens, ranks]. Accumulate in the
            // same fixed FP32 rank order as the shared collective consumer,
            // then preserve its BF16 boundary before the residual update.
            const int ranks = get_dim_size(value, 2);
            for (int rank = 1; rank < ranks; ++rank) {
                vc[2] = rank;
                const float128 peer = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(vc, value));
                x.v1 = v_f32_add_b(x.v1, peer.v1);
                x.v2 = v_f32_add_b(x.v2, peer.v2);
            }
            if (ranks > 1) {
                x = v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(x, SW_RHNE));
            }
            float128 source_value[4];
            #pragma unroll
            for (int source = 0; source < 4; ++source) {
                int5 rc = {feature, source, token, 0, 0};
                source_value[source] = v_convert_bf16_to_f32_all_b(
                    v_bf16_ld_tnsr_b(rc, residual));
            }

            float128 updated[4];
            #pragma unroll
            for (int target = 0; target < 4; ++target) {
                // Preserve the production expression order: source 0 starts
                // the residual mix, sources 1..3 are added in order, and the
                // projected value is added last.  Starting from x*post here
                // changes FP32 rounding before the BF16 semantic boundary.
                updated[target].v1 = v_f32_mul_b(
                    source_value[0].v1, weights.comb[0][target]);
                updated[target].v2 = v_f32_mul_b(
                    source_value[0].v2, weights.comb[0][target]);
                #pragma unroll
                for (int source = 1; source < 4; ++source) {
                    const float64 lo = v_f32_mul_b(
                        source_value[source].v1, weights.comb[source][target]);
                    const float64 hi = v_f32_mul_b(
                        source_value[source].v2, weights.comb[source][target]);
                    updated[target].v1 = v_f32_add_b(updated[target].v1, lo);
                    updated[target].v2 = v_f32_add_b(updated[target].v2, hi);
                }
                // The deployed decode fuser contracts this final product
                // and add. Keep its single FP32 rounding before BF16; a
                // separate multiply/add changes exact BF16 midpoint cases.
                updated[target].v1 = v_f32_mac_b(
                    x.v1, weights.post[target], updated[target].v1);
                updated[target].v2 = v_f32_mac_b(
                    x.v2, weights.post[target], updated[target].v2);
                const bfloat128 rounded = v_convert_f32_to_bf16_all_b(
                    updated[target], SW_RHNE);
                int5 oc = {feature, target, token, 0, 0};
                if (publish) v_bf16_st_tnsr(oc, residual_out, rounded);
                updated[target] = v_convert_bf16_to_f32_all_b(rounded);
            }

            float128 collapsed;
            collapsed.v1 = v_f32_mul_b(updated[0].v1, weights.pre[0]);
            collapsed.v2 = v_f32_mul_b(updated[0].v2, weights.pre[0]);
            #pragma unroll
            for (int target = 1; target < 4; ++target) {
                const float64 lo = v_f32_mul_b(
                    updated[target].v1, weights.pre[target]);
                const float64 hi = v_f32_mul_b(
                    updated[target].v2, weights.pre[target]);
                collapsed.v1 = v_f32_add_b(collapsed.v1, lo);
                collapsed.v2 = v_f32_add_b(collapsed.v2, hi);
            }
            int5 cc = {feature, token, 0, 0, 0};
            const bfloat128 result = v_convert_f32_to_bf16_all_b(collapsed, SW_RHNE);
            if (publish) v_bf16_st_tnsr(cc, collapsed_out, result);
            return result;
}

void main(tensor value, tensor residual, tensor post, tensor comb, tensor next_pre,
          tensor weight, tensor residual_out, tensor collapsed_out, tensor normalized,
          tensor quantized, tensor scales, float epsilon, float inverse_width) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int tiles = 40;
    const int row = 0;
    bfloat128 cached[40];
    // These 24 scalars are invariant across the forty feature tiles.
    // The previous inlined tile helper reloaded them inside that loop.
    const PostWeights weights = load_post_weights(post, comb, next_pre, row);

    for (int owner = begin[0]; owner < end[0]; ++owner) {
        float128 squares = {0};
        for (int tile = 0; tile < tiles; ++tile) {
            const bfloat128 item = post_tile(value,residual,weights,residual_out,collapsed_out,
                                              row,tile,tile % 4 == owner);
            cached[tile] = item;
            squares = v_bf16_mac_acc32_b(item, item, squares,
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
            if (tile % 4 == owner) v_bf16_st_tnsr(at, normalized, rounded);
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
        if (owner == 0) v_f32_st_tnsr(scale_at, scales, scale);

        // Statistics need the complete row, but quantization is elementwise.
        // Each owner only consumes its ten cached tiles after the global amax.
        for (int tile = owner; tile < tiles; tile += 4) {
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
}
