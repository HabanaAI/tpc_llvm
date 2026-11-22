// SPDX-License-Identifier: Apache-2.0
// Exact production FP8 W13 postprocess for one expert matrix with M1..6.
static inline float64 round_bf16(float64 value) {
    float64_pair_t pair; pair.v1=value; pair.v2=value;
    return v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(pair)).v1;
}
static inline float64 reciprocal_without_lookup(float64 value) {
    float64 estimate=as_float64((int64)0x7ef311c3-as_int64(value));
    estimate=estimate*(2.0f-value*estimate);estimate=estimate*(2.0f-value*estimate);
    estimate=estimate*(2.0f-value*estimate);
    return v_f32_sel_eq_f32_b(value,as_float64((int64)0x7f800000),0.0f,estimate);
}
static inline float64 row_max_without_lookup(float64 value) {
    value=v_f32_max_b(value,v_f32_mov_dual_group_all_b(value,0xffffffff,1,0,3,2,MkWrA(3,3,3,3),0));
    value=v_f32_max_b(value,v_f32_mov_dual_group_all_b(value,0xffffffff,2,3,0,1,MkWrA(3,3,3,3),0));
    value=v_f32_max_b(value,v_f32_mov_group_b(value,0xffffffff,63,0));
    float64 result=0;
    #pragma loop_unroll(8)
    for(int lane=0;lane<8;++lane) result=v_f32_max_b(result,v_f32_shuffle_b(value,(uchar256)(0x80|lane),0,value));
    return result;
}
static inline float64 projected(tensor product,tensor channel,int n,int row,int expert,float sx) {
    const float64 acc=v_f32_ld_tnsr_b((int5){n,row,0,0,0},product);
    const uint64 bits=v_u32_ld_tnsr_b((int5){n%256,n/256,expert},channel,
        SW_UNPACK|SW_UNPCK_16_TO_32)<<16;
    return round_bf16(v_f32_mul_b(v_f32_mul_b(acc,*((float64*)&bits)),sx));
}
static inline float64 activated_half(tensor product,tensor channel,int n,int row,
                                     int expert,int width,float sx,float route) {
    float64 gate=projected(product,channel,n,row,expert,sx);
    float64 up=projected(product,channel,n+width,row,expert,sx);
    gate=v_f32_min_b(gate,10.0f);up=v_f32_max_b(v_f32_min_b(up,10.0f),-10.0f);
    return v_f32_mul_b(v_f32_mul_b(v_f32_mul_b(gate,v_sigmoid_f32(gate)),up),route);
}
void main(tensor product,tensor metadata,tensor activation_scale,tensor channel,tensor routing,
          tensor output,tensor scales,int owner) {
    const int5 start=get_index_space_offset(),end=start+get_index_space_size();
    const int width=get_dim_size(output,0), experts=get_dim_size(channel,2);
    const int expert=s_i32_ld_g(gen_addr((int5){owner,1,0,0,0},metadata));
    const int rows=s_i32_ld_g(gen_addr((int5){owner,3,0,0,0},metadata));
    if(expert<0 || expert>=experts) return;
    bfloat128 activated[10];
    for(int row=start[0];row<end[0] && row<rows;++row) {
        const float sx=s_f32_ld_g(gen_addr((int5){row,owner,0,0,0},activation_scale));
        const float route=s_f32_ld_g(gen_addr((int5){row,owner,0,0,0},routing));
        float64 maximum=0;
        #pragma loop_unroll(2)
        for(int tile=0;tile<width/128;++tile) {
            float64_pair_t pair;
            pair.v1=activated_half(product,channel,tile*128,row,expert,width,sx,route);
            pair.v2=activated_half(product,channel,tile*128+64,row,expert,width,sx,route);
            const bfloat128 value=convert_float128_to_bfloat128(pair,SW_RHNE|SW_LINEAR);
            activated[tile]=value;
            const float64_pair_t rounded=v_convert_bf16_to_f32_all_b(value);
            maximum=v_f32_max_b(maximum,v_f32_abs_b(rounded.v1));
            maximum=v_f32_max_b(maximum,v_f32_abs_b(rounded.v2));
        }
        maximum=row_max_without_lookup(maximum);
        const float64 raw=round_bf16(maximum*(float)(bf16)(1.0f/240.0f));
        const float64 scale=round_bf16(raw+(float)(bf16)(1.0e-8f/240.0f));
        const float64 inverse=round_bf16(reciprocal_without_lookup(scale));
        v_f32_st_tnsr((int5){0,row,0,0,0},scales,scale);
        #pragma loop_unroll(2)
        for(int tile=0;tile<width/128;++tile) {
            const float64_pair_t fp32=v_convert_bf16_to_f32_all_b(activated[tile]);
            minifloat256 packed=0;
            packed=v_convert_f32_to_f8_b(round_bf16(fp32.v1*inverse),0,SW_CLIP_FP,packed);
            packed=v_convert_f32_to_f8_b(round_bf16(fp32.v2*inverse),2,SW_CLIP_FP,packed);
            const minifloat256 sparse=packed;
            packed=v_f8_pack_b(sparse,SW_GROUP_0|SW_STRIDE_2,(minifloat256)0);
            packed=v_f8_pack_b(sparse,SW_GROUP_1|SW_STRIDE_2,packed);
            packed=v_f8_mov_dual_group_pack_b(packed,SW_PACK21,(minifloat256)0);
            v_f8_st_tnsr_partial((int5){tile*128,row,0,0,0},output,packed,127,0);
        }
    }
}
