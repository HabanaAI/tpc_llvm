// SPDX-License-Identifier: Apache-2.0
// Normalized/rotated current KV is encoded and consumed directly by its SWA
// slot. Other slots only read preceding ring rows or shared selected main rows.
// No cross-TPC barrier is needed: no reader reloads the current cache row.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"
#define DSV4_QNORM_HELPERS_ONLY 1
#define DSV4_ROPE_SECOND_TERM_FMA 1
#include "deepseek_v4_qnorm_rope_kv_pack_bf16.h"
#define DSV41_DECODED_KV_WRITE 1
#define DSV41_OPTIONAL_DECODED_KV_WRITE 1
#define DSV41_RETURN_DECODED_KV 1
#include "deepseek_v41_swa_pack.h"
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


void main(tensor input,tensor weight,tensor phase,tensor swa,tensor shared_rows,
          tensor shared_mask,tensor positions,tensor lengths,tensor rows,
          tensor values,tensor mask,float epsilon,float inverse_width) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int position=s_i32_ld_g(gen_addr((int5){0},positions));
    const int length=s_i32_ld_g(gen_addr((int5){0},lengths));
    const uint64 lanes=V_LANE_ID_32;
    uint256 scale_directions={0};scale_directions.v1=(lanes>>5)|0x80;
    const uchar256 directions=convert_uint256_to_uchar256(scale_directions,SW_LINEAR);
    for(int chunk=begin[0];chunk<end[0];++chunk) {
        for(int slot=begin[1];slot<end[1];++slot) {
            const int absolute=position-127+slot,ring=absolute&255;
            const bool valid_swa=slot<length && absolute>=0;
            const float valid=slot<128?(valid_swa?1.0f:0.0f):
                s_f32_ld_g(gen_addr((int5){slot,0},shared_mask));
            if(chunk==0)s_f32_st_g(gen_addr((int5){slot,0},mask),valid);
            if(slot>=128) {
                const bfloat128 out=v_bf16_ld_tnsr_b((int5){chunk*128,slot,0},shared_rows);
                v_bf16_st_tnsr((int5){chunk*128,slot,0},rows,out);
                const float128 restored=convert_bfloat128_to_float128(out,SW_LINEAR);
                v_f32_st_tnsr((int5){chunk*128,slot,0},values,restored.v1);
                v_f32_st_tnsr((int5){chunk*128+64,slot,0},values,restored.v2);
                continue;
            }
            if(slot==127 && position>=0) {
                bfloat128 cached[4];float128 squares={0};
                for(int tile=0;tile<4;++tile) {
                    cached[tile]=v_bf16_ld_tnsr_b((int5){tile*128,0},input);
                    squares=v_bf16_mac_acc32_b(cached[tile],cached[tile],squares,(e_no_negation)<<1);
                }
                const float64 rrms=positive_rsqrt(row_sum(squares.v1+squares.v2)*inverse_width+epsilon);
                const float128 w=v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b((int5){chunk*128,0},weight));
                float128 x=v_convert_bf16_to_f32_all_b(cached[chunk]);
                x.v1=(x.v1*rrms)*w.v1;x.v2=(x.v2*rrms)*w.v2;
                float64_pair_t linear=kv_bf16_to_f32_linear(v_convert_f32_to_bf16_all_b(x));
                if(chunk==3) {
                    float128 rotated={0};rotated.v1=dsv4_qkv_apply_pairwise_rope_f32(linear.v2,phase,position);
                    linear.v2=convert_bfloat128_to_float128(convert_float128_to_bfloat128(rotated,SW_RHNE|SW_LINEAR),SW_LINEAR).v1;
                }
                for(int part=0;part<4;++part) {
                    const float64 half_values=part<2?linear.v1:linear.v2;
                    const float64 selected=(part&1)?v_f32_mov_dual_group_all_b(half_values,
                        0xffffffff,2,3,2,3,MkWrA(3,3,3,3),0):half_values;
                    const float64 number=v_f32_sel_less_i32_b((int64)V_LANE_ID_32,32,selected,(float64)0);
                    const int group=chunk*4+part;
                    const bfloat128 encoded=swa_pack_number(number,swa,group,position&255,512,rows,-1);
                    const bfloat128 out=valid_swa?encoded:(bfloat128)0;
                    v_bf16_st_tnsr_partial((int5){group*32,slot,0},rows,out,31,0);
                    v_f32_st_tnsr_partial((int5){group*32,slot,0},values,
                        convert_bfloat128_to_float128(out,SW_LINEAR).v1,31,0);
                }
                continue;
            }
            uchar256 scale_bytes={0};
            if(valid_swa)scale_bytes=v_u8_ld_tnsr_partial_b((int5){512,ring},swa,15,0);
            for(int half_chunk=chunk*2;half_chunk<chunk*2+2;++half_chunk) {
                bfloat128 out={0};
                if(valid_swa) {
                    const uchar256 bytes=v_u8_ld_tnsr_partial_b((int5){half_chunk*64,ring},swa,63,0);
                    const uint64 code=convert_uchar256_to_uint256(bytes,SW_LINEAR).v1;
                    const uchar256 scales=v_u8_shuffle_b(scale_bytes,directions+(uchar256)(half_chunk*2),0,scale_bytes);
                    float128 expanded={0};expanded.v1=e4m3fn(code)*ue8m0(convert_uchar256_to_uint256(scales,SW_LINEAR).v1);
                    out=convert_float128_to_bfloat128(expanded,SW_RHNE|SW_LINEAR);
                }
                v_bf16_st_tnsr_partial((int5){half_chunk*64,slot,0},rows,out,63,0);
                v_f32_st_tnsr((int5){half_chunk*64,slot,0},values,convert_bfloat128_to_float128(out,SW_LINEAR).v1);
            }
        }
    }
}
