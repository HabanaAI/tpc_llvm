// SPDX-License-Identifier: Apache-2.0
#include "mla_union_codec.h"

// Sorted selections are emitted by the shared indexer. Partition their
// logical row domain across TPCs, stream each list, and decode every union
// row once. No host metadata, serial CSR builder or full-context bitmap.
static inline int row_at(tensor selected, int slot, int token) {
    const int row=s_i32_ld_g(gen_addr((int5){slot,token,0,0,0},selected));
    return row<0 ? 0x7fffffff : row;
}
static inline int lower(tensor selected,int token,int count,int row) {
    int lo=0,hi=count;
    while(lo<hi) {
        const int mid=(lo+hi)>>1;
        if(row_at(selected,mid,token)<row)lo=mid+1;else hi=mid;
    }
    return lo;
}
static inline void write_row(tensor keys,tensor values,tensor mask,int slot,int token,
                             bfloat128 decoded[4],bool valid) {
    for(int chunk=0;chunk<4;++chunk) {
        const int offset=chunk*128;
        v_bf16_st_tnsr((int5){offset,slot,token,0,0},keys,decoded[chunk]);
        const float128 value=convert_bfloat128_to_float128(decoded[chunk],SW_LINEAR);
        v_f32_st_tnsr((int5){offset,slot,token,0,0},values,value.v1);
        v_f32_st_tnsr((int5){offset+64,slot,token,0,0},values,value.v2);
    }
    s_f32_st_g(gen_addr((int5){slot,token,0,0,0},mask),valid ? 1.0f : 0.0f);
}
void main(tensor swa,tensor main_cache,tensor selected,tensor positions,tensor pages,
          tensor lengths,tensor keys,tensor values,tensor mask,int ratio,int bound) {
    const int5 start=get_index_space_offset(),end=start+get_index_space_size();
    const int tokens=get_dim_size(selected,1),main_rows=get_dim_size(main_cache,1);
    const int page_count=get_dim_size(pages,0),page_width=128/ratio;
    int pos[6],window[6],limit[6],cursor[6],stop[6],current[6];
    for(int token=0;token<tokens;++token) {
        pos[token]=s_i32_ld_g(gen_addr((int5){token},positions));
        const int length=s_i32_ld_g(gen_addr((int5){token},lengths));
        window[token]=s_i32_min(128,s_i32_max(0,length));
        limit[token]=s_i32_min(512,s_i32_max(0,length-128));
    }
    bfloat128 zero[4];
    for(int chunk=0;chunk<4;++chunk)zero[chunk]=(bfloat128)0;
    // 128 disjoint partitions keep all 24 TPCs useful even when almost
    // every main-cache row is shared by the six queries.
    for(int part=start[0];part<end[0];++part) {
        for(int token=0;token<tokens;++token) {
            if(part>=window[token] || pos[token]-127+part<0)
                write_row(keys,values,mask,part,token,zero,0);
            for(int slot=part;slot<512;slot+=128) {
                if(slot>=limit[token] || row_at(selected,slot,token)==0x7fffffff)
                    write_row(keys,values,mask,128+slot,token,zero,0);
            }
        }
        for(int physical=part;physical<256;physical+=128) {
            bool present=0;
            for(int token=0;token<tokens;++token) {
                const int slot=(physical-((pos[token]-127)&255))&255;
                if(slot<window[token] && pos[token]-127+slot>=0)present=1;
            }
            if(present) {
                bfloat128 decoded[4];
                union_decode(swa,main_cache,physical,1,1,decoded);
                for(int token=0;token<tokens;++token) {
                    const int slot=(physical-((pos[token]-127)&255))&255;
                    if(slot<window[token] && pos[token]-127+slot>=0)
                        write_row(keys,values,mask,slot,token,decoded,1);
                }
            }
        }
        // Partition by the selections' rank quantiles, rather than absolute
        // positions: attention often clusters recent rows in a tiny part of
        // the context, which would leave most TPCs idle with equal row ranges.
        int low=part==0 ? 0 : bound,high=bound;
        if(part>0)for(int token=0;token<tokens;++token)
            low=s_i32_min(low,row_at(selected,part*4,token));
        if(part<127)for(int token=0;token<tokens;++token)
            high=s_i32_min(high,row_at(selected,(part+1)*4,token));
        for(int token=0;token<tokens;++token) {
            cursor[token]=lower(selected,token,limit[token],low);
            stop[token]=lower(selected,token,limit[token],high);
            current[token]=cursor[token]<stop[token] ? row_at(selected,cursor[token],token) : 0x7fffffff;
        }
        while(1) {
            int logical=0x7fffffff;
            for(int token=0;token<tokens;++token)logical=s_i32_min(logical,current[token]);
            if(logical==0x7fffffff)break;
            bool valid=logical>=0 && logical/page_width<page_count;
            int physical=0;
            if(valid) {
                const int page=s_i32_ld_g(gen_addr((int5){logical/page_width},pages));
                physical=page*page_width+(logical&(page_width-1));
                valid=page>=0 && physical>=0 && physical<main_rows;
            }
            bfloat128 decoded[4];
            union_decode(swa,main_cache,physical,0,valid,decoded);
            for(int token=0;token<tokens;++token) {
                // Duplicates within one query retain their original slots.
                // The codec still runs only once for their shared row.
                while(current[token]==logical && cursor[token]<stop[token]) {
                    write_row(keys,values,mask,128+cursor[token],token,decoded,valid);
                    ++cursor[token];
                    current[token]=cursor[token]<stop[token]
                        ? row_at(selected,cursor[token],token) : 0x7fffffff;
                }
            }
        }
    }
}
