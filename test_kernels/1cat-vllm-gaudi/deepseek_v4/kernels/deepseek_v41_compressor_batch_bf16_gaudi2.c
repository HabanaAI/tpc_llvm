// SPDX-License-Identifier: Apache-2.0
// Request-owned extension of the qualified C1 pair codec.
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

void main(tensor kv_history, tensor score_history, tensor kv, tensor score,
          tensor positions, tensor slots, tensor latent) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    const int capacity = get_dim_size(kv_history, 1) / 8;
    for (int request = begin[1]; request < end[1]; ++request) {
        const int logical = s_i32_ld_g(gen_addr((int5){request}, positions));
        const int owner = s_i32_ld_g(gen_addr((int5){request}, slots));
        const bool active = logical >= 0 && owner >= 0 && owner < capacity;
        const int ring = owner * 8 + (logical & 7);
        const bool first = (logical & 1) == 0;
        const int other = owner * 8 + ((logical ^ 1) & 7);
        for (int block = begin[0]; block < end[0]; ++block) {
            if (!active) {
                v_bf16_st_tnsr((int5){block * 128, request}, latent, (bfloat128)0);
                continue;
            }
            float128 result;
            #pragma unroll (2)
            for (int part = 0; part < 2; ++part) {
                const int feature = block * 128 + part * 64;
                const float64 current_value = v_f32_ld_tnsr_b((int5){feature, request}, kv);
                const float64 current_score = v_f32_ld_tnsr_b((int5){feature, request}, score);
                const float64 other_value = v_f32_ld_tnsr_b((int5){feature, other}, kv_history);
                const float64 other_score = v_f32_ld_tnsr_b((int5){feature, other}, score_history);
                v_f32_st_tnsr((int5){feature, ring}, kv_history, current_value);
                v_f32_st_tnsr((int5){feature, ring}, score_history, current_score);
                const float64 mixed = pair_mix(current_value, other_value, current_score, other_score, first);
                if (part == 0) result.v1 = mixed;
                else result.v2 = mixed;
            }
            v_bf16_st_tnsr((int5){block * 128, request}, latent,
                convert_float128_to_bfloat128(result, SW_RHNE | SW_LINEAR));
        }
    }
}
