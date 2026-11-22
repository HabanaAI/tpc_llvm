// SPDX-License-Identifier: Apache-2.0
// Deterministic first original slot per bucket; collisions retain a fallback.
#include "mla_hash_layout.h"
void main(tensor selection,tensor positions,tensor lengths,tensor owners) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int total=get_dim_size(selection,1)*640;
    for(int token=begin[1];token<end[1];++token) {
        const int position=s_i32_ld_g(gen_addr((int5){token},positions));
        const int length=s_i32_ld_g(gen_addr((int5){token},lengths));
        for(int slot=begin[0];slot<end[0];++slot) {
            if(slot>=length)continue;
            const int key=slot<128 ? position-127+slot : mla_source(selection,token,slot);
            if(key<0)continue;
            const int bucket=slot<128 ? key&255 : mla_bucket(key);
            v_i32_st_tnsr_partial_rmw((int5){bucket},owners,(int64)(total-token*640-slot),
                                     RMW_SET|RMW_TNSR_DT|RMW_OP_MAX,0,0);
        }
    }
}
