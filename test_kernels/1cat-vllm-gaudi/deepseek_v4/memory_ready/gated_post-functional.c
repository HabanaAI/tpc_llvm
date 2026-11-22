// SPDX-License-Identifier: Apache-2.0
// Fuse residual update and its next ordered collapse by feature tiles.
// Keep row reductions outside this kernel to retain TPC parallelism.  The rounded BF16 residual is the
// semantic boundary: collapse and RRMS consume that value, exactly as the
// unfused producer/consumer chain does.
#pragma clang fp contract(off)
#include "norm_math_gaudi2.h"
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


static inline float64 gate(float64 values,int lane) {
    return v_f32_shuffle_b(values,(uchar256)(0x80 | (lane&7) | ((lane&8)<<2)),0,values);
}

void main(tensor value, tensor residual, tensor gates, tensor norm_weight, tensor ready, tensor enabled, tensor residual_out, tensor collapsed_out, tensor norm_statistics, tensor status, int flag_line)
{
    // Private finite diagnostic, not production acquisition certification.
    __global volatile int* flag = gen_addr((int5){0,flag_line}, ready);
    cache_invalidate(SW_D);
    const int active = s_i32_ld_g(gen_addr((int5){0},enabled));
    int acquired = !active;
    for (int spin = 0; spin < 65536 && !acquired; ++spin) {
        cache_invalidate(SW_D);
        acquired = *flag == 1;
    }
    const int5 start = get_index_space_offset();
    if (!acquired) {
      const int5 stop=start+get_index_space_size();
      for(int block=start[0];block<stop[0];++block)
        v_i32_st_tnsr_partial((int5){block},status,(int64)-1,0,0);
      return;
    }
    const int5 end = start + get_index_space_size();
    for (int token = start[1]; token < end[1]; ++token) {
        const float64 first8 = v_f32_ld_tnsr_partial_b((int5){0,token}, gates, 7, 0);
        const float64 last16 = v_f32_ld_tnsr_partial_b((int5){8,token}, gates, 15, 0);
        const float64 first = v_f32_mov_dual_group_all_b(first8,0xffffffff,0,0,0,0,MkWrA(3,3,3,3),0);
        const float64 matrix = v_f32_mov_dual_group_all_b(last16,0xffffffff,0,0,0,0,MkWrA(3,3,3,3),0);

        for (int block = start[0]; block < end[0]; ++block) {
            v_i32_st_tnsr_partial((int5){block},status,(int64)1,0,0);
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
                    source_value[0].v1, gate(matrix,target));
                updated[target].v2 = v_f32_mul_b(
                    source_value[0].v2, gate(matrix,target));
                #pragma unroll
                for (int source = 1; source < 4; ++source) {
                    const float64 lo = v_f32_mul_b(
                        source_value[source].v1, gate(matrix,source*4+target));
                    const float64 hi = v_f32_mul_b(
                        source_value[source].v2, gate(matrix,source*4+target));
                    updated[target].v1 = v_f32_add_b(updated[target].v1, lo);
                    updated[target].v2 = v_f32_add_b(updated[target].v2, hi);
                }
                // The deployed decode fuser contracts this final product
                // and add. Keep its single FP32 rounding before BF16; a
                // separate multiply/add changes exact BF16 midpoint cases.
                updated[target].v1 = v_f32_mac_b(
                    x.v1, gate(first,target+4), updated[target].v1);
                updated[target].v2 = v_f32_mac_b(
                    x.v2, gate(first,target+4), updated[target].v2);
                const bfloat128 rounded = v_convert_f32_to_bf16_all_b(
                    updated[target], SW_RHNE);
                int5 oc = {feature, target, token, 0, 0};
                v_bf16_st_tnsr(oc, residual_out, rounded);
                updated[target] = v_convert_bf16_to_f32_all_b(rounded);
            }

            float128 collapsed;
            collapsed.v1 = v_f32_mul_b(updated[0].v1, gate(first,0));
            collapsed.v2 = v_f32_mul_b(updated[0].v2, gate(first,0));
            #pragma unroll
            for (int target = 1; target < 4; ++target) {
                const float64 lo = v_f32_mul_b(
                    updated[target].v1, gate(first,target));
                const float64 hi = v_f32_mul_b(
                    updated[target].v2, gate(first,target));
                collapsed.v1 = v_f32_add_b(collapsed.v1, lo);
                collapsed.v2 = v_f32_add_b(collapsed.v2, hi);
            }
            int5 cc = {feature, token, 0, 0, 0};
            const bfloat128 rounded = v_convert_f32_to_bf16_all_b(collapsed, SW_RHNE);
            v_bf16_st_tnsr(cc, collapsed_out, rounded);
            const float128 squares = v_bf16_mac_acc32_b(rounded, rounded, (float128){0}, (e_no_negation)<<1);
            const float128 weights = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b((int5){feature},norm_weight));
            const float128 values = v_convert_bf16_to_f32_all_b(rounded);
            const float64 max_weighted = row_max_without_lookup(v_f32_max_b(
                v_f32_abs_b(values.v1 * weights.v1),v_f32_abs_b(values.v2 * weights.v2)));
            v_f32_st_tnsr_partial((int5){block,0,token},norm_statistics,row_sum(squares.v1+squares.v2),0,0);
            v_f32_st_tnsr_partial((int5){block,1,token},norm_statistics,max_weighted,0,0);
        }

    }
}
