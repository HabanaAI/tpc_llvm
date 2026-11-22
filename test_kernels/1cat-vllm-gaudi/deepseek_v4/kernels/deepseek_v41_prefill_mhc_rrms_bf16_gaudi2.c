// SPDX-License-Identifier: Apache-2.0
// Read the BF16 residual once and reduce a complete token row in registers.
// The result matches the mHC contract shape while avoiding the 671 MiB FP32
// flattened tensor and square tensor used by the generic eager chain.
void main(tensor residual, tensor output)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int token = start[0]; token < end[0]; ++token) {
        float64 sum_lo = 0.0f;
        float64 sum_hi = 0.0f;
        #pragma unroll 1
        for (int source = 0; source < 4; ++source) {
            for (int feature = 0; feature < 5120; feature += 128) {
                const int5 at = {feature, source, token, 0, 0};
                const float128 value = v_convert_bf16_to_f32_all_b(
                    v_bf16_ld_tnsr_b(at, residual));
                sum_lo = v_f32_mac_b(value.v1, value.v1, sum_lo);
                sum_hi = v_f32_mac_b(value.v2, value.v2, sum_hi);
            }
        }
        const float64 total = v_f32_reduce_add(sum_lo + sum_hi);
        const float64 rrms = v_rsqrt_f32(total / 20480.0f + 1.0e-20f);
        const int5 out = {0, token, 0, 0, 0};
        v_f32_st_tnsr_partial(out, output, rrms, 0, 0);
    }
}
