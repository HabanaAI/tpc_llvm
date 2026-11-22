// SPDX-License-Identifier: Apache-2.0
// Reuse each tiled square/max result for Gaudi2 power-of-two FP8 and RRMS.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"
void main(tensor input,tensor statistics,tensor output,tensor scales,tensor rrms,
          float epsilon,float inverse_width) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int count=get_dim_size(statistics,0);
    for(int row=begin[1];row<end[1];++row) {
        float64 squares=0,maximum=0;
        for(int offset=0;offset<count;offset+=64) {
            const int left=count-offset,last=left<64?left-1:63;
            squares+=v_f32_ld_tnsr_partial_b((int5){offset,0,row},statistics,last,0);
            maximum=v_f32_max_b(maximum,
                v_f32_ld_tnsr_partial_b((int5){offset,1,row},statistics,last,0));
        }
        squares=v_f32_reduce_add(squares);
        maximum=v_f32_reduce_max(maximum);
        const uint64 bits=as_uint64(maximum);
        int64 power=convert_uint64_to_int64(bits>>23,0)-134;
        power+=v_i32_sel_grt_u32_b(bits&0x7fffff,0x700000,1,0);
        power=v_i32_sel_eq_f32_b(maximum,0.0f,0,power);
        const float64 scale=as_float64((power+127)<<23),inverse=as_float64((127-power)<<23);
        const bfloat128 inv=v_convert_f32_to_bf16_all_b((float128){inverse,inverse});
        if(begin[0]==0) {
            v_f32_st_tnsr_partial((int5){0,row},scales,scale,0,0);
            v_f32_st_tnsr_partial((int5){0,row},rrms,
                positive_rsqrt(squares*inverse_width+epsilon),0,0);
        }
        for(int tile=begin[0];tile<end[0];++tile)for(int sub=0;sub<4;++sub) {
            const int5 at={tile*512+sub*128,row};
            const bfloat128 x=v_bf16_ld_tnsr_b(at,input)*inv;
            minifloat256 packed=v_convert_bf16_to_f8_b(x,0,SW_RHNE|SW_CLIP_FP,(minifloat256)0);
            const minifloat256 sparse=packed;
            packed=v_f8_pack_b(sparse,SW_GROUP_0|SW_STRIDE_2,(minifloat256)0);
            packed=v_f8_pack_b(sparse,SW_GROUP_1|SW_STRIDE_2,packed);
            packed=v_f8_mov_dual_group_pack_b(packed,SW_PACK21,(minifloat256)0);
            uchar256 raw=*((uchar256*)&packed);
            raw=v_u8_sel_eq_u8_b(raw&0x78,0,0,raw);
            v_f8_st_tnsr_partial(at,output,*((minifloat256*)&raw),127,0);
        }
    }
}
