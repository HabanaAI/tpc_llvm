// SPDX-License-Identifier: Apache-2.0
// Capability only. The receiver is never selected by the serving defaults.
// Input/flag tables must remain in HBM until the independent DMA publishes
// epoch; verify compiled placement before any communication timing.
#pragma clang fp contract(off)
#ifndef DSV41_RECEIVE_POLL_ONCE
#define DSV41_RECEIVE_POLL_ONCE 0
#endif
void main(tensor local, tensor received, tensor flags, tensor epoch,
          tensor output, tensor status, int point, int rank, int maximum_spins) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int epoch_index=get_dim_size(epoch,0)==1?0:get_dim_size(epoch,0)-1;
    cache_invalidate(SW_D);
    const int expected=s_i32_ld_g(gen_addr((int5){epoch_index},epoch));
    const int parity=expected&1;
    const int group=get_dim_size(received,2);
    __global__ void* ready_addr=gen_addr((int5){point*2+parity},flags);
#if DSV41_RECEIVE_POLL_ONCE
    // One completion covers the entire point. The NIC payload is immutable
    // until this recipe retires; the same acquire therefore covers all tiles
    // owned by this invocation. Keep the installed per-tile variant default.
    int ready=0, used_spins=0;
    for(int spin=0;spin<maximum_spins;++spin) {
        cache_invalidate(SW_D);
        ready=s_i32_ld_g(ready_addr);
        used_spins=spin+1;
        if(ready==expected)break;
    }
    const bool valid=ready==expected;
#endif
    for(int row=begin[1];row<end[1];++row)for(int tile=begin[0];tile<end[0];++tile) {
#if !DSV41_RECEIVE_POLL_ONCE
        int ready=0, used_spins=0;
        for(int spin=0;spin<maximum_spins;++spin) {
            // Scalar global loads cache values. Gaudi2 requires explicit SW_D:
            // the default zero switch does not select the data cache.
            cache_invalidate(SW_D);
            ready=s_i32_ld_g(ready_addr);
            used_spins=spin+1;
            if(ready==expected)break;
        }
        const bool valid=ready==expected;
#endif
        float128 sum={0};
        if(valid) {
            const bfloat128 first=rank==0?v_bf16_ld_tnsr_b((int5){tile*128,row},local):
                v_bf16_ld_tnsr_b((int5){tile*128,row,0,point,parity},received);
            sum=v_convert_bf16_to_f32_all_b(first);
            for(int peer=1;peer<group;++peer) {
                bfloat128 value;
                if(peer==rank)value=v_bf16_ld_tnsr_b((int5){tile*128,row},local);
                else value=v_bf16_ld_tnsr_b((int5){tile*128,row,peer,point,parity},received);
                const float128 expanded=v_convert_bf16_to_f32_all_b(value);
                sum.v1=sum.v1+expanded.v1;
                sum.v2=sum.v2+expanded.v2;
            }
        }
        v_bf16_st_tnsr((int5){tile*128,row},output,v_convert_f32_to_bf16_all_b(sum));
        v_i32_st_tnsr_partial((int5){tile,row},status,(int64)(valid?used_spins:-used_spins),0,0);
    }
}
