// SPDX-License-Identifier: Apache-2.0
// Prompt pre-mix collapse.  Keep the eager contract's separate FP32 products
// and source order, then perform the single BF16 rounding at the output.
void main(tensor residual, tensor previous_pre, tensor output)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int token = start[1]; token < end[1]; ++token) {
        float weight[4];
        #pragma unroll
        for (int source = 0; source < 4; ++source) {
            int5 pc = {source, token, 0, 0, 0};
            weight[source] = s_f32_ld_g(gen_addr(pc, previous_pre));
        }
        for (int block = start[0]; block < end[0]; ++block) {
            const int feature = block * 128;
            float128 value[4];
            #pragma unroll
            for (int source = 0; source < 4; ++source) {
                int5 rc = {feature, source, token, 0, 0};
                value[source] = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc, residual));
            }
            float128 collapsed;
            collapsed.v1 = v_f32_mul_b(value[0].v1, weight[0]);
            collapsed.v2 = v_f32_mul_b(value[0].v2, weight[0]);
            #pragma unroll
            for (int source = 1; source < 4; ++source) {
                const float64 lo = v_f32_mul_b(value[source].v1, weight[source]);
                const float64 hi = v_f32_mul_b(value[source].v2, weight[source]);
                collapsed.v1 = v_f32_add_b(collapsed.v1, lo);
                collapsed.v2 = v_f32_add_b(collapsed.v2, hi);
            }
            int5 oc = {feature, token, 0, 0, 0};
            v_bf16_st_tnsr(oc, output, v_convert_f32_to_bf16_all_b(collapsed, SW_RHNE));
        }
    }
}
