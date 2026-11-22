// SPDX-License-Identifier: Apache-2.0
#pragma clang fp contract(off)
#include "deepseek_v41_sampling_reduce.h"
void main(tensor cumulative,tensor ids,tensor controls,tensor packet,tensor maximum,tensor selected,
          int columns,int ranks,int width) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int64 lanes=(int64)V_LANE_ID_32;
    for(int row=begin[0];row<end[0];++row) {
        const float temperature=s_f32_ld_g(gen_addr((int5){0,row},controls));
        const float u=s_f32_ld_g(gen_addr((int5){2,row},controls));
        const float final=s_f32_ld_g(gen_addr((int5){columns-1,row},cumulative));
        const float threshold=u*final;
        int64 count=0;
        for(int col=0;col<columns;col+=64) {
            const float64 cdf=v_f32_ld_tnsr_b((int5){col,row},cumulative);
            count+=v_i32_mov_vb(1,0,0,v_f32_cmp_less_b(cdf,threshold),0);
        }
        const int64 wanted=v_i32_min_b(sampling_sum(count),columns-1);
        int64 token=0;
        for(int col=0;col<columns;col+=64) {
            const int64 value=v_i32_ld_tnsr_b((int5){col,row},ids);
            token|=v_i32_mov_vb(value,0,0,v_i32_cmp_eq_b(lanes+col,wanted),0);
        }
        token=sampling_sum(token);
        if(temperature==0) {
            const float highest=s_f32_ld_g(gen_addr((int5){0,row},maximum));
            int greedy=16777216;
            for(int rank=0;rank<ranks;++rank) {
                const int at=rank*(3+2*width);
                const float score=s_f32_ld_g(gen_addr((int5){at,row},packet));
                const float id=s_f32_ld_g(gen_addr((int5){at+2,row},packet));
                if(score==highest && id<greedy)greedy=(int)id;
            }
            token=(int64)greedy;
        }
        v_i32_st_tnsr_partial((int5){0,row},selected,token,0,0);
    }
}
