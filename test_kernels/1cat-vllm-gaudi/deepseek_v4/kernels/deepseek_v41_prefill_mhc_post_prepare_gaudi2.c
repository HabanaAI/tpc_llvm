// SPDX-License-Identifier: Apache-2.0
// Fuse the first mHC residual update with the next sublayer's ordered
// collapse and row-RRMS preparation.  The rounded BF16 residual is the
// semantic boundary: collapse and RRMS consume that value, exactly as the
// unfused producer/consumer chain does.
void main(tensor value, tensor residual, tensor post, tensor comb,
          tensor next_pre, tensor residual_out, tensor collapsed_out,
          tensor rrms_out)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int token = start[0]; token < end[0]; ++token) {
        float post_weight[4];
        float comb_weight[4][4];
        float collapse_weight[4];
        #pragma unroll
        for (int target = 0; target < 4; ++target) {
            int5 pc = {target, token, 0, 0, 0};
            post_weight[target] = s_f32_ld_g(gen_addr(pc, post));
            collapse_weight[target] = s_f32_ld_g(gen_addr(pc, next_pre));
            #pragma unroll
            for (int source = 0; source < 4; ++source) {
                int5 cc = {target, source, token, 0, 0};
                comb_weight[source][target] = s_f32_ld_g(gen_addr(cc, comb));
            }
        }

        float64 sumsq = 0.0f;
        for (int block = 0; block < 40; ++block) {
            const int feature = block * 128;
            int5 vc = {feature, token, 0, 0, 0};
            const float128 x = v_convert_bf16_to_f32_all_b(
                v_bf16_ld_tnsr_b(vc, value));
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
                    source_value[0].v1, comb_weight[0][target]);
                updated[target].v2 = v_f32_mul_b(
                    source_value[0].v2, comb_weight[0][target]);
                #pragma unroll
                for (int source = 1; source < 4; ++source) {
                    const float64 lo = v_f32_mul_b(
                        source_value[source].v1, comb_weight[source][target]);
                    const float64 hi = v_f32_mul_b(
                        source_value[source].v2, comb_weight[source][target]);
                    updated[target].v1 = v_f32_add_b(updated[target].v1, lo);
                    updated[target].v2 = v_f32_add_b(updated[target].v2, hi);
                }
                const float64 projected_lo = v_f32_mul_b(
                    x.v1, post_weight[target]);
                const float64 projected_hi = v_f32_mul_b(
                    x.v2, post_weight[target]);
                updated[target].v1 = v_f32_add_b(
                    projected_lo, updated[target].v1);
                updated[target].v2 = v_f32_add_b(
                    projected_hi, updated[target].v2);
                const bfloat128 rounded = v_convert_f32_to_bf16_all_b(
                    updated[target], SW_RHNE);
                int5 oc = {feature, target, token, 0, 0};
                v_bf16_st_tnsr(oc, residual_out, rounded);
                updated[target] = v_convert_bf16_to_f32_all_b(rounded);
                sumsq = v_f32_mac_b(updated[target].v1,
                                    updated[target].v1, sumsq);
                sumsq = v_f32_mac_b(updated[target].v2,
                                    updated[target].v2, sumsq);
            }

            float128 collapsed;
            collapsed.v1 = v_f32_mul_b(updated[0].v1, collapse_weight[0]);
            collapsed.v2 = v_f32_mul_b(updated[0].v2, collapse_weight[0]);
            #pragma unroll
            for (int target = 1; target < 4; ++target) {
                const float64 lo = v_f32_mul_b(
                    updated[target].v1, collapse_weight[target]);
                const float64 hi = v_f32_mul_b(
                    updated[target].v2, collapse_weight[target]);
                collapsed.v1 = v_f32_add_b(collapsed.v1, lo);
                collapsed.v2 = v_f32_add_b(collapsed.v2, hi);
            }
            int5 cc = {feature, token, 0, 0, 0};
            v_bf16_st_tnsr(cc, collapsed_out,
                           v_convert_f32_to_bf16_all_b(collapsed, SW_RHNE));
        }

        const float64 reduced = v_f32_reduce_add(sumsq);
        const uchar256 lane_zero = 0x80;
        const float64 scalar = v_f32_shuffle_b(reduced, lane_zero, 0, reduced);
        const float64 rrms = v_rsqrt_f32(scalar / 20480.0f + 1.0e-20f);
        int5 rc = {0, token, 0, 0, 0};
        v_f32_st_tnsr_partial(rc, rrms_out, rrms, 0, 0);
    }
}
