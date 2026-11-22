// SPDX-License-Identifier: Apache-2.0
#include "deepseek_v41_attention_interleave.h"

void main(tensor value, tensor positions, tensor phase_table, tensor output) {
    const int5 begin = get_index_space_offset(), end = begin + get_index_space_size();
    for (int token = begin[2]; token < end[2]; ++token) {
        const int position = s_i32_ld_g(gen_addr((int5){token,0,0,0,0}, positions));
        const float64 phase = v_f32_ld_tnsr_b((int5){0,position,0,0,0}, phase_table);
        for (int group = begin[1]; group < end[1]; ++group) {
            for (int block = begin[0]; block < end[0]; ++block) {
                float64 x[4];
                #pragma clang loop unroll(full)
                for (int part = 0; part < 2; ++part) {
                    int5 at = {block*256+part*128,group,token,0,0};
                    float128 v;
                    v.v1 = v_f32_ld_tnsr_b(at, value);
                    at[0] += 64;
                    v.v2 = v_f32_ld_tnsr_b(at, value);
                    // Preserve the original PV -> BF16 boundary before RoPE.
                    v = convert_bfloat128_to_float128(convert_float128_to_bfloat128(v, SW_RHNE | SW_LINEAR), SW_LINEAR);
                    if (block >= 14) {
                        const int pair = (block - 14) * 16 + part * 8;
                        v.v1 = interleaved_rope(v.v1, phase, pair, 1);
                        v.v2 = interleaved_rope(v.v2, phase, pair + 4, 1);
                        // Preserve inverse-RoPE -> BF16 before group-32 quantization.
                        v = convert_bfloat128_to_float128(convert_float128_to_bfloat128(v, SW_RHNE | SW_LINEAR), SW_LINEAR);
                    }
                    x[2*part] = v.v1; x[2*part+1] = v.v2;
                }
#ifdef DSV41_INTERLEAVED_GROUP32
                float64 maximum = 0.0f, any_nan = 0.0f;
                #pragma clang loop unroll(full)
                for (int part = 0; part < 4; ++part) {
                    const float64 absolute = v_f32_abs_b(x[part]);
                    maximum = v_f32_max_b(maximum, absolute);
                    any_nan = v_f32_max_b(any_nan, v_f32_sel_grt_u32_b(as_uint64(absolute), 0x7f800000, 1.0f, 0.0f));
                }
                maximum = v_f32_max_b(head_maximum(maximum), 1.0e-4f);
                maximum = v_f32_sel_grt_f32_b(head_maximum(any_nan), 0.0f, 1.0e-4f, maximum);
#endif
                #pragma clang loop unroll(full)
                for (int part = 0; part < 2; ++part) {
                    float128 result;
#ifdef DSV41_INTERLEAVED_GROUP32
                    result.v1 = group32_roundtrip(x[2*part], maximum);
                    result.v2 = group32_roundtrip(x[2*part+1], maximum);
#else
                    result.v1 = x[2*part];
                    result.v2 = x[2*part+1];
#endif
                    v_bf16_st_tnsr((int5){block*256+part*128,token,group,0,0}, output,
                        convert_float128_to_bfloat128(result, SW_RHNE | SW_LINEAR));
                }
            }
        }
    }
}
