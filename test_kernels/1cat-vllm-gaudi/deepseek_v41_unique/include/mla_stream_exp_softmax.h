// SPDX-License-Identifier: Apache-2.0
// C1 sink-inclusive FP32 reduction; BF16 exponentials feed one PV per bank.
// Two passes avoid materializing ten score/exponential vectors in local RAM.
static inline float64 stream_score(tensor first, tensor second,
                                   int chunk, int head, int token, float factor,
                                   bool64 valid) {
#ifdef DSV41_STREAM_EXP_SPLIT
    const int5 at={chunk<2?chunk*64:(chunk-2)*64,head,token};
    const float64 score=(chunk<2?v_f32_ld_tnsr_b(at,first):v_f32_ld_tnsr_b(at,second))*factor;
#else
    const float64 score=v_f32_ld_tnsr_b((int5){chunk*64,head,token},first)*factor;
#endif
    return v_f32_mov_vb(score,0,-3.402823466e+38f,valid,0);
}
void main(tensor first,tensor second,tensor mask,tensor sink,tensor scale,
          tensor first_exp,
#ifdef DSV41_STREAM_EXP_SPLIT
          tensor second_exp,
#endif
          tensor inverse_denominator) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const float factor=s_f32_ld_g(gen_addr((int5){0},scale));
    for(int head=begin[0];head<end[0];++head) {
        const float sink_value=s_f32_ld_g(gen_addr((int5){head},sink));
        for(int token=begin[1];token<end[1];++token) {
            float64 maximum=sink_value;
            for(int chunk=0;chunk<10;++chunk) {
                const bool64 valid=v_f32_cmp_grt_b(
                    v_f32_ld_tnsr_b((int5){chunk*64,token},mask),0.f);
                maximum=v_f32_max_b(maximum,stream_score(first,second,chunk,head,token,factor,valid));
            }
            maximum=v_f32_reduce_max(maximum);
            float64 total=0;
            for(int chunk=0;chunk<10;chunk+=2) {
                const bool64 va=v_f32_cmp_grt_b(v_f32_ld_tnsr_b((int5){chunk*64,token},mask),0.f);
                const bool64 vb=v_f32_cmp_grt_b(v_f32_ld_tnsr_b((int5){(chunk+1)*64,token},mask),0.f);
                float64 ea=v_exp_cephes_f32(stream_score(first,second,chunk,head,token,factor,va)-maximum);
                float64 eb=v_exp_cephes_f32(stream_score(first,second,chunk+1,head,token,factor,vb)-maximum);
                ea=v_f32_mov_vb(ea,0,0,va,0);eb=v_f32_mov_vb(eb,0,0,vb,0);
                // Preserve the original c0,c1,... FP32 accumulation order.
                total+=ea;total+=eb;
                const float128 pair={ea,eb};
                const bfloat128 rounded=convert_float128_to_bfloat128(pair,SW_RHNE|SW_LINEAR);
#ifdef DSV41_STREAM_EXP_SPLIT
                if(chunk<2)v_bf16_st_tnsr((int5){chunk*64,head,token},first_exp,rounded);
                else v_bf16_st_tnsr((int5){(chunk-2)*64,head,token},second_exp,rounded);
#else
                v_bf16_st_tnsr((int5){chunk*64,head,token},first_exp,rounded);
#endif
            }
            total=v_f32_reduce_add(total)+v_exp_cephes_f32(sink_value-maximum);
            float64 inverse=v_reciprocal_f32(total);
            inverse=inverse*(2.f-total*inverse);
            v_f32_st_tnsr_partial((int5){0,head,token},inverse_denominator,inverse,0,0);
        }
    }
}
