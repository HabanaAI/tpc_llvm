// SPDX-License-Identifier: Apache-2.0
// Exact checkpoint KV codecs shared by selection and incremental state writes.
#pragma once

static inline float64 e4m3fn(uint64 code) {
    const uint64 exponent = (code >> 3) & 15;
    const uint64 mantissa = code & 7;
    const uint64 normal_bits = ((exponent + 120) << 23) | (mantissa << 20);
    const float64 tiny = convert_uint64_to_float64(mantissa, 0) * 0.001953125f;
    float64 value = v_f32_sel_eq_u32_b(exponent, 0, tiny, as_float64(normal_bits));
    const uint64 sign = (code & 128) << 24;
    value = as_float64(as_uint64(value) | sign);
    const uint64 nan_bits = 0x7fffffff;
    value = v_f32_sel_eq_u32_b(code & 127, 127, as_float64(nan_bits), value);
    return v_f32_sel_eq_f32_b(value, 0.0f, 0.0f, value);
}

static inline float64 ue8m0(uint64 code) {
    uint64 bits = code << 23;
    bits = v_u32_sel_eq_u32_b(code, 0, 0x00400000, bits);
    bits = v_u32_sel_eq_u32_b(code, 255, 0x7fffffff, bits);
    return as_float64(bits);
}
