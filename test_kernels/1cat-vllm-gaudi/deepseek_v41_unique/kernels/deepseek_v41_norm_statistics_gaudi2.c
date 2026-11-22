// SPDX-License-Identifier: Apache-2.0
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"
void main(tensor input,tensor rrms,float epsilon,float inverse_width) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int tiles=get_dim_size(input,0)/128;
    for(int row=begin[0];row<end[0];++row) {
        // Same lane accumulation and final row reduction as the C1 norm.
        // Do not cache forty vectors in local memory across the reduction.
        float128 squares={0};
        for(int tile=0;tile<tiles;++tile) {
            const bfloat128 value=v_bf16_ld_tnsr_b((int5){tile*128,row},input);
            squares=v_bf16_mac_acc32_b(value,value,squares,(e_no_negation)<<1);
        }
        const float64 result=positive_rsqrt(row_sum(squares.v1+squares.v2)*inverse_width+epsilon);
        v_f32_st_tnsr_partial((int5){0,row},rrms,result,0,0);
    }
}
