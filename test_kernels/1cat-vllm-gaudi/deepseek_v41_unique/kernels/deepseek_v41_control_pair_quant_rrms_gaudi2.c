// SPDX-License-Identifier: Apache-2.0
// No local-memory activation cache: one fixed statistic pass, then two
// normal FP8 planes at scales s and s/16, in the same affine 512-column tile.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"
static inline minifloat256 linear_pack(minifloat256 sparse) {
    minifloat256 value=v_f8_pack_b(sparse,SW_GROUP_0|SW_STRIDE_2,(minifloat256)0);
    value=v_f8_pack_b(sparse,SW_GROUP_1|SW_STRIDE_2,value);
    return v_f8_mov_dual_group_pack_b(value,SW_PACK21,(minifloat256)0);
}
static inline minifloat256 normal_only(minifloat256 packed) {
    uchar256 raw=*((uchar256*)&packed);
    raw=v_u8_sel_eq_u8_b(raw&0x78,0,0,raw);
    return *((minifloat256*)&raw);
}
void main(tensor input,tensor statistics,tensor output,tensor scales,tensor rrms,
          float epsilon,float inverse_width) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int rows=get_dim_size(input,1),count=get_dim_size(statistics,0);
    for(int row=begin[1];row<end[1];++row) {
        float64 squares=0,maximum=0;
        for(int offset=0;offset<count;offset+=64) {
            const int left=count-offset,last=left<64?left-1:63;
            squares+=v_f32_ld_tnsr_partial_b((int5){offset,0,row},statistics,last,0);
            maximum=v_f32_max_b(maximum,
                v_f32_ld_tnsr_partial_b((int5){offset,1,row},statistics,last,0));
        }
        squares=v_f32_reduce_add(squares);maximum=v_f32_reduce_max(maximum);
        const uint64 bits=as_uint64(maximum);
        int64 power=convert_uint64_to_int64(bits>>23,0)-134;
        power+=v_i32_sel_grt_u32_b(bits&0x7fffff,0x700000,1,0);
        power=v_i32_sel_eq_f32_b(maximum,0.0f,0,power);
        const float64 scale=as_float64((power+127)<<23),inverse=as_float64((127-power)<<23);
        const bfloat128 inv=v_convert_f32_to_bf16_all_b((float128){inverse,inverse});
        if(begin[0]==0) {
            v_f32_st_tnsr_partial((int5){0,row},scales,scale,0,0);
            v_f32_st_tnsr_partial((int5){0,row+rows},scales,scale*.0625f,0,0);
            v_f32_st_tnsr_partial((int5){0,row},rrms,
                positive_rsqrt(squares*inverse_width+epsilon),0,0);
        }
        for(int tile=begin[0];tile<end[0];++tile)for(int sub=0;sub<4;++sub) {
            const int5 at={tile*512+sub*128,row};
            const bfloat128 x=v_bf16_ld_tnsr_b(at,input)*inv;
            const minifloat256 high=normal_only(v_convert_bf16_to_f8_b(x,0,SW_RHNE|SW_CLIP_FP,
                                                                     (minifloat256)0));
            const bfloat128 restored=v_convert_f8_to_bf16_b(high);
            const bfloat128 residue=(x-restored)*(bf16)16;
            const minifloat256 low=normal_only(v_convert_bf16_to_f8_b(residue,0,SW_RHNE|SW_CLIP_FP,
                                                                    (minifloat256)0));
            v_f8_st_tnsr_partial(at,output,linear_pack(high),127,0);
            v_f8_st_tnsr_partial((int5){at[0],row+rows},output,linear_pack(low),127,0);
        }
    }
}
