// SPDX-License-Identifier: Apache-2.0
#ifndef DSV41_SCORE_KEY_BITS
#define DSV41_SCORE_KEY_BITS 16
#endif
// Index scores retain a BF16 boundary. Canonicalize zero and rank NaNs above
// infinity, as top-k does; retain the original score bits in the output.
static inline uint64 dsv41_score_key(float64 score) {
#if DSV41_SCORE_KEY_BITS == 32
    uint64 bits = as_uint64(score);
    const uint64 magnitude = bits & 0x7fffffffu;
    bits = v_u32_sel_eq_u32_b(magnitude, 0, (uint64)0, bits);
    uint64 key = v_u32_sel_grt_u32_b(bits, 0x7fffffffu, ~bits, bits ^ 0x80000000u);
    return v_u32_sel_grt_u32_b(magnitude, 0x7f800000u, (uint64)0xffffffffu, key);
#else
    uint64 bits = as_uint64(score) >> 16;
    const uint64 magnitude = bits & 32767;
    bits = v_u32_sel_eq_u32_b(magnitude, 0, (uint64)0, bits);
    uint64 key = v_u32_sel_grt_u32_b(bits, 32767, (~bits) & 65535, bits ^ 32768);
    return v_u32_sel_grt_u32_b(magnitude, 32640, (uint64)65535, key);
#endif
}
