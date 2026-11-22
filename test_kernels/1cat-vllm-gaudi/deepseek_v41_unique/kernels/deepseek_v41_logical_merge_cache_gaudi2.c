// SPDX-License-Identifier: Apache-2.0
// Merge six sorted selections on device. Decode once, retain the original
// [C,640,512] consumer layout: no union holes, expanded GEMM or CPU metadata.
#include "mla_union_codec.h"
#define INF_KEY 2147483647
static inline int key_at(tensor selected,int token,int slot,int limit) {
    if(slot>=limit)return INF_KEY;
    const int key=s_i32_ld_g(gen_addr((int5){slot,token},selected));
    return key<0?INF_KEY:key;
}
static inline int lower_bound(tensor selected,int token,int limit,int key) {
    int first=0,last=limit;
    while(first<last) {
        const int middle=(first+last)>>1;
        if(key_at(selected,token,middle,limit)<key)first=middle+1;else last=middle;
    }
    return first;
}
static inline void publish(tensor rows,tensor values,tensor mask,int token,int slot,
                           int chunk,bfloat128 value,bool valid) {
    if(!valid)value=(bfloat128)0;
    v_bf16_st_tnsr((int5){chunk*128,slot,token},rows,value);
    const float128 wide=convert_bfloat128_to_float128(value,SW_LINEAR);
    v_f32_st_tnsr((int5){chunk*128,slot,token},values,wide.v1);
    v_f32_st_tnsr((int5){chunk*128+64,slot,token},values,wide.v2);
    if(chunk==0)s_f32_st_g(gen_addr((int5){slot,token},mask),valid?1.0f:0.0f);
}
void main(tensor swa,tensor main_cache,tensor selected,tensor positions,tensor pages,
          tensor lengths,tensor rows,tensor values,tensor mask,int ratio) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int tokens=get_dim_size(selected,1),page_width=128/ratio;
    const int page_count=get_dim_size(pages,0),main_length=get_dim_size(main_cache,1);
    int limits[6],position[6],length[6];
    for(int t=0;t<tokens;++t) {
        length[t]=s_i32_ld_g(gen_addr((int5){t},lengths));
        limits[t]=s_i32_min(512,s_i32_max(0,length[t]-128));
        position[t]=s_i32_ld_g(gen_addr((int5){t},positions));
    }
        for(int part=begin[0];part<end[0];++part) {
            // One physical SWA row supplies every query which can see it.
            for(int physical=part*16;physical<(part+1)*16;++physical) {
                bool needed=0;
                for(int t=0;t<tokens;++t) {
                    const int slot=(physical-(position[t]-127))&255;
                    needed|=slot<s_i32_min(128,length[t]) && position[t]-127+slot>=0;
                }
                const uchar256 scales=union_row_scales(swa,main_cache,physical,1,needed);
                for(int chunk=0;chunk<4;++chunk) {
                    const bfloat128 value=union_decode_chunk(swa,main_cache,physical,1,needed,scales,chunk);
                    for(int t=0;t<tokens;++t) {
                        const int slot=(physical-(position[t]-127))&255;
                        if(slot<128)publish(rows,values,mask,t,slot,chunk,value,
                            slot<length[t] && position[t]-127+slot>=0);
                    }
                }
            }
            // Rank-zero ordinal boundaries balance real sorted selections.
            // Bounds cover every integer key even if other rows differ.
            const int lower=part==0?0:key_at(selected,0,part*32,limits[0]);
            const int upper=part==15?INF_KEY:key_at(selected,0,(part+1)*32,limits[0]);
            int cursor[6],keys[6];
            for(int t=0;t<tokens;++t) {
                cursor[t]=lower_bound(selected,t,limits[t],lower);
                keys[t]=key_at(selected,t,cursor[t],limits[t]);
                // Padding remains in its original slots, split over 16 TPC
                // tasks instead of serializing all zero writes in one task.
                for(int slot=part*32;slot<(part+1)*32;++slot)
                    if(key_at(selected,t,slot,limits[t])==INF_KEY)
                        for(int chunk=0;chunk<4;++chunk)
                            publish(rows,values,mask,t,128+slot,chunk,(bfloat128)0,0);
            }
            while(1) {
                int key=INF_KEY;
                for(int t=0;t<tokens;++t)key=s_i32_min(key,keys[t]);
                if(key>=upper)break;
                const int page=key/page_width;
                bool valid=page>=0 && page<page_count;
                int physical=0;
                if(valid) {
                    physical=s_i32_ld_g(gen_addr((int5){page},pages))*page_width+(key&(page_width-1));
                    physical=s_i32_max(0,physical);valid=physical<main_length;
                }
                const uchar256 scales=union_row_scales(swa,main_cache,physical,0,valid);
                int matched=0;
                for(int t=0;t<tokens;++t)if(keys[t]==key)matched|=1<<t;
                for(int chunk=0;chunk<4;++chunk) {
                    const bfloat128 value=union_decode_chunk(swa,main_cache,physical,0,valid,scales,chunk);
                    for(int t=0;t<tokens;++t)if(matched&(1<<t))
                        publish(rows,values,mask,t,128+cursor[t],chunk,value,1);
                }
                for(int t=0;t<tokens;++t) {
                    if(matched&(1<<t)) {
                        ++cursor[t];keys[t]=key_at(selected,t,cursor[t],limits[t]);
                    }
                }
            }
        }
}
