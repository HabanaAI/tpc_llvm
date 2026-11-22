// SPDX-License-Identifier: Apache-2.0
#include "mla_union_codec.h"

// One codec invocation per union row; all consumers retain their old slot
// order and exact BF16/FP32 operands for the unchanged batch GEMMs.
void main(tensor swa,tensor main_cache,tensor rows,tensor heads,tensor next,
          tensor counts,tensor pages,tensor keys,tensor values,tensor mask,int ratio) {
    const int5 start=get_index_space_offset(),end=start+get_index_space_size();
    const int used=257+s_i32_ld_g(gen_addr((int5){0},counts));
    const int main_length=get_dim_size(main_cache,1);
    for(int point=start[0];point<end[0];++point) for(int row=point;row<used;row+=128) {
        const int first=s_i32_ld_g(gen_addr((int5){row},heads));
        if(first<0) continue;
        bool valid=row!=0;
        int physical=row-1;
        if(row>=257) {
            const int logical=s_i32_ld_g(gen_addr((int5){row-257},rows));
            const int width=128/ratio;
            valid=logical>=0 && logical/width<get_dim_size(pages,0);
            if(valid) {
                const int page=s_i32_ld_g(gen_addr((int5){logical/width},pages));
                physical=page*width+(logical&(width-1));
                physical=s_i32_max(physical,0);
                valid=physical<main_length;
            }
        }
        bfloat128 decoded[4];
        union_decode(swa,main_cache,physical,row<257,valid,decoded);
        for(int member=first;member>=0;) {
            const int token=member/640,slot=member%640;
            for(int chunk=0;chunk<4;++chunk) {
                const int offset=chunk*128;
                const bfloat128 value=decoded[chunk];
                v_bf16_st_tnsr((int5){offset,slot,token,0,0},keys,value);
                const float128 rounded=convert_bfloat128_to_float128(value,SW_LINEAR);
                v_f32_st_tnsr((int5){offset,slot,token,0,0},values,rounded.v1);
                v_f32_st_tnsr((int5){offset+64,slot,token,0,0},values,rounded.v2);
            }
            s_f32_st_g(gen_addr((int5){slot,token,0,0,0},mask),row==0 ? 0.0f : 1.0f);
            member=s_i32_ld_g(gen_addr((int5){slot,token,0,0,0},next));
        }
    }
}
