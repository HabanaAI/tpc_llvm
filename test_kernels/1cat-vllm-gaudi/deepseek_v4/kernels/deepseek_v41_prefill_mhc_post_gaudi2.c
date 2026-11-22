// SPDX-License-Identifier: Apache-2.0
// Prompt residual update. The eager reference rounds every FP32 multiply
// and sums source streams in order 0,1,2,3 before adding the projected value.
void main(tensor value, tensor residual, tensor post, tensor comb, tensor output)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int token = start[1]; token < end[1]; ++token) {
        float weights[4][4], gains[4];
        #pragma unroll
        for (int target = 0; target < 4; ++target) {
            gains[target] = s_f32_ld_g(gen_addr((int5){target, token, 0, 0, 0}, post));
            #pragma unroll
            for (int source = 0; source < 4; ++source) {
                weights[target][source] = s_f32_ld_g(gen_addr((int5){target, source, token, 0, 0}, comb));
            }
        }
        for (int block = start[0]; block < end[0]; ++block) {
            const int feature = block * 128;
            int5 vc = {feature, token, 0, 0, 0};
            const float128 x = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(vc, value));
            float128 r[4];
            #pragma unroll
            for (int source = 0; source < 4; ++source) {
                int5 rc = {feature, source, token, 0, 0};
                r[source] = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc, residual));
            }
            #pragma unroll
            for (int target = 0; target < 4; ++target) {
                float128 mixed;
                mixed.v1 = v_f32_mul_b(r[0].v1, weights[target][0]);
                mixed.v2 = v_f32_mul_b(r[0].v2, weights[target][0]);
                #pragma unroll
                for (int source = 1; source < 4; ++source) {
                    const float weight = weights[target][source];
                    const float64 lo = v_f32_mul_b(r[source].v1, weight);
                    const float64 hi = v_f32_mul_b(r[source].v2, weight);
                    mixed.v1 = v_f32_add_b(mixed.v1, lo);
                    mixed.v2 = v_f32_add_b(mixed.v2, hi);
                }
                const float gain = gains[target];
                const float64 lo = v_f32_mul_b(x.v1, gain);
                const float64 hi = v_f32_mul_b(x.v2, gain);
                mixed.v1 = v_f32_add_b(lo, mixed.v1);
                mixed.v2 = v_f32_add_b(hi, mixed.v2);
                int5 oc = {feature, target, token, 0, 0};
                v_bf16_st_tnsr(oc, output, v_convert_f32_to_bf16_all_b(mixed, SW_RHNE));
            }
        }
    }
}
