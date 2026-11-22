// SPDX-License-Identifier: Apache-2.0
// The FFN producer already supplies the row scale. No amax/reduction pass.
#pragma clang fp contract(off)
static inline minifloat256 pack_normal(minifloat256 sparse) {
    uchar256 bits = *((uchar256*)&sparse);
    bits = v_u8_sel_eq_u8_b(bits & 0x78, 0, 0, bits);
    sparse = *((minifloat256*)&bits);
    minifloat256 packed = v_f8_pack_b(sparse, SW_GROUP_0|SW_STRIDE_2, (minifloat256)0);
    packed = v_f8_pack_b(sparse, SW_GROUP_1|SW_STRIDE_2, packed);
    return v_f8_mov_dual_group_pack_b(packed, SW_PACK21, (minifloat256)0);
}
void main(tensor input, tensor scales, tensor output, tensor output_scales) {
    const int5 begin=get_index_space_offset(), end=begin+get_index_space_size();
    const int rows=get_dim_size(input,1);
    for (int row=begin[1];row<end[1];++row) {
        const float scale=s_f32_ld_g(gen_addr((int5){0,row},scales));
        // FFN scales are BF16-rounded amax/240, not powers of two. Round
        // their exponent upward, including exact powers, for a conservative
        // covering scale. The BF16 1/240 coefficient/rounding bound is <1.001;
        // a mantissa below2 after BF16 rounding is at most1.9921875.
        const int exponent=(*((int*)&scale)>>23)+1;
        const int inverse_bits=(254-exponent)<<23;
        const bfloat128 inverse=(bf16)(*((float*)&inverse_bits));
        if (begin[0]==0) {
            const int scale_bits=exponent<<23;
            s_f32_st_g(gen_addr((int5){0,row},output_scales),*((float*)&scale_bits));
        }
        for (int tile=begin[0];tile<end[0];++tile) {
            const int5 at={tile*128,row};
            const bfloat128 x=v_bf16_ld_tnsr_b(at,input)*inverse;
            minifloat256 high=v_convert_bf16_to_f8_b(x,0,SW_RHNE|SW_CLIP_FP,(minifloat256)0);
            uchar256 raw=*((uchar256*)&high);
            raw=v_u8_sel_eq_u8_b(raw&0x78,0,0,raw);
            high=*((minifloat256*)&raw);
            const bfloat128 remainder=(x-v_convert_f8_to_bf16_b(high))*(bf16)16;
            const minifloat256 low=v_convert_bf16_to_f8_b(remainder,0,SW_RHNE|SW_CLIP_FP,(minifloat256)0);
            v_f8_st_tnsr_partial(at,output,pack_normal(high),127,0);
            v_f8_st_tnsr_partial((int5){at[0],row+rows},output,pack_normal(low),127,0);
        }
    }
}
