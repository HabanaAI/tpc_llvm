// SPDX-License-Identifier: Apache-2.0
// The input has adjacent high/low probability products for every logical head.
void main(tensor product,tensor output) {
    const int5 first=get_index_space_offset(),end=first+get_index_space_size();
    for(int token=first[2];token<end[2];++token)
    for(int head=first[1];head<end[1];++head)
    for(int tile=first[0];tile<end[0];++tile) {
        const int5 a={tile*128,2*head,token},b={tile*128,2*head+1,token};
        int5 ah=a,bh=b;ah[0]+=64;bh[0]+=64;
        float128 sum;
        sum.v1=v_f32_ld_tnsr_b(a,product)+v_f32_ld_tnsr_b(b,product);
        sum.v2=v_f32_ld_tnsr_b(ah,product)+v_f32_ld_tnsr_b(bh,product);
        v_bf16_st_tnsr((int5){tile*128,head,token},output,
            convert_float128_to_bfloat128(sum,SW_RHNE|SW_LINEAR));
    }
}
