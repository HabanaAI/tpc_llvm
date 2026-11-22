// SPDX-License-Identifier: Apache-2.0
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"
void main(tensor partials,tensor output,float epsilon,float inverse_width) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    for(int row=begin[0];row<end[0];++row) {
        float64 control=0;
        for(int tile=0;tile<20;++tile)
            control+=v_f32_ld_tnsr_partial_b((int5){0,tile,row},partials,24,0);
        v_f32_st_tnsr_partial((int5){0,row},output,control,23,0);
        // Broadcast lane 24 within dual group 1 and then across all groups.
        const float64 group=v_f32_mov_dual_group_all_b(control,0xffffffff,1,1,1,1,MkWrA(3,3,3,3),0);
        // TPC shuffle uses bank selectors: logical lane eight is selector
        // 0x20, matching matrix_direction(8) in the accepted gate kernel.
        const float64 mean=v_f32_shuffle_b(group,(uchar256)0xa0,0,(float64)0)*inverse_width;
        v_f32_st_tnsr_partial((int5){24,row},output,positive_rsqrt(mean+epsilon),0,0);
    }
}
