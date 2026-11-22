// SPDX-License-Identifier: Apache-2.0
// Output-centric affine row tiles; explicitly zero inactive capacity without a separate memset.
#include "mla_union_codec.h"
void main(tensor swa,tensor main_cache,tensor ids,tensor members,tensor prefix,tensor pages,
          tensor keys,tensor values,tensor shared_members,int ratio) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int capacity=get_dim_size(keys,1),buckets=get_dim_size(prefix,0)-1;
    const int actual=s_i32_ld_g(gen_addr((int5){buckets},prefix));
    const int page_shift=ratio==1?7:6;
    const int page_count=get_dim_size(pages,0),main_length=get_dim_size(main_cache,1);
    int bucket=0,first=0,limit=s_i32_ld_g(gen_addr((int5){1},prefix));
    for(int point=begin[0];point<end[0];++point) {
        for(int row=point*8;row<(point+1)*8 && row<capacity;++row) {
            if(row>=actual) {
                s_i32_st_g(gen_addr((int5){row},shared_members),0);
                for(int chunk=0;chunk<4;++chunk) {
                    v_bf16_st_tnsr((int5){chunk*128,row},keys,(bfloat128)0);
                    v_f32_st_tnsr((int5){chunk*128,row},values,(float64)0);
                    v_f32_st_tnsr((int5){chunk*128+64,row},values,(float64)0);
                }
                continue;
            }
            while(row>=limit && bucket+1<buckets) {
                first=limit;++bucket;limit=s_i32_ld_g(gen_addr((int5){bucket+1},prefix));
            }
            const int local=row-first;
            const int id=s_i32_ld_g(gen_addr((int5){local,bucket},ids));
            s_i32_st_g(gen_addr((int5){row},shared_members),
                       s_i32_ld_g(gen_addr((int5){local,bucket},members)));
            const bool window=id<-1;
            bool valid=1;
            int physical=window?-2-id:0;
            if(!window) {
                const int page=id>>page_shift;
                if(id<0 || page>=page_count)valid=0;
                else {
                    physical=s_i32_ld_g(gen_addr((int5){page},pages))*(1<<page_shift)+(id&((1<<page_shift)-1));
                    physical=physical<0?0:physical;valid=physical<main_length;
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
