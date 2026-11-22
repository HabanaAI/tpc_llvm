// SPDX-License-Identifier: Apache-2.0
// Compute gates in registers at each residual feature owner, then perform
// ordered post/collapse without materializing gate tensors. FFN norm remains
// independent of the controller MME. No inter-workpoint barrier is required.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"
#ifndef DSV41_MHC_POST_FEATURE_TILE
#define DSV41_MHC_POST_FEATURE_TILE 1
#endif
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

static inline float64 load_mix(tensor mixes, int feature, int token, int last) {
    const float64 high=v_f32_ld_tnsr_partial_b((int5){feature, token, 0, 0, 0}, mixes, last, 0);
    if (get_dim_size(mixes,0)==25) return high;
    return high + v_f32_ld_tnsr_partial_b((int5){feature + 24, token, 0, 0, 0}, mixes, last, 0);
}
typedef struct {float64 pre;float64 post;float64 comb;} GateRegisters;
static inline GateRegisters compute_gates(tensor raw_mixes, tensor residual,
                                tensor hc_scale, tensor hc_base,
                                int token, float epsilon) {
    const uint64 lane = read_lane_id_4b_b();
    const uchar256 r0 = matrix_direction(lane & 0xfffffffc);
    const uchar256 r1 = matrix_direction((lane & 0xfffffffc) | 1);
    const uchar256 r2 = matrix_direction((lane & 0xfffffffc) | 2);
    const uchar256 r3 = matrix_direction((lane & 0xfffffffc) | 3);
    const uchar256 c0 = matrix_direction(lane & 3);
    const uchar256 c1 = matrix_direction((lane & 3) | 4);
    const uchar256 c2 = matrix_direction((lane & 3) | 8);
    const uchar256 c3 = matrix_direction((lane & 3) | 12);

        float64 inverse_rms;
        if (get_dim_size(raw_mixes,0)==25) {
            // The exact FP32 control producer already computed this statistic.
            // Carry its bits instead of repeating the 20480-value reduction
            // at every post owner or changing the controller's sum order.
            inverse_rms=s_f32_ld_g(gen_addr((int5){24,token},raw_mixes));
        } else {
        float64_pair_t squares = {0};
        for (int k = 0; k < 20480; k += 128) {
            const float64_pair_t x = mhc_bf16_to_f32_linear(
                v_bf16_ld_tnsr_b((int5){k%5120,k/5120,token,0,0}, residual));
            squares.v1 = v_f32_mac_b(x.v1, x.v1, squares.v1);
            squares.v2 = v_f32_mac_b(x.v2, x.v2, squares.v2);
        }
        inverse_rms = positive_rsqrt(
            v_f32_reduce_add(squares.v1 + squares.v2) * (1.0f / 20480.0f) + epsilon);
        }
        const float64 pre_scale = s_f32_ld_g(gen_addr((int5){0}, hc_scale));
        const float64 post_scale = s_f32_ld_g(gen_addr((int5){1, 0, 0, 0, 0}, hc_scale));
        const float64 comb_scale = s_f32_ld_g(gen_addr((int5){2, 0, 0, 0, 0}, hc_scale));

        const float64 pre_raw = load_mix(raw_mixes, 0, token, 3);
        const float64 pre_base = v_f32_ld_tnsr_partial_b((int5){0}, hc_base, 3, 0);
        const float64 pre = sigmoid_f32(pre_raw * inverse_rms * pre_scale + pre_base) + HC_EPS;
        

        const float64 post_raw = load_mix(raw_mixes, 4, token, 3);
        const float64 post_base = v_f32_ld_tnsr_partial_b(
            (int5){4, 0, 0, 0, 0}, hc_base, 3, 0);
        const float64 post = sigmoid_f32(post_raw * inverse_rms * post_scale + post_base) * 2.0f;
        

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
        return (GateRegisters){pre,post,values};
}

static inline float64 broadcast_gate(float64 values,int lane) {
    // SHUFFLE selects into dual group zero. Replicate the selected group
    // after the shuffle, as in the upstream TPC scale broadcast example.
    float64 copied=v_f32_shuffle_b(values,(uchar256)(0x80 | (lane&7) | ((lane&8)<<2)),0,values);
    copied=v_f32_mov_dual_group_b(copied,0xffffffff,0,1,MkWr(1,1),copied);
    copied=v_f32_mov_dual_group_b(copied,0xffffffff,0,2,MkWr(1,1),copied);
    return v_f32_mov_dual_group_b(copied,0xffffffff,0,3,MkWr(1,1),copied);
}
void main(tensor value,tensor residual,tensor raw,tensor hc_scale,tensor hc_base,
          tensor residual_out,tensor collapsed_out,tensor next_pre,float epsilon) {
    const int5 start=get_index_space_offset(),end=start+get_index_space_size();
    for(int token=start[1];token<end[1];++token) {
        const GateRegisters gates=compute_gates(raw,residual,hc_scale,hc_base,token,epsilon);
        if(start[0]==0) {
            v_f32_st_tnsr_partial((int5){0,token},next_pre,gates.pre,3,0);
            v_f32_st_tnsr_partial((int5){4,token},next_pre,gates.post,3,0);
            v_f32_st_tnsr_partial((int5){8,token},next_pre,gates.comb,15,0);
        }
        for(int block=start[0]*DSV41_MHC_POST_FEATURE_TILE;
            block<end[0]*DSV41_MHC_POST_FEATURE_TILE;++block) {
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
                    source_value[0].v1, broadcast_gate(gates.comb,target));
                updated[target].v2 = v_f32_mul_b(
                    source_value[0].v2, broadcast_gate(gates.comb,target));
                #pragma unroll
                for (int source = 1; source < 4; ++source) {
                    const float64 lo = v_f32_mul_b(
                        source_value[source].v1, broadcast_gate(gates.comb,source*4+target));
                    const float64 hi = v_f32_mul_b(
                        source_value[source].v2, broadcast_gate(gates.comb,source*4+target));
                    updated[target].v1 = v_f32_add_b(updated[target].v1, lo);
                    updated[target].v2 = v_f32_add_b(updated[target].v2, hi);
                }
                // The deployed decode fuser contracts this final product
                // and add. Keep its single FP32 rounding before BF16; a
                // separate multiply/add changes exact BF16 midpoint cases.
                updated[target].v1 = v_f32_mac_b(
                    x.v1, broadcast_gate(gates.post,target), updated[target].v1);
                updated[target].v2 = v_f32_mac_b(
                    x.v2, broadcast_gate(gates.post,target), updated[target].v2);
                const bfloat128 rounded = v_convert_f32_to_bf16_all_b(
                    updated[target], SW_RHNE);
                int5 oc = {feature, target, token, 0, 0};
                v_bf16_st_tnsr(oc, residual_out, rounded);
                updated[target] = v_convert_bf16_to_f32_all_b(rounded);
            }

            float128 collapsed;
            collapsed.v1 = v_f32_mul_b(updated[0].v1, broadcast_gate(gates.pre,0));
            collapsed.v2 = v_f32_mul_b(updated[0].v2, broadcast_gate(gates.pre,0));
            #pragma unroll
            for (int target = 1; target < 4; ++target) {
                const float64 lo = v_f32_mul_b(
                    updated[target].v1, broadcast_gate(gates.pre,target));
                const float64 hi = v_f32_mul_b(
                    updated[target].v2, broadcast_gate(gates.pre,target));
                collapsed.v1 = v_f32_add_b(collapsed.v1, lo);
                collapsed.v2 = v_f32_add_b(collapsed.v2, hi);
            }
            int5 cc = {feature, token, 0, 0, 0};
            v_bf16_st_tnsr(cc, collapsed_out,
                           v_convert_f32_to_bf16_all_b(collapsed, SW_RHNE));
        }
    }
}
