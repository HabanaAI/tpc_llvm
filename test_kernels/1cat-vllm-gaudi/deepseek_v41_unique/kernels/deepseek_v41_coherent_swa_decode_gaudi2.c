// SPDX-License-Identifier: Apache-2.0
#define DSV41_SELECTED_CODEC_ONLY 1
#include "deepseek_v41_selected_kv_vector.h"

void main(tensor swa, tensor positions, tensor keys
#ifndef DSV41_COHERENT_SWA_KEYS_ONLY
          , tensor values
#endif
) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int tokens=get_dim_size(positions,0);
    const int first=s_i32_ld_g(gen_addr((int5){0},positions))-127;
    const uchar256 byte_lanes=V_LANE_ID_8;
    for(int slot=begin[1];slot<end[1];++slot) {
        const int absolute=first+slot,index=absolute&255;
        const bool valid=slot<128+tokens-1 && absolute>=0;
        uchar256 raw_scales=0;
        if(valid)raw_scales=v_u8_ld_tnsr_partial_b((int5){512,index},swa,15,0);
        const ushort128 codes=convert_uchar256_to_ushort256(raw_scales,SW_LINEAR).v1;
        const bfloat128 scales=selected_ue8m0(codes);
        const uchar256 bits=v_u8_mov_dual_group_all_b(*((uchar256*)&scales),
            0xffffffff,0,0,0,0,MkWrA(3,3,3,3),(uchar256){0});
        for(int chunk=begin[0];chunk<end[0];++chunk) {
            bfloat128 value=0;
            if(valid) {
                const uchar256 raw=v_u8_ld_tnsr_partial_b((int5){chunk*128,index},swa,127,0);
                const ushort128 code=convert_uchar256_to_ushort256(raw,SW_LINEAR).v1;
                const uchar256 directions=(((byte_lanes>>6)<<1)+(byte_lanes&1)+chunk*8)|0x80;
                const uchar256 expanded=v_u8_shuffle_b(bits,directions,0,(uchar256){0});
                value=v_bf16_mul_b(selected_e4m3fn(code),*((bfloat128*)&expanded));
            }
            v_bf16_st_tnsr((int5){chunk*128,slot},keys,value);
#ifndef DSV41_COHERENT_SWA_KEYS_ONLY
            const float128 wide=convert_bfloat128_to_float128(value,SW_LINEAR);
            v_f32_st_tnsr((int5){chunk*128,slot},values,wide.v1);
            v_f32_st_tnsr((int5){chunk*128+64,slot},values,wide.v2);
#endif
        }
    }
}
