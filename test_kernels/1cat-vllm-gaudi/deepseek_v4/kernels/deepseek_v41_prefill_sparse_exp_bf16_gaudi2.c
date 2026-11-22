// SPDX-License-Identifier: Apache-2.0
// FP32 logits/denominator and BF16 MME probabilities, per token and head.
// The sink contributes to the denominator only, matching sparse MLA.
void main(tensor logits, tensor mask, tensor sink, tensor scale,
          tensor probabilities, tensor inverse_denominator) {
    const int5 first = get_index_space_offset();
    const int5 end = first + get_index_space_size();
    const int chunks = get_dim_size(logits, 0) / 64;
    const float factor = s_f32_ld_g(gen_addr((int5){0}, scale));
    for (int token = first[1]; token < end[1]; ++token) {
        for (int head = first[0]; head < end[0]; ++head) {
            float64 scores[10], exps[10];
            bool64 valid[10];
            const float sink_value = s_f32_ld_g(gen_addr((int5){head}, sink));
            float64 maximum = sink_value;
            for (int c = 0; c < chunks; ++c) {
                const int5 score_at = {c * 64, head, token};
                const int5 mask_at = {c * 64, token};
                valid[c] = v_f32_cmp_grt_b(v_f32_ld_tnsr_b(mask_at, mask), 0.0f);
                scores[c] = v_f32_ld_tnsr_b(score_at, logits) * factor;
                scores[c] = v_f32_mov_vb(scores[c], 0, -3.402823466e+38f, valid[c], 0);
                maximum = v_f32_max_b(maximum, scores[c]);
            }
            maximum = v_f32_reduce_max(maximum);
            float64 total = 0;
            for (int c = 0; c < chunks; ++c) {
                exps[c] = v_exp_cephes_f32(scores[c] - maximum);
                exps[c] = v_f32_mov_vb(exps[c], 0, 0, valid[c], 0);
                total += exps[c];
            }
            total = v_f32_reduce_add(total) + v_exp_cephes_f32(sink_value - maximum);
            float64 inverse = v_reciprocal_f32(total);
            inverse = inverse * (2.0f - total * inverse);
            v_f32_st_tnsr_partial((int5){0, head, token}, inverse_denominator, inverse, 0, 0);
            for (int c = 0; c < chunks; ++c) {
                float128 wide = {0};
                wide.v1 = exps[c];
                v_bf16_st_tnsr_partial((int5){c * 64, head, token}, probabilities,
                                      convert_float128_to_bfloat128(wide, SW_LINEAR), 63, 0);
            }
        }
    }
}
