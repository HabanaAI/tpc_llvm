// SPDX-License-Identifier: Apache-2.0
void main(tensor high, tensor low, tensor result) {
    const int5 first = get_index_space_offset();
    const int5 last = first + get_index_space_size();
    for(int token=first[2];token<last[2];++token)
    for(int head=first[1];head<last[1];++head)
    for(int tile=first[0];tile<last[0];++tile) {
        const int5 at={tile*128,head,token};
        int5 upper=at;upper[0]+=64;
        float128 value;
        value.v1=v_f32_ld_tnsr_b(at,high)+v_f32_ld_tnsr_b(at,low);
        value.v2=v_f32_ld_tnsr_b(upper,high)+v_f32_ld_tnsr_b(upper,low);
        v_bf16_st_tnsr(at,result,convert_float128_to_bfloat128(value,SW_RHNE|SW_LINEAR));
    }
}
