// SPDX-License-Identifier: Apache-2.0
// Visit each ring row once, then scatter its exact codec to the query windows.
#define DSV41_SELECTED_CODEC_ONLY 1
#include "deepseek_v41_selected_kv_vector.h"
void main(tensor swa,tensor shared_rows,tensor shared_mask,tensor positions,tensor lengths,
          tensor rows,tensor values,tensor mask) {
    const int5 first=get_index_space_offset(),end=first+get_index_space_size();
    const int count=get_dim_size(positions,0);
    const int p0=s_i32_ld_g(gen_addr((int5){0},positions),0,0,count>0);
    const int l0=s_i32_ld_g(gen_addr((int5){0},lengths),0,0,count>0);
    const int p1=s_i32_ld_g(gen_addr((int5){1},positions),0,0,count>1);
    const int l1=s_i32_ld_g(gen_addr((int5){1},lengths),0,0,count>1);
    const int p2=s_i32_ld_g(gen_addr((int5){2},positions),0,0,count>2);
    const int l2=s_i32_ld_g(gen_addr((int5){2},lengths),0,0,count>2);
    const int p3=s_i32_ld_g(gen_addr((int5){3},positions),0,0,count>3);
    const int l3=s_i32_ld_g(gen_addr((int5){3},lengths),0,0,count>3);
    const int p4=s_i32_ld_g(gen_addr((int5){4},positions),0,0,count>4);
    const int l4=s_i32_ld_g(gen_addr((int5){4},lengths),0,0,count>4);
    const int p5=s_i32_ld_g(gen_addr((int5){5},positions),0,0,count>5);
    const int l5=s_i32_ld_g(gen_addr((int5){5},lengths),0,0,count>5);
    const uchar256 byte_lanes=V_LANE_ID_8;
    for(int ring=first[0];ring<end[0];++ring) {
        const int o0=(ring-p0+127)&255;
        const bool w0=count>0 && o0<128;
        const bool v0=w0 && o0<l0 && p0-127+o0>=0;
        const int o1=(ring-p1+127)&255;
        const bool w1=count>1 && o1<128;
        const bool v1=w1 && o1<l1 && p1-127+o1>=0;
        const int o2=(ring-p2+127)&255;
        const bool w2=count>2 && o2<128;
        const bool v2=w2 && o2<l2 && p2-127+o2>=0;
        const int o3=(ring-p3+127)&255;
        const bool w3=count>3 && o3<128;
        const bool v3=w3 && o3<l3 && p3-127+o3>=0;
        const int o4=(ring-p4+127)&255;
        const bool w4=count>4 && o4<128;
        const bool v4=w4 && o4<l4 && p4-127+o4>=0;
        const int o5=(ring-p5+127)&255;
        const bool w5=count>5 && o5<128;
        const bool v5=w5 && o5<l5 && p5-127+o5>=0;
        if(!(w0 || w1 || w2 || w3 || w4 || w5))continue;
        const bool decode=v0 || v1 || v2 || v3 || v4 || v5;
        if(w0)s_f32_st_g(gen_addr((int5){o0,0},mask),v0?1.0f:0.0f);
        if(w1)s_f32_st_g(gen_addr((int5){o1,1},mask),v1?1.0f:0.0f);
        if(w2)s_f32_st_g(gen_addr((int5){o2,2},mask),v2?1.0f:0.0f);
        if(w3)s_f32_st_g(gen_addr((int5){o3,3},mask),v3?1.0f:0.0f);
        if(w4)s_f32_st_g(gen_addr((int5){o4,4},mask),v4?1.0f:0.0f);
        if(w5)s_f32_st_g(gen_addr((int5){o5,5},mask),v5?1.0f:0.0f);
        const bool invalid=(w0 && !v0) || (w1 && !v1) || (w2 && !v2) || (w3 && !v3) || (w4 && !v4) || (w5 && !v5);
        if(invalid)for(int chunk=0;chunk<4;++chunk) {
            v_bf16_st_tnsr((int5){chunk*128,o0,0},rows,(bfloat128)0,0,w0 && !v0);
            v_f32_st_tnsr((int5){chunk*128,o0,0},values,(float64)0,0,w0 && !v0);
            v_f32_st_tnsr((int5){chunk*128+64,o0,0},values,(float64)0,0,w0 && !v0);
            v_bf16_st_tnsr((int5){chunk*128,o1,1},rows,(bfloat128)0,0,w1 && !v1);
            v_f32_st_tnsr((int5){chunk*128,o1,1},values,(float64)0,0,w1 && !v1);
            v_f32_st_tnsr((int5){chunk*128+64,o1,1},values,(float64)0,0,w1 && !v1);
            v_bf16_st_tnsr((int5){chunk*128,o2,2},rows,(bfloat128)0,0,w2 && !v2);
            v_f32_st_tnsr((int5){chunk*128,o2,2},values,(float64)0,0,w2 && !v2);
            v_f32_st_tnsr((int5){chunk*128+64,o2,2},values,(float64)0,0,w2 && !v2);
            v_bf16_st_tnsr((int5){chunk*128,o3,3},rows,(bfloat128)0,0,w3 && !v3);
            v_f32_st_tnsr((int5){chunk*128,o3,3},values,(float64)0,0,w3 && !v3);
            v_f32_st_tnsr((int5){chunk*128+64,o3,3},values,(float64)0,0,w3 && !v3);
            v_bf16_st_tnsr((int5){chunk*128,o4,4},rows,(bfloat128)0,0,w4 && !v4);
            v_f32_st_tnsr((int5){chunk*128,o4,4},values,(float64)0,0,w4 && !v4);
            v_f32_st_tnsr((int5){chunk*128+64,o4,4},values,(float64)0,0,w4 && !v4);
            v_bf16_st_tnsr((int5){chunk*128,o5,5},rows,(bfloat128)0,0,w5 && !v5);
            v_f32_st_tnsr((int5){chunk*128,o5,5},values,(float64)0,0,w5 && !v5);
            v_f32_st_tnsr((int5){chunk*128+64,o5,5},values,(float64)0,0,w5 && !v5);
        }
        if(!decode)continue;
        const uchar256 raw=v_u8_ld_tnsr_partial_b((int5){512,ring},swa,15,0);
        const ushort128 codes=convert_uchar256_to_ushort256(raw,SW_LINEAR).v1;
        const bfloat128 scales=selected_ue8m0(codes);
        const uchar256 scale_bits=v_u8_mov_dual_group_all_b(*((uchar256*)&scales),
            0xffffffff,0,0,0,0,MkWrA(3,3,3,3),(uchar256)0);
        for(int chunk=0;chunk<4;++chunk) {
            bfloat128 value=0;
            {
                const uchar256 bytes=v_u8_ld_tnsr_partial_b((int5){chunk*128,ring},swa,127,0);
                const ushort128 code=convert_uchar256_to_ushort256(bytes,SW_LINEAR).v1;
                const uchar256 directions=(((byte_lanes>>6)<<1)+(byte_lanes&1)+chunk*8)|0x80;
                const uchar256 expanded=v_u8_shuffle_b(scale_bits,directions,0,(uchar256)0);
                value=v_bf16_mul_b(selected_e4m3fn(code),*((bfloat128*)&expanded));
            }
            const float128 wide=convert_bfloat128_to_float128(value,SW_LINEAR);
            v_bf16_st_tnsr((int5){chunk*128,o0,0},rows,value,0,v0);
            v_f32_st_tnsr((int5){chunk*128,o0,0},values,wide.v1,0,v0);
            v_f32_st_tnsr((int5){chunk*128+64,o0,0},values,wide.v2,0,v0);
            v_bf16_st_tnsr((int5){chunk*128,o1,1},rows,value,0,v1);
            v_f32_st_tnsr((int5){chunk*128,o1,1},values,wide.v1,0,v1);
            v_f32_st_tnsr((int5){chunk*128+64,o1,1},values,wide.v2,0,v1);
            v_bf16_st_tnsr((int5){chunk*128,o2,2},rows,value,0,v2);
            v_f32_st_tnsr((int5){chunk*128,o2,2},values,wide.v1,0,v2);
            v_f32_st_tnsr((int5){chunk*128+64,o2,2},values,wide.v2,0,v2);
            v_bf16_st_tnsr((int5){chunk*128,o3,3},rows,value,0,v3);
            v_f32_st_tnsr((int5){chunk*128,o3,3},values,wide.v1,0,v3);
            v_f32_st_tnsr((int5){chunk*128+64,o3,3},values,wide.v2,0,v3);
            v_bf16_st_tnsr((int5){chunk*128,o4,4},rows,value,0,v4);
            v_f32_st_tnsr((int5){chunk*128,o4,4},values,wide.v1,0,v4);
            v_f32_st_tnsr((int5){chunk*128+64,o4,4},values,wide.v2,0,v4);
            v_bf16_st_tnsr((int5){chunk*128,o5,5},rows,value,0,v5);
            v_f32_st_tnsr((int5){chunk*128,o5,5},values,wide.v1,0,v5);
            v_f32_st_tnsr((int5){chunk*128+64,o5,5},values,wide.v2,0,v5);
        }
    }
}
