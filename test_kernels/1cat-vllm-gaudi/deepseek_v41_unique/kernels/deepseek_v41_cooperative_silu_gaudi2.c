// SPDX-License-Identifier: Apache-2.0
// Same producer/rounding helpers as the qualified row kernel. Shared row
// maxima are published as positive FP32 bits with a sign-bit readiness tag.
#define DSV41_SILU_QUANT_FUNCTION 1
#include "../../deepseek_v4/kernels/deepseek_v41_expert_n256_silu_quant_gaudi2.c"

void main(tensor product,tensor ids,tensor activation_scale,tensor channel,tensor router,
          tensor output,tensor scales,tensor coordination) {
    const int5 begin=get_index_space_offset(),size=get_index_space_size();
    const int width=get_dim_size(output,0),groups=width/128;
    const int stop=(begin[0]+size[0]<groups?begin[0]+size[0]:groups);
    const int experts=get_dim_size(channel,2),scale_rows=get_dim_size(activation_scale,1);
    for(int row=begin[1];row<begin[1]+size[1];++row) {
        const float sx=s_f32_ld_g(gen_addr((int5){0,scale_rows==1?0:row},activation_scale));
        const int expert=s_i32_ld_g(gen_addr((int5){row},ids));
        const float route=s_f32_ld_g(gen_addr((int5){row},router));
        const bool valid=expert>=0&&expert<experts;
        bfloat128 activated[10];
        float64 maximum=0;
        for(int f=begin[0];f<stop;++f) {
            float64 a=activated_half(product,channel,f*128,row,expert,width,sx,route,valid);
            float64 b=activated_half(product,channel,f*128+64,row,expert,width,sx,route,valid);
            float128 pair;pair.v1=a;pair.v2=b;
            const bfloat128 value=convert_float128_to_bfloat128(pair,SW_RHNE|SW_LINEAR);
            activated[f-begin[0]]=value;
            const float128 rounded=v_convert_bf16_to_f32_all_b(value);
            maximum=v_f32_max_b(maximum,v_f32_abs_b(rounded.v1));
            maximum=v_f32_max_b(maximum,v_f32_abs_b(rounded.v2));
        }
        maximum=row_max_without_lookup(maximum);
        const int64 tagged=as_int64(maximum)|(int64)0x80000000;
        v_i32_st_tnsr_partial((int5){begin[0],row},coordination,tagged,stop-begin[0]-1,0);
        aso(SW_INC|SW_VPU); aso(SW_DEC|SW_SPU);
        int ready=0;
        for(int poll=0;poll<4096 && !ready;++poll) {
            cache_invalidate(SW_D);
            const int64 raw=v_i32_ld_tnsr_b((int5){0,row},coordination);
            int64 valid_count=v_i32_sel_less_i32_b(raw,0,1,0);
            valid_count=v_i32_sel_geq_i32_b((int64)read_lane_id_4b_b(),groups,0,valid_count);
            valid_count=v_i32_reduce_add(valid_count);
            v_i32_st_tnsr_partial((int5){31,row},coordination,valid_count,0,0);
            aso(SW_INC|SW_VPU); aso(SW_DEC|SW_SPU);
            ready=s_i32_ld_g(gen_addr((int5){31,row},coordination))==groups;
        }
        cache_invalidate(SW_D);
        int64 bits=v_i32_ld_tnsr_b((int5){0,row},coordination)&(int64)0x7fffffff;
        bits=v_i32_sel_geq_i32_b((int64)read_lane_id_4b_b(),groups,0,bits);
        maximum=row_max_without_lookup(as_float64(bits));
        const float64 raw_scale=round_bf16(maximum*(float)(bf16)(1.0f/240.0f));
        float64 scale=round_bf16(raw_scale+(float)(bf16)(1.0e-8f/240.0f));
        const float64 inverse=round_bf16(reciprocal_without_lookup(scale));
        if(!ready)scale=as_float64((int64)0x7fc00000);
        if(begin[0]==0)v_f32_st_tnsr_partial((int5){0,0,row},scales,scale,0,0);
        for(int f=begin[0];f<stop;++f) {
            const float128 rounded=v_convert_bf16_to_f32_all_b(activated[f-begin[0]]);
            minifloat256 packed=0;
            packed=v_convert_f32_to_f8_b(round_bf16(rounded.v1*inverse),0,SW_CLIP_FP,packed);
            packed=v_convert_f32_to_f8_b(round_bf16(rounded.v2*inverse),2,SW_CLIP_FP,packed);
            const minifloat256 sparse=packed;
            packed=v_f8_pack_b(sparse,SW_GROUP_0|SW_STRIDE_2,(minifloat256)0);
            packed=v_f8_pack_b(sparse,SW_GROUP_1|SW_STRIDE_2,packed);
            packed=v_f8_mov_dual_group_pack_b(packed,SW_PACK21,(minifloat256)0);
            v_f8_st_tnsr_partial((int5){f*128,0,row},output,packed,127,0);
        }
    }
}
