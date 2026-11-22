// SPDX-License-Identifier: Apache-2.0
#include "mla_hash_layout.h"
#include "mla_union_codec.h"
void main(tensor swa,tensor main_cache,tensor selected,tensor owners,tensor pages,tensor decoded,int ratio) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int total=get_dim_size(selected,1)*640;
    for(int bucket=begin[0];bucket<end[0];++bucket) {
        const int owner=s_i32_ld_g(gen_addr((int5){bucket},owners));
        if(owner==0)continue;
        const bool window=bucket<256;
        int physical=bucket;bool valid=1;
        if(!window) {
            const int member=total-owner;
            const int logical=mla_source(selected,member/640,member%640);
            const int width=128/ratio;
            valid=logical>=0 && logical/width<get_dim_size(pages,0);
            if(valid) {
                const int page=s_i32_ld_g(gen_addr((int5){logical/width},pages));
                physical=s_i32_max(page*width+(logical&(width-1)),0);
                valid=physical<get_dim_size(main_cache,1);
            }
        }
        const uchar256 scales=union_row_scales(swa,main_cache,physical,window,valid);
        for(int chunk=0;chunk<4;++chunk) {
            const bfloat128 value=union_decode_chunk(swa,main_cache,physical,window,valid,scales,chunk);
            v_bf16_st_tnsr((int5){chunk*128,bucket},decoded,value);
        }
    }
}
