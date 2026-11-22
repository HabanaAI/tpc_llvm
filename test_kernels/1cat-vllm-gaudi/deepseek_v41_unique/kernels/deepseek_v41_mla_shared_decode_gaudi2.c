// SPDX-License-Identifier: Apache-2.0
#include "mla_union_codec.h"
void main(tensor swa,tensor main_cache,tensor ids,tensor members,tensor pages,
          tensor keys,tensor values,int ratio) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int width=get_dim_size(ids,0),page_shift=ratio==1?7:6;
    const int page_count=get_dim_size(pages,0),main_length=get_dim_size(main_cache,1);
    for(int point=begin[0];point<end[0];++point) {
        for(int row=point*8;row<(point+1)*8 && row<width;++row) {
            const int id=s_i32_ld_g(gen_addr((int5){row},ids));
            bool valid=s_i32_ld_g(gen_addr((int5){row},members))!=0;
            const bool window=id<-1;
            int physical=window?-2-id:0;
            if(!window && valid) {
                const int page=id>>page_shift;
                if(id<0 || page>=page_count)valid=0;
                else {
                    physical=s_i32_ld_g(gen_addr((int5){page},pages))*(1<<page_shift)+(id&((1<<page_shift)-1));
                    physical=physical<0?0:physical;
                    valid=physical<main_length;
                }
            }
            const uchar256 scales=union_row_scales(swa,main_cache,physical,window,valid);
            for(int chunk=0;chunk<4;++chunk) {
                const bfloat128 decoded=union_decode_chunk(swa,main_cache,physical,window,valid,scales,chunk);
                v_bf16_st_tnsr((int5){chunk*128,row},keys,decoded);
                const float128 widened=convert_bfloat128_to_float128(decoded,SW_LINEAR);
                v_f32_st_tnsr((int5){chunk*128,row},values,widened.v1);
                v_f32_st_tnsr((int5){chunk*128+64,row},values,widened.v2);
            }
        }
    }
}
