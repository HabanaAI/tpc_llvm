// SPDX-License-Identifier: Apache-2.0
// Four independent group-32 codecs share a BF16 vector. Positive BF16 bit
// patterns preserve magnitude order, with NaNs sorting above infinity.
static inline bfloat128 v41_group32_roundtrip_bf16(bfloat128 value) {
    const ushort128 original = *((ushort128*)&value);
    const ushort128 magnitude = original & 0x7fff;
    ushort128 maximum = v_u16_max_b(magnitude, v_u16_mov_group_b(magnitude, 0xffffffff, 63, 0));
    const uchar256 lane = read_lane_id_1b_b();
    #pragma loop_unroll(4)
    for (int offset = 2; offset <= 16; offset *= 2) {
        const uchar256 selector = ((lane ^ offset) & 31) | 0x80;
        const uchar256 shuffled = v_u8_shuffle_b(*((uchar256*)&maximum), selector, 0, (uchar256)0);
        maximum = v_u16_max_b(maximum, *((ushort128*)&shuffled));
    }
    // The BF16 value below 1e-4 selects the same scale exponent as the FP32
    // floor. A group containing NaN retains the original minimum-scale rule.
    maximum = v_u16_max_b(maximum, 0x38d1);
    maximum = v_u16_sel_grt_u16_b(maximum, 0x7f80, 0x38d1, maximum);
    const short128 exponent = convert_ushort128_to_short128(maximum >> 7, 0) - 135
        + v_i16_sel_grt_u16_b(maximum & 127, 96, 1, 0);
    const short128 input_exponent = convert_ushort128_to_short128(magnitude >> 7, 0);
    // Adding a positive BF16 offset places the subnormal grid in one
    // normal binade. The first add rounds to that grid; subtracting the
    // offset is exact. Both operations retain round-to-nearest-even.
    const ushort128 magic_bits = (ushort128)((exponent + 125) << 7);
    const bfloat128 magic = *((bfloat128*)&magic_bits);
    const bfloat128 positive = *((bfloat128*)&magnitude);
    const bfloat128 shifted = v_bf16_add_b(positive, magic);
    const bfloat128 tiny = v_bf16_sub_b(shifted, magic);
    // Keep three mantissa bits with ties to even, then saturate at 448*scale.
    const ushort128 rounded = (magnitude + 7 + ((magnitude >> 4) & 1)) & 0xfff0;
    ushort128 result = v_u16_sel_less_i16_b(input_exponent, exponent + 121,
                                            *((ushort128*)&tiny), rounded);
    const ushort128 cap = v_u16_min_b((ushort128)(((exponent + 135) << 7) + 96), 0x7f80);
    result = v_u16_min_b(result, cap) | (original & 0x8000);
    result = v_u16_sel_eq_u16_b(result & 0x7fff, 0, 0, result);
    result = v_u16_sel_grt_u16_b(magnitude, 0x7f80, 0x7fff, result);
    return *((bfloat128*)&result);
}
