// SPDX-License-Identifier: Apache-2.0
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"
#include "../include/block32_roundtrip_inline.h"

void main(tensor input,tensor weight,tensor normalized,tensor quantized,
          float epsilon,float inverse_width) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int columns=get_dim_size(input,0),tiles=columns/128,groups=columns/64;
    for(int row=begin[1];row<end[1];++row) {
        for(int feature=begin[0];feature<end[0];++feature) {
            // One row statistic per 512-column work item, reused by all eight
            // group pairs. This bounded reread removes an execution boundary
            // while keeping the accepted C1 accumulation association.
            float128 squares={0};
            for(int tile=0;tile<tiles;++tile) {
                const bfloat128 value=v_bf16_ld_tnsr_b((int5){tile*128,row},input);
                squares=v_bf16_mac_acc32_b(value,value,squares,(e_no_negation)<<1);
            }
            const float64 reciprocal=positive_rsqrt(row_sum(squares.v1+squares.v2)*inverse_width+epsilon);
            for(int group=feature*8;group<(feature+1)*8 && group<groups;++group) {
                const int5 at={group*64,row},wt={group*64,0};
                const float64 x=convert_bfloat128_to_float128(v_bf16_ld_tnsr_partial_b(at,input,63,0),SW_LINEAR).v1;
                const float64 w=convert_bfloat128_to_float128(v_bf16_ld_tnsr_partial_b(wt,weight,63,0),SW_LINEAR).v1;
                float128 expanded={0};expanded.v1=(x*reciprocal)*w;
                const bfloat128 rounded=convert_float128_to_bfloat128(expanded,SW_RHNE|SW_LINEAR);
                v_bf16_st_tnsr_partial(at,normalized,rounded,63,0);
                const float64 value=convert_bfloat128_to_float128(rounded,SW_LINEAR).v1;
                float128 result={0};result.v1=paired_block32_roundtrip(value);
                v_bf16_st_tnsr_partial(at,quantized,convert_float128_to_bfloat128(result,SW_RHNE|SW_LINEAR),63,0);
            }
        }
    }
}
