// SPDX-License-Identifier: Apache-2.0
// One work point owns one token/stream. Keep the two RMS statistics and
// weighted dot in FP32, then publish the original BF16 residual boundary.
#pragma clang fp contract(off)
#define FLASHINFER_NORM_USE_LOOKUP_RSQRT
#include "../../flashinfer_gaudi/kernels/norm_math_gaudi2.h"

// Same PRE/POST-SQRT sequence as the qualified router and SDK sqrt kernel.
static inline float64 stock_sqrt(float64 input) {
    float64 result = v_f32_form_fp_num_b(input, input, input, SW_FORCE_SIGN0 | SW_PRE_SQRT_RSQRT);
    const uint64_float64_pair_t lut = v_f32_get_lut_entry_and_interval_start_b(
        result, 17, e_func_variant_sqrt_rsqrt << 13, (uint64_float64_pair_t){0}, 1, 0);
    const float64 reduced = result-lut.v2;
    const float64 c0 = v_f32_lookup_1c(lut.v1, e_fp32_sqrt, SW_BV32, 0, 1, 0);
    const float64_pair_t c1c2 = v_f32_lookup_2c(lut.v1, e_fp32_sqrt, SW_BV32, (float64_pair_t){0}, 1, 0);
    result = v_f32_mac_b(c1c2.v2, reduced, c1c2.v1);
    result = v_f32_mac_b(result, reduced, c0);
    result = v_f32_form_fp_num_b(input, result, result, SW_FORCE_SIGN0 | SW_POST_SQRT);
    const float64 fclass = v_f32_fclass_b(input);
    return v_f32_calc_fp_special_b(fclass, fclass, e_fp_sqrt, result);
}

void main(tensor residual, tensor kv, tensor q_weight, tensor k_weight,
          tensor active_mask, tensor output, float epsilon) {
    const int5 begin = get_index_space_offset();
    const int5 end = begin+get_index_space_size();
    for (int token = begin[1]; token < end[1]; ++token) {
        const bool active = s_i8_ld_g(gen_addr((int5){token,0,0,0,0}, active_mask)) != 0;
        for (int stream = begin[0]; stream < end[0]; ++stream) {
            float128 h_square = {0}, key_square = {0}, dot_sum = {0};
            for (int feature = 0; feature < 5120; feature += 128) {
                const int5 rc = {feature,stream,token,0,0};
                const int5 kc = {stream*5120+feature,token,0,0,0};
                const int5 wc = {feature,stream,0,0,0};
                const float128 h = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc,residual));
                const float128 key = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(kc,kv));
                const float128 qw = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc,q_weight));
                const float128 kw = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(wc,k_weight));
                h_square.v1 += h.v1*h.v1; h_square.v2 += h.v2*h.v2;
                key_square.v1 += key.v1*key.v1; key_square.v2 += key.v2*key.v2;
                dot_sum.v1 += (h.v1*(qw.v1*kw.v1))*key.v1;
                dot_sum.v2 += (h.v2*(qw.v2*kw.v2))*key.v2;
            }
            const float64 rrms_h = positive_rsqrt(row_sum(h_square.v1+h_square.v2)*(1.0f/5120.0f)+epsilon);
            const float64 rrms_k = positive_rsqrt(row_sum(key_square.v1+key_square.v2)*(1.0f/5120.0f)+epsilon);
            const float64 dot = row_sum(dot_sum.v1+dot_sum.v2)*(rrms_h*rrms_k)*0.013975424859373685f;
            const float64 root = stock_sqrt(v_f32_max_b(v_f32_abs_b(dot),1e-6f));
            const float64 signed_root = as_float64(as_uint64(root)|(as_uint64(dot)&0x80000000));
            float64 gate = v_reciprocal_f32(1.0f+v_exp_cephes_f32(-signed_root));
            if (!active) gate = 0;
            for (int feature = 0; feature < 5120; feature += 128) {
                const int5 rc = {feature,stream,token,0,0};
                const int5 vc = {20480+feature,token,0,0,0};
                float128 h = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(rc,residual));
                const float128 value = v_convert_bf16_to_f32_all_b(v_bf16_ld_tnsr_b(vc,kv));
                h.v1 = v_f32_mac_b(gate,value.v1,h.v1);
                h.v2 = v_f32_mac_b(gate,value.v2,h.v2);
                v_bf16_st_tnsr(rc,output,v_convert_f32_to_bf16_all_b(h));
            }
        }
    }
}
