// SPDX-License-Identifier: Apache-2.0
#pragma clang fp contract(off)
#include "deepseek_v41_softmax_shuffle.h"
void main(tensor logits,tensor statistics,tensor probability) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int columns=get_dim_size(logits,0),tiles=(columns+255)/256,parts=(columns+2047)/2048;
    const int64 lanes=(int64)V_LANE_ID_32;
    for(int task=begin[0];task<end[0];++task) {
        const int row=task/tiles,first=task%tiles*256;
        float64 maxima=v_f32_ld_tnsr_b((int5){0,0,row},statistics);
        float64 masses=v_f32_ld_tnsr_b((int5){0,1,row},statistics);
        const bool64 valid=v_i32_cmp_less_b(lanes,parts);
        maxima=v_f32_mov_vb(maxima,0,(float64)(-1.0f/0.0f),valid,0);
        masses=v_f32_mov_vb(masses,0,(float64)0,valid,0);
        const float64 maximum=softmax_max(maxima);
        const float64 mass=softmax_sum(masses*v_exp_cephes_f32(maxima-maximum));
        const float64 reciprocal=(float64)1.0f/mass;
        for(int i=0;i<4;++i) {
            const int offset=first+i*64;
            const float64 values=v_f32_ld_tnsr_b((int5){offset,row},logits);
            v_f32_st_tnsr((int5){offset,row},probability,v_exp_cephes_f32(values-maximum)*reciprocal);
        }
    }
}
