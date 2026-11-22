// SPDX-License-Identifier: Apache-2.0
#pragma clang fp contract(off)
#include "../include/block32_roundtrip_inline.h"
void main(tensor input,tensor weight,tensor rrms,tensor normalized,tensor quantized) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    for(int row=begin[1];row<end[1];++row) {
        const float reciprocal=s_f32_ld_g(gen_addr((int5){0,row},rrms));
        for(int group=begin[0];group<end[0];++group) {
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
