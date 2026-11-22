// SPDX-License-Identifier: Apache-2.0
// Sink contributes to the denominator once and has no value row. Probabilities
// remain FP32 through the PV matrix operation; padding is explicitly masked.
void main(tensor logits, tensor mask, tensor sink, tensor scale, tensor probabilities
#ifdef DSV41_MLA_BF16_PV
          , tensor inverse_denominator
#endif
          ) {
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    const int chunks = get_dim_size(logits, 0) / 64;
    const float factor = s_f32_ld_g(gen_addr((int5){0}, scale));
    for (int head = start[0]; head < end[0]; ++head) {
        float64 scores[10], exponentials[10];
        bool64 valid[10];
        const float sink_value = s_f32_ld_g(gen_addr((int5){head, 0, 0, 0, 0}, sink));
        float64 maximum = sink_value;
        for (int c = 0; c < chunks; ++c) {
            const int5 at = {c * 64, head, 0, 0, 0};
            const float64 enabled = v_f32_ld_tnsr_b((int5){c * 64, 0, 0, 0, 0}, mask);
            valid[c] = v_f32_cmp_grt_b(enabled, 0.0f);
            scores[c] = v_f32_ld_tnsr_b(at, logits) * factor;
            scores[c] = v_f32_mov_vb(scores[c], 0, -3.402823466e+38f, valid[c], 0);
            maximum = v_f32_max_b(maximum, scores[c]);
        }
        maximum = v_f32_reduce_max(maximum);
        float64 total = 0;
        for (int c = 0; c < chunks; ++c) {
            exponentials[c] = v_exp_cephes_f32(scores[c] - maximum);
            exponentials[c] = v_f32_mov_vb(exponentials[c], 0, 0, valid[c], 0);
            total += exponentials[c];
        }
        total = v_f32_reduce_add(total) + v_exp_cephes_f32(sink_value - maximum);
        float64 inverse = v_reciprocal_f32(total);
        inverse = inverse * (2.0f - total * inverse);
#ifdef DSV41_MLA_BF16_PV
        v_f32_st_tnsr_partial((int5){0, head, 0, 0, 0}, inverse_denominator, inverse, 0, 0);
        for (int c = 0; c < chunks; ++c) {
            float128 wide = {0}; wide.v1 = exponentials[c];
            v_bf16_st_tnsr_partial((int5){c * 64, head, 0, 0, 0}, probabilities,
                                  convert_float128_to_bfloat128(wide, SW_LINEAR), 63, 0);
        }
#else
        for (int c = 0; c < chunks; ++c)
            v_f32_st_tnsr((int5){c * 64, head, 0, 0, 0}, probabilities, exponentials[c] * inverse);
#endif
    }
}
