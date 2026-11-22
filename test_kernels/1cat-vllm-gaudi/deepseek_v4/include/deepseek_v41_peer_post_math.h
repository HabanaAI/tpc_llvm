// SPDX-License-Identifier: Apache-2.0
#pragma once
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
