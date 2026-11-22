// SPDX-License-Identifier: Apache-2.0
// Once per row: join the hi/lo MME projection and compute the control RRMS.
// Feature owners in the post consumer read this shared statistic.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"
void main(tensor residual, tensor projected, tensor output, float epsilon) {
    const int5 start=get_index_space_offset(), end=start+get_index_space_size();
    for(int token=start[0];token<end[0];++token) {
        float128 squares={0};
        #pragma unroll(4)
        for(int k=0;k<20480;k+=128) {
            const float128 x=convert_bfloat128_to_float128(
                v_bf16_ld_tnsr_b((int5){k,token},residual),SW_LINEAR);
            squares.v1=v_f32_mac_b(x.v1,x.v1,squares.v1);
            squares.v2=v_f32_mac_b(x.v2,x.v2,squares.v2);
        }
        const float64 rrms=positive_rsqrt(v_f32_reduce_add(squares.v1+squares.v2)*(1.0f/20480.0f)+epsilon);
        const float64 high=v_f32_ld_tnsr_partial_b((int5){0,token},projected,23,0);
        const float64 low=v_f32_ld_tnsr_partial_b((int5){24,token},projected,23,0);
        v_f32_st_tnsr_partial((int5){0,token},output,high+low,23,0);
        v_f32_st_tnsr_partial((int5){24,token},output,rrms,0,0);
    }
}
