// SPDX-License-Identifier: Apache-2.0
#pragma clang fp contract(off)
#define DSV4_QNORM_HELPERS_ONLY 1
#define DSV4_ROPE_SECOND_TERM_FMA 1
#include "deepseek_v4_qnorm_rope_kv_pack_bf16.h"
static inline ushort128 group_max(ushort128 value) {
    value = v_u16_max_b(value, v_u16_mov_group_b(value, 0xffffffff, 63, 0));
    const uchar256 lane = read_lane_id_1b_b();
    #pragma loop_unroll(4)
    for (int offset = 2; offset <= 16; offset *= 2) {
        const uchar256 selector = ((lane ^ offset) & 31) | 0x80;
        const uchar256 shuffled = v_u8_shuffle_b(*((uchar256*)&value), selector, 0, (uchar256)0);
        value = v_u16_max_b(value, *((ushort128*)&shuffled));
    }
    return value;
}
static inline float64 round_half(float64 number, float64 scale) {
    const float64 mag = v_f32_abs_b(number);
    int64 code = v_i32_sel_grt_f32_b(mag, scale * 0.25f, 1, 0);
    code += v_i32_sel_geq_f32_b(mag, scale * 0.75f, 1, 0);
    code += v_i32_sel_grt_f32_b(mag, scale * 1.25f, 1, 0);
    code += v_i32_sel_geq_f32_b(mag, scale * 1.75f, 1, 0);
    code += v_i32_sel_grt_f32_b(mag, scale * 2.5f, 1, 0);
    code += v_i32_sel_geq_f32_b(mag, scale * 3.5f, 1, 0);
    code += v_i32_sel_grt_f32_b(mag, scale * 5.0f, 1, 0);
    code = v_i32_sel_eq_f32_b(mag, 0.0f, 0, code);
    const float64 c = convert_int64_to_float64(code, 0);
    float64 decoded = v_f32_sel_less_i32_b(code, 6, c - 2.0f, c * 2.0f - 8.0f);
    decoded = v_f32_sel_less_i32_b(code, 4, c * 0.5f, decoded);
    decoded *= scale;
    return as_float64(as_uint64(decoded) | (as_uint64(number) & 0x80000000));
}
static inline bfloat128 fp4_four_groups(bfloat128 value) {
    const ushort128 raw = *((ushort128*)&value);
    const ushort128 mag = raw & 0x7fff;
    const ushort128 nan_first = v_u16_sel_eq_u16_b(read_lane_id_2b_b() & 31, 0,
                                      v_u16_sel_grt_u16_b(mag, 0x7f80, 1, 0), 0);
    ushort128 maximum = group_max(v_u16_sel_grt_u16_b(mag, 0x7f80, 0, mag));
    maximum = v_u16_max_b(maximum, 0x01c0);
    maximum = v_u16_sel_grt_u16_b(group_max(nan_first), 0, 0x01c0, maximum);
    const ushort128 scale_code = (maximum >> 7) - 2 + v_u16_sel_grt_u16_b(maximum & 127, 64, 1, 0);
    const ushort128 scale_bits = scale_code << 7;
    const float128 scale = convert_bfloat128_to_float128(*((bfloat128*)&scale_bits), SW_LINEAR);
    const float128 number = convert_bfloat128_to_float128(value, SW_LINEAR);
    float128 result = {0};
    result.v1 = round_half(number.v1, scale.v1);
    result.v2 = round_half(number.v2, scale.v2);
    return convert_float128_to_bfloat128(result, SW_RHNE | SW_LINEAR);
}
void main(tensor value, tensor positions, tensor phase, tensor output) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin + get_index_space_size();
    for (int token=begin[2]; token<end[2]; ++token) {
        const int position = s_i32_ld_g(gen_addr((int5){token,0,0,0,0}, positions));
        for (int head=begin[1]; head<end[1]; ++head) {
            const int5 at = {0,head,token,0,0};
            const bfloat128 original = v_bf16_ld_tnsr_b(at, value);
            float128 expanded = convert_bfloat128_to_float128(original, SW_LINEAR);
            expanded.v2 = dsv4_qkv_apply_pairwise_rope_f32(expanded.v2, phase, position);
            const bfloat128 roped = convert_float128_to_bfloat128(expanded, SW_RHNE | SW_LINEAR);
            const ushort128 combined = v_u16_sel_less_u16_b(read_lane_id_2b_b(), 64,
                                              *((ushort128*)&original), *((ushort128*)&roped));
            v_bf16_st_tnsr(at, output, fp4_four_groups(*((bfloat128*)&combined)));
        }
    }
}
