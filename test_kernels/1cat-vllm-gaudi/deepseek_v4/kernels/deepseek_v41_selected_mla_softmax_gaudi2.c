// SPDX-License-Identifier: Apache-2.0
// Sink contributes to the denominator once and has no value row. Probabilities
// remain FP32 through the PV matrix operation; padding is explicitly masked.
void main(tensor logits,
#ifdef DSV41_MLA_FLAT_QK
          tensor main_logits,
#endif
          tensor mask, tensor sink, tensor scale, tensor probabilities
#ifdef DSV41_MLA_PAIR_PROBABILITIES
          , tensor low_probabilities
#endif
#ifdef DSV41_MLA_EXP_BF16
          , tensor inverse_denominator
#endif
) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
#if defined(DSV41_MLA_EXP_BF16) || defined(DSV41_MLA_FLAT_QK)
    const int chunks = 10;  // Canonical 128 SWA + 512 selected rows.
#else
    const int chunks = get_dim_size(logits, 0) / 64;
#endif
    const float factor = s_f32_ld_g(gen_addr((int5){0}, scale));
    for (int token = start[1]; token < end[1]; ++token) {
    for (int head = start[0]; head < end[0]; ++head) {
        float64 scores[10], exponentials[10];
        bool64 valid[10];
        const float sink_value = s_f32_ld_g(gen_addr((int5){head, 0, 0, 0, 0}, sink));
        float64 maximum = sink_value;
#ifdef DSV41_MLA_EXP_BF16
        #pragma loop_unroll(10)
#endif
        for (int c = 0; c < chunks; ++c) {
#ifdef DSV41_MLA_FLAT_QK
#ifdef DSV41_MLA_FLAT_QK_SPLIT
            const int5 at = {c < 2 ? token * 128 + c * 64 : token * 512 + (c - 2) * 64,
                             head, token, 0, 0};
#else
            const int5 at = {token * 640 + c * 64, head, token, 0, 0};
#endif
#else
            const int5 at = {c * 64, head, token, 0, 0};
#endif
            const float64 enabled = v_f32_ld_tnsr_b((int5){c * 64, token, 0, 0, 0}, mask);
            valid[c] = v_f32_cmp_grt_b(enabled, 0.0f);
#ifdef DSV41_MLA_FLAT_QK_SPLIT
            scores[c] = (c < 2 ? v_f32_ld_tnsr_b(at, logits) : v_f32_ld_tnsr_b(at, main_logits)) * factor;
#else
            scores[c] = v_f32_ld_tnsr_b(at, logits) * factor;
#endif
            scores[c] = v_f32_mov_vb(scores[c], 0, -3.402823466e+38f, valid[c], 0);
            maximum = v_f32_max_b(maximum, scores[c]);
        }
        maximum = v_f32_reduce_max(maximum);
        float64 total = 0;
#ifdef DSV41_MLA_EXP_BF16
        #pragma loop_unroll(10)
#endif
        for (int c = 0; c < chunks; ++c) {
            exponentials[c] = v_exp_cephes_f32(scores[c] - maximum);
            exponentials[c] = v_f32_mov_vb(exponentials[c], 0, 0, valid[c], 0);
            total += exponentials[c];
        }
        total = v_f32_reduce_add(total) + v_exp_cephes_f32(sink_value - maximum);
        float64 inverse = v_reciprocal_f32(total);
        inverse = inverse * (2.0f - total * inverse);
#ifdef DSV41_MLA_EXP_BF16
        // The checkpoint's sparse attention feeds BF16 unnormalized
        // exponentials to PV, then normalizes its FP32 accumulated result.
        // Keep the stable sink-inclusive maximum and the existing FP32 sum.
        v_f32_st_tnsr_partial((int5){0, head, token}, inverse_denominator, inverse, 0, 0);
        #pragma loop_unroll(5)
        for (int c = 0; c < chunks; c += 2) {
            const float128 values = {exponentials[c], exponentials[c + 1]};
            v_bf16_st_tnsr((int5){c * 64, head, token}, probabilities,
                convert_float128_to_bfloat128(values, SW_RHNE | SW_LINEAR));
        }
#elif defined(DSV41_MLA_PAIR_PROBABILITIES) || defined(DSV41_MLA_STACKED_PROBABILITIES)
        for (int c = 0; c < chunks; c += 2) {
            float128 full;
            full.v1 = exponentials[c] * inverse;
            full.v2 = exponentials[c + 1] * inverse;
            const bfloat128 high = convert_float128_to_bfloat128(full, SW_RHNE | SW_LINEAR);
            const float128 rounded = convert_bfloat128_to_float128(high, SW_LINEAR);
            float128 residual;
            residual.v1 = full.v1 - rounded.v1;
            residual.v2 = full.v2 - rounded.v2;
            const bfloat128 low = convert_float128_to_bfloat128(residual, SW_RHNE | SW_LINEAR);
#ifdef DSV41_MLA_ADJACENT_PROBABILITIES
            v_bf16_st_tnsr((int5){c*64,head*2,token},probabilities,high);
            v_bf16_st_tnsr((int5){c*64,head*2+1,token},probabilities,low);
#else
            v_bf16_st_tnsr((int5){c * 64, head, token}, probabilities, high);
#ifdef DSV41_MLA_STACKED_PROBABILITIES
            v_bf16_st_tnsr((int5){c * 64, head + get_dim_size(logits, 1), token}, probabilities, low);
#else
            v_bf16_st_tnsr((int5){c * 64, head, token}, low_probabilities, low);
#endif
#endif
        }
#elif defined(DSV41_MLA_FP16_PROBABILITIES) && DSV41_MLA_FP16_PROBABILITIES
        for (int c = 0; c < chunks; c += 2) {
            const float128 full = {exponentials[c] * inverse, exponentials[c + 1] * inverse};
            v_f16_st_tnsr((int5){c * 64, head, token}, probabilities,
                convert_float128_to_half128(full, SW_RHNE | SW_LINEAR));
        }
#else
        for (int c = 0; c < chunks; ++c)
            v_f32_st_tnsr((int5){c * 64, head, token, 0, 0}, probabilities, exponentials[c] * inverse);
#endif
    }
    }
}
