// SPDX-License-Identifier: Apache-2.0
// Three streaming passes; no dynamic vector workspace, unchanged FP32 probability.
#pragma clang fp contract(off)
void main(tensor logits,tensor members,tensor sink,tensor scale,tensor prefix,tensor probabilities) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int heads=get_dim_size(sink,0),capacity=get_dim_size(logits,0);
    const int actual=s_i32_ld_g(gen_addr((int5){get_dim_size(prefix,0)-1},prefix));
    const int chunks=(actual+63)/64;
    const int64 lanes=(int64)V_LANE_ID_32;
    const float factor=s_f32_ld_g(gen_addr((int5){0},scale));
    for(int flat=begin[0];flat<end[0];++flat) {
        const int token=flat/heads,head=flat-token*heads;
        const float sink_value=s_f32_ld_g(gen_addr((int5){head},sink));
        float64 maximum=sink_value;
        for(int chunk=0;chunk<chunks;++chunk) {
            const int64 mask=v_i32_ld_tnsr_b((int5){chunk*64},members);
            const bool64 valid=v_i32_cmp_neq_b(mask&(1<<token),0)&v_i32_cmp_less_b(lanes+chunk*64,actual);
            float64 scores=v_f32_ld_tnsr_b((int5){chunk*64,flat},logits)*factor;
            maximum=v_f32_max_b(maximum,v_f32_mov_vb(scores,0,-3.402823466e+38f,valid,0));
        }
        maximum=v_f32_reduce_max(maximum);
        float64 total=0;
        for(int chunk=0;chunk<chunks;++chunk) {
            const int64 mask=v_i32_ld_tnsr_b((int5){chunk*64},members);
            const bool64 valid=v_i32_cmp_neq_b(mask&(1<<token),0)&v_i32_cmp_less_b(lanes+chunk*64,actual);
            const float64 scores=v_f32_ld_tnsr_b((int5){chunk*64,flat},logits)*factor;
            const float64 exponential=v_exp_cephes_f32(scores-maximum);
            total+=v_f32_mov_vb(exponential,0,0,valid,0);
        }
        total=v_f32_reduce_add(total)+v_exp_cephes_f32(sink_value-maximum);
        float64 inverse=v_reciprocal_f32(total);
        inverse=inverse*(2.0f-total*inverse);
        for(int chunk=0;chunk*64<capacity;++chunk) {
            float64 value=0;
            if(chunk<chunks) {
                const int64 mask=v_i32_ld_tnsr_b((int5){chunk*64},members);
                const bool64 valid=v_i32_cmp_neq_b(mask&(1<<token),0)&v_i32_cmp_less_b(lanes+chunk*64,actual);
                const float64 scores=v_f32_ld_tnsr_b((int5){chunk*64,flat},logits)*factor;
                value=v_f32_mov_vb(v_exp_cephes_f32(scores-maximum),0,0,valid,0)*inverse;
            }
            v_f32_st_tnsr((int5){chunk*64,flat},probabilities,value);
        }
    }
}
