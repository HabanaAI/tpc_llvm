// SPDX-License-Identifier: Apache-2.0
// Normalize the FP32 BF16-PV accumulator at its existing BF16 output boundary.
void main(tensor product, tensor inverse_denominator, tensor output) {
    const int5 begin=get_index_space_offset(), end=begin+get_index_space_size();
    for(int token=begin[2];token<end[2];++token)
    for(int head=begin[1];head<end[1];++head) {
        const float inverse=s_f32_ld_g(gen_addr((int5){0,head,token},inverse_denominator));
        for(int tile=begin[0];tile<end[0];++tile) {
            const int n=tile*128;
            float128 values;
            values.v1=v_f32_ld_tnsr_b((int5){n,head,token},product)*inverse;
            values.v2=v_f32_ld_tnsr_b((int5){n+64,head,token},product)*inverse;
            v_bf16_st_tnsr((int5){n,head,token},output,
                convert_float128_to_bfloat128(values,SW_RHNE|SW_LINEAR));
        }
    }
}
