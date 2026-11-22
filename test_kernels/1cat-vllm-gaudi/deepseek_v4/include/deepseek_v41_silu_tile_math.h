// SPDX-License-Identifier: Apache-2.0
// Reuses FlashInfer-Gaudi's row statistics and FP8 packing, with V4.1
// FP32 clamp/SwiGLU/routing and explicit BF16 projection/activation boundaries.
static inline float64 round_bf16(float64 value) {
    float64_pair_t pair;
    pair.v1 = value;
    pair.v2 = value;
    return v_convert_bf16_to_f32_all_b(v_convert_f32_to_bf16_all_b(pair)).v1;
}

static inline float64 reciprocal_without_lookup(float64 value) {
    // Positive normal scales are bounded below by epsilon/240 and above
    // by BF16_MAX/240. Three Newton steps preserve the BF16-rounded result.
    float64 estimate = as_float64((int64)0x7ef311c3 - as_int64(value));
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    estimate = estimate * (2.0f - value * estimate);
    return v_f32_sel_eq_f32_b(value, as_float64((int64)0x7f800000), 0.0f, estimate);
}

static inline float64 row_max_without_lookup(float64 value) {
    // Fold the four dual groups and their two groups. Unlike the generic
    // reduction helper, explicit lane broadcasts do not reserve the LUT cache.
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(value, 0xffffffff, 1, 0, 3, 2,
                                                      MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_dual_group_all_b(value, 0xffffffff, 2, 3, 0, 1,
                                                      MkWrA(3, 3, 3, 3), 0));
    value = v_f32_max_b(value, v_f32_mov_group_b(value, 0xffffffff, 63, 0));
    float64 result = 0;
    #pragma loop_unroll(8)
    for (int lane = 0; lane < 8; ++lane) {
        const uchar256 broadcast = 0x80 | lane;
        result = v_f32_max_b(result, v_f32_shuffle_b(value, broadcast, 0, value));
    }
    return result;
}

static inline float64 projected_at(tensor product, tensor channel, int product_column, int n, int row,
                                   int expert, float sx, bool valid) {
    const float64 acc = v_f32_ld_tnsr_b((int5){product_column, 0, row}, product);
    const uint64 bits = v_u32_ld_tnsr_b((int5){n % 256, n / 256, expert}, channel,
        SW_UNPACK | SW_UNPCK_16_TO_32, (uint64){0}, valid) << 16;
    return round_bf16(v_f32_mul_b(v_f32_mul_b(acc, *((float64*)&bits)), sx));
}

static inline float64 projected(tensor product, tensor channel, int n, int row,
                                int expert, float sx, bool valid) {
    return projected_at(product, channel, n, n, row, expert, sx, valid);
}

static inline float64 activated_half(tensor product, tensor channel, int n, int row,
                                     int expert, int width, float sx, float route, bool valid) {
    float64 gate = projected(product, channel, n, row, expert, sx, valid);
    float64 up = projected(product, channel, n + width, row, expert, sx, valid);
    gate = v_f32_min_b(gate, 10.0f);
    up = v_f32_max_b(v_f32_min_b(up, 10.0f), -10.0f);
    const float64 silu = v_f32_mul_b(gate, v_sigmoid_f32(gate));
    return v_f32_mul_b(v_f32_mul_b(silu, up), route);
}
