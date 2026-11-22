// SPDX-License-Identifier: Apache-2.0
// Exact production FP8 W2 rescale and BF16 boundary for one owner.
void main(tensor product,tensor metadata,tensor activation_scale,tensor channel,
          tensor output,int owner) {
    const int5 start=get_index_space_offset(),end=start+get_index_space_size();
    const int experts=get_dim_size(channel,2);
    const int expert=s_i32_ld_g(gen_addr((int5){owner,1,0,0,0},metadata));
    const int rows=s_i32_ld_g(gen_addr((int5){owner,3,0,0,0},metadata));
    if(expert<0 || expert>=experts) return;
    for(int row=0;row<rows;++row) {
        const float sx=s_f32_ld_g(gen_addr((int5){0,row,0,0,0},activation_scale));
        for(int block=start[0];block<end[0];++block) {
            const int n=block*64;
            const float64 acc=v_f32_ld_tnsr_b((int5){n,row,0,0,0},product);
            const uint64 bits=v_u32_ld_tnsr_b((int5){n%256,n/256,expert},channel,
                SW_UNPACK|SW_UNPCK_16_TO_32)<<16;
            const float64 scaled=v_f32_mul_b(v_f32_mul_b(acc,*((float64*)&bits)),sx);
            const float128 pair={scaled,(float64){0}};
            const bfloat128 rounded=convert_float128_to_bfloat128(pair,SW_RHNE|SW_LINEAR);
            v_bf16_st_tnsr_partial((int5){n,row,0,0,0},output,rounded,63,0);
        }
    }
}
