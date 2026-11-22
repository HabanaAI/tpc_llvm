// SPDX-License-Identifier: Apache-2.0
// Static ISA candidate only; no production GUID selects this implementation.
// The projection and BF16 activation boundaries stay in the parent kernel.
static inline float64 sigmoid_polynomial(float64 gate) {
    const float64 input = -v_f32_min_b(v_f32_abs_b(gate), 87.0f);
    float64 power = v_f32_mac_b(input, 1.44269504088896341f, 0.5f, 0);
    power = v_f32_nearbyint_b(power, SW_RD);
    float64 residual = v_f32_mac_b(power, 0.693359375f, input, SW_NEG);
    residual = v_f32_mac_b(power, -2.12194440e-4f, residual, SW_NEG);
    float64 exponential = v_f32_mac_b(residual, 1.0f / 720.0f, 1.0f / 120.0f, 0);
    exponential = v_f32_mac_b(exponential, residual, 1.0f / 24.0f, 0);
    exponential = v_f32_mac_b(exponential, residual, 1.0f / 6.0f, 0);
    exponential = v_f32_mac_b(exponential, residual, 0.5f, 0);
    exponential = v_f32_mac_b(exponential, residual, 1.0f, 0);
    exponential = v_f32_mac_b(exponential, residual, 1.0f, 0);
    int64 bits = as_int64(exponential) + (v_convert_f32_to_i32_b(power, SW_RD) << 23);
    exponential = as_float64(bits);
    exponential = v_f32_sel_geq_f32_b(v_f32_abs_b(gate), 87.0f, 0.0f, exponential);
    const float64 denominator = 1.0f + exponential;
    float64 inverse = as_float64((int64)0x7ef311c3 - as_int64(denominator));
    inverse = inverse * (2.0f - denominator * inverse);
    inverse = inverse * (2.0f - denominator * inverse);
    inverse = inverse * (2.0f - denominator * inverse);
    return v_f32_sel_less_f32_b(gate, 0.0f, exponential * inverse, inverse);
}

#define v_sigmoid_f32(value) sigmoid_polynomial(value)
#include "../../deepseek_v4/kernels/deepseek_v41_expert_n256_silu_quant_gaudi2.c"
