// SPDX-License-Identifier: Apache-2.0
// Independent K accumulators remove the loop-carried FP32 MAC dependency.
// Reduction order follows the upstream tolerance contract, not bit equality.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"
#ifdef DSV41_CONTROL_SWIZZLED
#define WEIGHT_COORD(K,R) ((int5){(K)%128,R,(K)/128})
#else
#define WEIGHT_COORD(K,R) ((int5){K,R})
#endif
void main(tensor activation,tensor weight,tensor output,float epsilon,float inverse_width) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    for(int token=begin[1];token<end[1];++token)for(int row=begin[0];row<end[0];++row) {
        float128 a0={0},a1={0},a2={0},a3={0},squares={0};
        for(int k=0;k<20480;k+=512) {
#ifdef DSV41_CONTROL_BF16_WEIGHT
            const bfloat128 x0=v_bf16_ld_tnsr_b((int5){k,token},activation);
            const bfloat128 x1=v_bf16_ld_tnsr_b((int5){k+128,token},activation);
            const bfloat128 x2=v_bf16_ld_tnsr_b((int5){k+256,token},activation);
            const bfloat128 x3=v_bf16_ld_tnsr_b((int5){k+384,token},activation);
#define ACC(I,A,X) \
            A=v_bf16_mac_acc32_b(X,v_bf16_ld_tnsr_b(WEIGHT_COORD(k+I*128,row),weight),A,0); \
            if(row==0) { \
                const float128 xf=convert_bfloat128_to_float128(X,SW_LINEAR); \
                squares.v1=v_f32_mac_b(xf.v1,xf.v1,squares.v1); \
                squares.v2=v_f32_mac_b(xf.v2,xf.v2,squares.v2); \
            }
#else
            const float128 x0=convert_bfloat128_to_float128(v_bf16_ld_tnsr_b((int5){k,token},activation),SW_LINEAR);
            const float128 x1=convert_bfloat128_to_float128(v_bf16_ld_tnsr_b((int5){k+128,token},activation),SW_LINEAR);
            const float128 x2=convert_bfloat128_to_float128(v_bf16_ld_tnsr_b((int5){k+256,token},activation),SW_LINEAR);
            const float128 x3=convert_bfloat128_to_float128(v_bf16_ld_tnsr_b((int5){k+384,token},activation),SW_LINEAR);
#define ACC(I,A,X) \
            A.v1=v_f32_mac_b(X.v1,v_f32_ld_tnsr_b(WEIGHT_COORD(k+I*128,row),weight),A.v1); \
            A.v2=v_f32_mac_b(X.v2,v_f32_ld_tnsr_b(WEIGHT_COORD(k+I*128+64,row),weight),A.v2); \
            if(row==0) {squares.v1=v_f32_mac_b(X.v1,X.v1,squares.v1);squares.v2=v_f32_mac_b(X.v2,X.v2,squares.v2);}
#endif
            ACC(0,a0,x0);ACC(1,a1,x1);ACC(2,a2,x2);ACC(3,a3,x3);
#undef ACC
        }
        const float64 total=(a0.v1+a0.v2)+(a1.v1+a1.v2)+(a2.v1+a2.v2)+(a3.v1+a3.v2);
        v_f32_st_tnsr_partial((int5){row,token},output,v_f32_reduce_add(total),0,0);
        if(row==0) v_f32_st_tnsr_partial((int5){24,token},output,
            positive_rsqrt(v_f32_reduce_add(squares.v1+squares.v2)*inverse_width+epsilon),0,0);
    }
}
