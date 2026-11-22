// SPDX-License-Identifier: Apache-2.0
#pragma once
#define DSV41_SELECTED_CODEC_ONLY
#include "../../deepseek_v4/include/deepseek_v41_selected_kv_vector.h"
static inline uchar256 union_row_scales(tensor swa,tensor main_cache,int physical,bool window,bool valid) {
    if(!valid)return (uchar256)0;
    const uchar256 raw=window ? v_u8_ld_tnsr_partial_b((int5){512,physical},swa,15,0) :
        v_u8_ld_tnsr_partial_b((int5){256,physical},main_cache,31,0);
    return v_u8_mov_dual_group_all_b(raw,0xffffffff,0,0,0,0,MkWrA(3,3,3,3),(uchar256)0);
}
static inline bfloat128 union_decode_chunk(tensor swa,tensor main_cache,int physical,bool window,bool valid,
                                          uchar256 row_scales,int chunk) {
    if(!valid)return (bfloat128)0;
    const uchar256 lanes=V_LANE_ID_8;
    if(window) {
        const uchar256 bytes=v_u8_ld_tnsr_partial_b((int5){chunk*128,physical},swa,127,0);
        const ushort128 code=convert_uchar256_to_ushort256(bytes,SW_LINEAR).v1;
        const uchar256 directions=((lanes>>5)+chunk*4)|0x80;
        const uchar256 scale_bytes=v_u8_shuffle_b(row_scales,directions,0,(uchar256)0);
        const ushort128 scales=convert_uchar256_to_ushort256(scale_bytes,SW_LINEAR).v1;
        return v_bf16_mul_b(selected_e4m3fn(code),selected_ue8m0(scales));
    }
    const uchar256 raw=v_u8_ld_tnsr_partial_b((int5){chunk*64,physical},main_cache,63,0);
    const uchar256 bytes=v_u8_mov_dual_group_all_b(raw,0xffffffff,0,0,0,0,MkWrA(3,3,3,3),(uchar256)0);
    const uchar256 expanded=v_u8_shuffle_b(bytes,(lanes>>1)|0x80,0,(uchar256)0);
    const ushort128 shifts=((ushort128)V_LANE_ID_16&1)<<2;
    const ushort128 code=(convert_uchar256_to_ushort256(expanded,SW_LINEAR).v1>>shifts)&15;
    const uchar256 directions=((lanes>>4)+chunk*8)|0x80;
    const uchar256 scale_bytes=v_u8_shuffle_b(row_scales,directions,0,(uchar256)0);
    const ushort128 scales=convert_uchar256_to_ushort256(scale_bytes,SW_LINEAR).v1;
    const bfloat128 value=v_bf16_mul_b(selected_fp4(code),selected_e4m3fn(scales));
    return v_bf16_sel_eq_bf16_b(value,(bfloat)0,(bfloat)0,value);
}
static inline void union_decode(tensor swa,tensor main_cache,int physical,bool window,bool valid,bfloat128 decoded[4]) {
    const uchar256 scales=union_row_scales(swa,main_cache,physical,window,valid);
    for(int chunk=0;chunk<4;++chunk)
        decoded[chunk]=union_decode_chunk(swa,main_cache,physical,window,valid,scales,chunk);
}
