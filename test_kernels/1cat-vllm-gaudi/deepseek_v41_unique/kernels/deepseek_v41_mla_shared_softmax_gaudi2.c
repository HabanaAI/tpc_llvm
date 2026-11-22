// SPDX-License-Identifier: Apache-2.0
// The per-query membership mask preserves causality and the attention sink.
// Probability stays FP32 through the shared PV GEMM, as on the parent path.
void main(tensor logits,tensor members,tensor sink,tensor scale,
#ifdef DSV41_MLA_SHARED_COMPACT
          tensor prefix,
#endif
          tensor probabilities) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int heads=get_dim_size(sink,0);
#ifdef DSV41_MLA_SHARED_COMPACT
    const int actual=s_i32_ld_g(gen_addr((int5){get_dim_size(prefix,0)-1},prefix));
    const int chunks=(actual+63)/64;
#else
    const int chunks=get_dim_size(logits,0)/64;
#endif
    const float factor=s_f32_ld_g(gen_addr((int5){0},scale));
    for(int flat=begin[0];flat<end[0];++flat) {
        const int token=flat/heads,head=flat-token*heads;
        const float sink_value=s_f32_ld_g(gen_addr((int5){head},sink));
        // One local array fits Gaudi2's 16 KiB per-TPC vector memory. Reuse
        // its score storage for exponentials instead of spilling three arrays.
        float64 workspace[52];
        float64 maximum=sink_value;
        for(int chunk=0;chunk<chunks;++chunk) {
            const int64 mask=v_i32_ld_tnsr_b((int5){chunk*64},members);
            const bool64 valid=v_i32_cmp_neq_b(mask&(1<<token),0);
            float64 scores=v_f32_ld_tnsr_b((int5){chunk*64,flat},logits)*factor;
            scores=v_f32_mov_vb(scores,0,-3.402823466e+38f,valid,0);
            workspace[chunk]=scores;
            maximum=v_f32_max_b(maximum,scores);
        }
        maximum=v_f32_reduce_max(maximum);
        float64 total=0;
        for(int chunk=0;chunk<chunks;++chunk) {
            const int64 mask=v_i32_ld_tnsr_b((int5){chunk*64},members);
            const bool64 valid=v_i32_cmp_neq_b(mask&(1<<token),0);
            float64 exponential=v_exp_cephes_f32(workspace[chunk]-maximum);
            exponential=v_f32_mov_vb(exponential,0,0,valid,0);
            workspace[chunk]=exponential;
            total+=exponential;
        }
        total=v_f32_reduce_add(total)+v_exp_cephes_f32(sink_value-maximum);
        float64 inverse=v_reciprocal_f32(total);
        inverse=inverse*(2.0f-total*inverse);
        for(int chunk=0;chunk<chunks;++chunk)
            v_f32_st_tnsr((int5){chunk*64,flat},probabilities,workspace[chunk]*inverse);
    }
}
