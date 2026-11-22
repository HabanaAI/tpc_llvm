// SPDX-License-Identifier: Apache-2.0
// Preserve original per-query slots and GEMM operands. Hash collisions decode
// their own original row rather than silently substituting another logical ID.
#include "mla_hash_layout.h"
#include "mla_union_codec.h"
void main(tensor swa,tensor main_cache,tensor selected,tensor positions,tensor lengths,tensor pages,
          tensor owners,tensor decoded,tensor keys,tensor values,tensor mask,int ratio) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int total=get_dim_size(selected,1)*640;
    for(int token=begin[1];token<end[1];++token) {
        const int position=s_i32_ld_g(gen_addr((int5){token},positions));
        const int length=s_i32_ld_g(gen_addr((int5){token},lengths));
        for(int slot=begin[0];slot<end[0];++slot) {
            const bool window=slot<128;
            const int logical=window ? position-127+slot : mla_source(selected,token,slot);
            const bool local_valid=slot<length && logical>=0;
            bool shared=0;int bucket=0;
            if(local_valid) {
                bucket=window ? logical&255 : mla_bucket(logical);
                const int owner=s_i32_ld_g(gen_addr((int5){bucket},owners));
                if(owner>0) {
                    const int member=total-owner;
                    shared=window || mla_source(selected,member/640,member%640)==logical;
                }
            }
            int physical=logical&255;bool valid=local_valid;
            if(!shared && !window) {
                const int width=128/ratio;
                valid=valid && logical/width<get_dim_size(pages,0);
                if(valid) {
                    const int page=s_i32_ld_g(gen_addr((int5){logical/width},pages));
                    physical=s_i32_max(page*width+(logical&(width-1)),0);
                    valid=physical<get_dim_size(main_cache,1);
                }
            }
            const uchar256 scales=union_row_scales(swa,main_cache,physical,window,valid&&!shared);
            for(int chunk=0;chunk<4;++chunk) {
                const int col=chunk*128;
                const bfloat128 value=shared ? v_bf16_ld_tnsr_b((int5){col,bucket},decoded) :
                    union_decode_chunk(swa,main_cache,physical,window,valid,scales,chunk);
                v_bf16_st_tnsr((int5){col,slot,token},keys,value);
                const float128 expanded=convert_bfloat128_to_float128(value,SW_LINEAR);
                v_f32_st_tnsr((int5){col,slot,token},values,expanded.v1);
                v_f32_st_tnsr((int5){col+64,slot,token},values,expanded.v2);
            }
            s_f32_st_g(gen_addr((int5){slot,token},mask),local_valid?1.0f:0.0f);
        }
    }
}
