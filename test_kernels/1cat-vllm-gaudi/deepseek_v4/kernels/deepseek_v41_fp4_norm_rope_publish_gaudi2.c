// SPDX-License-Identifier: Apache-2.0
// Common C1 compressor publication: index norm/RoPE, latent RoPE,
// page addressing, packed main/index stores and derived mirrors in one launch.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"
#define DSV4_QNORM_HELPERS_ONLY 1
#define DSV4_ROPE_SECOND_TERM_FMA 1
#include "deepseek_v4_qnorm_rope_kv_pack_bf16.h"
#define DSV41_DECODED_KV_WRITE 1
#define DSV41_FP4_INDEX_ZERO_SIGN 1
#include "deepseek_v41_fp4_pack.h"
static inline float64_pair_t kv_bf16_to_f32_linear(bfloat128 input) {
    bfloat128_pair_t unpacked;
    unpacked.v1 = v_bf16_unpack_b(
        input,
        ((e_group_0) << 8) | ((e_every_second_element) << 9) |
            ((e_lower_half_group) << 10),
        unpacked.v1);
    unpacked.v2 = v_bf16_unpack_b(
        input,
        ((e_group_1) << 8) | ((e_every_second_element) << 9) |
            ((e_lower_half_group) << 10),
        unpacked.v2);

    const bfloat128 first_groups = unpacked.v1;
    unpacked.v1 = v_bf16_mov_dual_group_b(
        unpacked.v2, 0xFFFFFFFF, 0, 1, MkWr(1, 1), unpacked.v1);
    unpacked.v1 = v_bf16_mov_dual_group_b(
        first_groups, 0xFFFFFFFF, 1, 2, MkWr(1, 1), unpacked.v1);
    unpacked.v1 = v_bf16_mov_dual_group_b(
        unpacked.v2, 0xFFFFFFFF, 1, 3, MkWr(1, 1), unpacked.v1);
    unpacked.v2 = v_bf16_mov_dual_group_b(
        first_groups, 0xFFFFFFFF, 2, 0, MkWr(1, 1), unpacked.v2);
    unpacked.v2 = v_bf16_mov_dual_group_b(
        unpacked.v2, 0xFFFFFFFF, 2, 1, MkWr(1, 1), unpacked.v2);
    unpacked.v2 = v_bf16_mov_dual_group_b(
        first_groups, 0xFFFFFFFF, 3, 2, MkWr(1, 1), unpacked.v2);

    const float128 first = v_convert_bf16_to_f32_all_b(unpacked.v1);
    const float128 second = v_convert_bf16_to_f32_all_b(unpacked.v2);
    return (float64_pair_t){first.v1, second.v1};
}


static inline float64 select_group(float64 half_values, int offset, int width) {
    if (offset >= 32)
        half_values = v_f32_mov_dual_group_all_b(half_values, 0xffffffff, 2,3,2,3, MkWrA(3,3,3,3), 0);
    if (offset & 16)
        half_values = v_f32_mov_dual_group_all_b(half_values,0xffffffff,1,1,3,3,MkWrA(3,3,3,3),0);
    return v_f32_sel_less_i32_b((int64)V_LANE_ID_32,width,half_values,(float64)0);
}
void main(tensor latent, tensor raw_index, tensor norm, tensor positions,
          tensor phase, tensor pages, tensor main_cache, tensor index_cache,
          tensor main_mirror, tensor hot_mirror, tensor index_mirror,
          tensor completion, float epsilon, int ratio, int main_enabled,
          int hot_enabled, int index_enabled) {
    const int5 begin=get_index_space_offset(), end=begin+get_index_space_size();
    const int position=s_i32_ld_g(gen_addr((int5){0},positions));
    const int logical=position/ratio, width=128/ratio;
    const int first=position & -ratio;
    const bool visible=(position & (ratio-1))==ratio-1;
    const int page=s_i32_ld_g(gen_addr((int5){logical/width},pages));
    const int physical=visible ? page*width + (logical & (width-1)) : logical & (width-1);
    const bool valid=position>=0 && physical>=0 && physical<get_dim_size(main_cache,1)
                     && physical<get_dim_size(index_cache,1);
    for (int owner=begin[0];owner<end[0];++owner) {
        if (valid && owner<4) {
            const bfloat128 raw=v_bf16_ld_tnsr_b((int5){0},raw_index);
            const float128 square=v_bf16_mac_acc32_b(raw,raw,(float128){0},(e_no_negation)<<1);
            const float64 rrms=positive_rsqrt(row_sum(square.v1+square.v2)*(1.0f/128.0f)+epsilon);
            const float128 weight=v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b((int5){0},norm));
            float128 x=v_convert_bf16_to_f32_all_b(raw);
            x.v1=(x.v1*rrms)*weight.v1;x.v2=(x.v2*rrms)*weight.v2;
            float64_pair_t linear=kv_bf16_to_f32_linear(v_convert_f32_to_bf16_all_b(x));
            if(owner>=2) {
                float128 rotated={0};rotated.v1=dsv4_qkv_apply_pairwise_rope_f32(linear.v2,phase,first);
                linear.v2=convert_bfloat128_to_float128(convert_float128_to_bfloat128(rotated,SW_RHNE|SW_LINEAR),SW_LINEAR).v1;
            }
            const float64 number=select_group(owner<2?linear.v1:linear.v2,(owner&1)*32,32);
            const bool hot=hot_enabled && logical<get_dim_size(hot_mirror,1);
            const bool mirror=index_enabled && logical<get_dim_size(index_mirror,1);
            const bfloat128 decoded=fp4_pack_number(number,index_cache,owner,physical,32,128,
                hot_mirror,hot||mirror,hot?logical:-1);
            if(mirror) v_bf16_st_tnsr_partial((int5){owner*32,logical},index_mirror,decoded,31,0);
        } else if(valid) {
            const int group=owner-4, feature=group*16, tile=feature/128;
            const bfloat128 raw=v_bf16_ld_tnsr_b((int5){tile*128,0},latent);
            float64_pair_t linear=kv_bf16_to_f32_linear(raw);
            if(feature>=448) {
                float128 rotated={0};rotated.v1=dsv4_qkv_apply_pairwise_rope_f32(linear.v2,phase,first);
                linear.v2=convert_bfloat128_to_float128(convert_float128_to_bfloat128(rotated,SW_RHNE|SW_LINEAR),SW_LINEAR).v1;
            }
            const float64 half_values=(feature&127)<64?linear.v1:linear.v2;
            const float64 number=select_group(half_values,feature&63,16);
            fp4_pack_number(number,main_cache,group,physical,16,512,main_mirror,
                            main_enabled && logical<get_dim_size(main_mirror,1),logical);
        }
        s_i32_st_g(gen_addr((int5){owner},completion),valid?position:-1);
    }
}
