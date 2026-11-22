// SPDX-License-Identifier: Apache-2.0
#include "mla_union_codec.h"
// Each selected slot checks six owners, avoiding a serial merge/linked list.
void main(tensor swa,tensor main_cache,tensor selection,tensor positions,tensor pages,
          tensor map,tensor completion,tensor keys,tensor values,tensor mask,int ratio) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int tokens=get_dim_size(selection,1),page_width=128/ratio;
    for(int token=begin[1];token<end[1];++token)for(int slot=begin[0];slot<end[0];++slot) {
        const int key=s_i32_ld_g(gen_addr((int5){slot,token},completion));
        const bool selected=key!=-1;
        const bool window=slot<128;
        int members[6];
        bool direct=key<0;
        if(!direct)direct=s_i32_ld_g(gen_addr((int5){key,token},map))!=slot+1;
        int owner=token;
        if(!direct) {
            for(int t=0;t<tokens;++t)members[t]=s_i32_ld_g(gen_addr((int5){key,t},map))-1;
            for(int t=token-1;t>=0;--t)if(members[t]>=0)owner=t;
            if(owner!=token)continue;
        }
        int physical=0;
        bool valid=selected;
        if(window)physical=key;
        else {
            const int logical=s_i32_ld_g(gen_addr((int5){slot-128,token},selection));
            valid=valid&&logical>=0&&logical/page_width<get_dim_size(pages,0);
            if(valid) {
                const int page=s_i32_ld_g(gen_addr((int5){logical/page_width},pages));
                physical=s_i32_max(page*page_width+(logical&(page_width-1)),0);
                valid=physical<get_dim_size(main_cache,1);
            }
        }
        bfloat128 decoded[4];
        union_decode(swa,main_cache,physical,window,valid,decoded);
        for(int t=0;t<tokens;++t) {
            const int destination=direct?(t==token?slot:-1):members[t];
            if(destination<0)continue;
            for(int chunk=0;chunk<4;++chunk) {
                const int offset=chunk*128;
                v_bf16_st_tnsr((int5){offset,destination,t},keys,decoded[chunk]);
                const float128 widened=convert_bfloat128_to_float128(decoded[chunk],SW_LINEAR);
                v_f32_st_tnsr((int5){offset,destination,t},values,widened.v1);
                v_f32_st_tnsr((int5){offset+64,destination,t},values,widened.v2);
            }
            s_f32_st_g(gen_addr((int5){destination,t},mask),selected?1.0f:0.0f);
        }
    }
}
