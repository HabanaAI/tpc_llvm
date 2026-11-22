// SPDX-License-Identifier: Apache-2.0
#pragma clang fp contract(off)
#include "deepseek_v41_softmax_shuffle.h"
__local__ float64 softmax_values[32];
void main(tensor logits,tensor statistics) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int columns=get_dim_size(logits,0),parts=(columns+2047)/2048;
    const int64 lanes=(int64)V_LANE_ID_32;
    for(int task=begin[0];task<end[0];++task) {
        const int row=task/parts,part=task%parts;
        float64 maximum=(float64)(-1.0f/0.0f);
        for(int i=0;i<32;++i) {
            const int first=part*2048+i*64;
            float64 values=v_f32_ld_tnsr_b((int5){first,row},logits);
            values=v_f32_mov_vb(values,0,(float64)(-1.0f/0.0f),v_i32_cmp_less_b(lanes+first,columns),0);
            softmax_values[i]=values;
            maximum=v_f32_max_b(maximum,values);
        }
        maximum=softmax_max(maximum);
        float64 mass0=0,mass1=0;
        for(int i=0;i<32;i+=2) {
            mass0+=v_exp_cephes_f32(softmax_values[i]-maximum);
            mass1+=v_exp_cephes_f32(softmax_values[i+1]-maximum);
        }
        const float64 mass=softmax_sum(mass0+mass1);
        v_f32_st_tnsr_partial((int5){part,0,row},statistics,maximum,0,0);
        v_f32_st_tnsr_partial((int5){part,1,row},statistics,mass,0,0);
    }
}
