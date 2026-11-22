// SPDX-License-Identifier: Apache-2.0
// Keep the C1 FP32 softmax and PV arithmetic, with one shared SWA operand.
void main(tensor swa_scores,tensor main_scores,tensor main_mask,tensor sink,tensor scale,
          tensor positions,tensor lengths,tensor swa_probability,tensor main_probability) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const float factor=s_f32_ld_g(gen_addr((int5){0},scale));
    const int first=s_i32_ld_g(gen_addr((int5){0},positions));
    const int tokens=get_dim_size(positions,0);
    const int64 lanes=convert_uint64_to_int64(V_LANE_ID_32,0);
    for(int token=begin[1];token<end[1];++token) {
        const int delta=s_i32_ld_g(gen_addr((int5){token},positions))-first;
        const int length=s_i32_ld_g(gen_addr((int5){token},lengths));
        for(int head=begin[0];head<end[0];++head) {
            float64 scores[11],exponentials[11];bool64 valid[11];
            const float sink_value=s_f32_ld_g(gen_addr((int5){head},sink));
            float64 maximum=sink_value;
            for(int c=0;c<11;++c) {
                if(c<3) {
                    const int64 slot=lanes+c*64;
                    valid[c]=v_i32_cmp_geq_b(slot,delta)&v_i32_cmp_less_b(slot,delta+128)&
                        v_i32_cmp_less_b(slot,128+tokens-1)&v_i32_cmp_geq_b(slot,127-first)&
                        v_i32_cmp_less_b(slot,delta+length);
                    scores[c]=v_f32_ld_tnsr_b((int5){c*64,head,token},swa_scores)*factor;
                } else {
                    const int offset=(c-3)*64;
                    valid[c]=v_f32_cmp_grt_b(v_f32_ld_tnsr_b((int5){offset+128,token},main_mask),0.f);
                    scores[c]=v_f32_ld_tnsr_b((int5){offset,head,token},main_scores)*factor;
                }
                scores[c]=v_f32_mov_vb(scores[c],0,-3.402823466e+38f,valid[c],0);
                maximum=v_f32_max_b(maximum,scores[c]);
            }
            maximum=v_f32_reduce_max(maximum);
            float64 total=0;
            for(int c=0;c<11;++c) {
                exponentials[c]=v_exp_cephes_f32(scores[c]-maximum);
                exponentials[c]=v_f32_mov_vb(exponentials[c],0,0,valid[c],0);
                total+=exponentials[c];
            }
            total=v_f32_reduce_add(total)+v_exp_cephes_f32(sink_value-maximum);
            float64 inverse=v_reciprocal_f32(total);
            inverse=inverse*(2.f-total*inverse);
            for(int c=0;c<11;++c) {
                const int5 at={c<3?c*64:(c-3)*64,head,token};
                if(c<3) {
#ifdef DSV41_COHERENT_SWA_BF16_PV
                    float128 full={0};full.v1=exponentials[c]*inverse;
                    const bfloat128 high=convert_float128_to_bfloat128(full,SW_RHNE|SW_LINEAR);
                    const float128 restored=convert_bfloat128_to_float128(high,SW_LINEAR);
                    float128 residual={0};residual.v1=full.v1-restored.v1;
                    const bfloat128 low=convert_float128_to_bfloat128(residual,SW_RHNE|SW_LINEAR);
                    v_bf16_st_tnsr_partial((int5){c*64,head*2,token},swa_probability,high,63,0);
                    v_bf16_st_tnsr_partial((int5){c*64,head*2+1,token},swa_probability,low,63,0);
#else
                    v_f32_st_tnsr(at,swa_probability,exponentials[c]*inverse);
#endif
                } else v_f32_st_tnsr(at,main_probability,exponentials[c]*inverse);
            }
        }
    }
}
