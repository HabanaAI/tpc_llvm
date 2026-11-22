// SPDX-License-Identifier: Apache-2.0
// Decode ratio-2 compressor state update and pair reduction.  One index
// point owns 128 feature columns, so each history element is written exactly
// once and the two-token softmax is consumed without materializing four
// gathered rows or a probability tensor in HBM.
#pragma clang fp contract(off)

static inline float64 pair_mix(float64 current_value,
                               float64 other_value,
                               float64 current_score,
                               float64 other_score,
                               bool current_is_first)
{
    const float64 first_score = current_is_first ? current_score : other_score;
    const float64 second_score = current_is_first ? other_score : current_score;
    const float64 maximum = v_f32_max_b(first_score, second_score);
    const float64 first_exp = v_exp_cephes_f32(first_score - maximum);
    const float64 second_exp = v_exp_cephes_f32(second_score - maximum);
    const float64 inverse = v_reciprocal_f32(first_exp + second_exp);
    const float64 first_value = current_is_first ? current_value : other_value;
    const float64 second_value = current_is_first ? other_value : current_value;
    const float64 first_product = first_value * (first_exp * inverse);
    const float64 second_product = second_value * (second_exp * inverse);
    return first_product + second_product;
}

void main(tensor kv_history,
          tensor score_history,
          tensor kv,
          tensor score,
          tensor position,
          tensor latent)
{
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
#if defined(DSV41_COMPRESSOR_BATCH) && DSV41_COMPRESSOR_BATCH
    const int rows=get_dim_size(kv,1);
    const int base=s_i32_ld_g(gen_addr((int5){0},position));
    for(int token=begin[1];token<end[1];++token) {
    const int logical=s_i32_ld_g(gen_addr((int5){token},position));
#else
    const int token=0;
    const int logical = s_i32_ld_g(gen_addr((int5){0, 0, 0, 0, 0}, position));
#endif
    const int ring = logical & 7;
    const bool current_is_first = (logical & 1) == 0;
    const int other = current_is_first ? ((ring + 1) & 7) : ((ring - 1) & 7);

    for (int block = begin[0]; block < end[0]; ++block) {
        float128 result;
        #pragma unroll (2)
        for (int lane_half = 0; lane_half < 2; ++lane_half) {
            const int feature = block * 128 + lane_half * 64;
            const int5 current_at = {feature, token, 0, 0, 0};
            const int5 ring_at = {feature, ring, 0, 0, 0};
            const int5 other_at = {feature, other, 0, 0, 0};
            const float64 current_value = v_f32_ld_tnsr_b(current_at, kv);
            const float64 current_score = v_f32_ld_tnsr_b(current_at, score);
#if defined(DSV41_COMPRESSOR_BATCH) && DSV41_COMPRESSOR_BATCH
            // The qualified C<=6 transaction writes all rows before mixing.
            // Forward a present partner directly; only boundary partners read
            // old ring state. No cross-TPC store/load dependency or barrier.
            const int partner=logical+(current_is_first?1:-1)-base;
            const bool present=partner>=0 && partner<rows;
            const int5 partner_at={feature,partner,0,0,0};
            const float64 other_value=present?v_f32_ld_tnsr_b(partner_at,kv):v_f32_ld_tnsr_b(other_at,kv_history);
            const float64 other_score=present?v_f32_ld_tnsr_b(partner_at,score):v_f32_ld_tnsr_b(other_at,score_history);
#else
            const float64 other_value = v_f32_ld_tnsr_b(other_at, kv_history);
            const float64 other_score = v_f32_ld_tnsr_b(other_at, score_history);
#endif

            // Persist the current row after loading the distinct partner row.
            // The current value is forwarded from registers into the pair
            // reduction, avoiding a same-kernel store/load dependency.
            v_f32_st_tnsr(ring_at, kv_history, current_value);
            v_f32_st_tnsr(ring_at, score_history, current_score);
            const float64 mixed = pair_mix(current_value, other_value,
                                           current_score, other_score,
                                           current_is_first);
            if (lane_half == 0) result.v1 = mixed;
            else result.v2 = mixed;
        }
        v_bf16_st_tnsr((int5){block * 128, token, 0, 0, 0}, latent,
                       convert_float128_to_bfloat128(result,
                                                     SW_RHNE | SW_LINEAR));
    }
#if defined(DSV41_COMPRESSOR_BATCH) && DSV41_COMPRESSOR_BATCH
    }
#endif
}
