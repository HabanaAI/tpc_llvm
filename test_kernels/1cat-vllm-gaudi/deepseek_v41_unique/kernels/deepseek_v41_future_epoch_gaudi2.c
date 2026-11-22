// SPDX-License-Identifier: Apache-2.0
// Transport generation belongs to the plan, never to KV rollback state.
// Advancing by two keeps the fixed-address NIC program on bank zero. The
// compute stream must serialize complete plans before reusing that bank.
void main(tensor input, tensor flags, tensor output) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    if(begin[0]==0) {
        const int last=get_dim_size(flags,0)-1;
        __global__ void* counter=gen_addr((int5){last},flags);
        cache_invalidate(SW_D);
        const int previous=s_i32_ld_g(counter);
        const bool restart=previous<0||previous>0x7ffffffa||(previous&1);
        if(restart)for(int slot=0;slot<256;++slot)
            s_i32_st_g(gen_addr((int5){slot},flags),0);
        s_i32_st_g(counter,restart?2:previous+2);
    }
    for(int block=begin[0];block<end[0];++block) {
        const int5 at={block*128};
        v_bf16_st_tnsr(at,output,v_bf16_ld_tnsr_b(at,input));
    }
}
