/**********************************************************************
Copyright (c) 2026 Habana Labs. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the conditions in the project
license are met.
********************************************************************/

typedef struct {
    unsigned int low;
    unsigned int high;
} split_u64;

static inline split_u64 multiply_low_u64(
    unsigned int value,
    unsigned int multiplier_low,
    unsigned int multiplier_high)
{
    const unsigned int mask = 0xffffU;
    const unsigned int value_low = value & mask;
    const unsigned int value_high = value >> 16;
    const unsigned int multiplier_0 = multiplier_low & mask;
    const unsigned int multiplier_1 = multiplier_low >> 16;
    const unsigned int multiplier_2 = multiplier_high & mask;
    const unsigned int multiplier_3 = multiplier_high >> 16;

    unsigned int partial = value_low * multiplier_0;
    const unsigned int digit_0 = partial & mask;
    unsigned int carry = partial >> 16;
    partial = value_low * multiplier_1 + value_high * multiplier_0 + carry;
    const unsigned int digit_1 = partial & mask;
    carry = partial >> 16;
    partial = value_low * multiplier_2 + value_high * multiplier_1 + carry;
    const unsigned int digit_2 = partial & mask;
    carry = partial >> 16;
    partial = value_low * multiplier_3 + value_high * multiplier_2 + carry;

    split_u64 result;
    result.low = digit_0 | (digit_1 << 16);
    result.high = digit_2 | ((partial & mask) << 16);
    return result;
}

static inline unsigned int remainder_u64(
    split_u64 value,
    unsigned int divisor)
{
    unsigned int remainder = 0;
    for (int bit = 31; bit >= 0; --bit) {
        remainder = (remainder << 1) | ((value.high >> bit) & 1U);
        if (remainder >= divisor) {
            remainder -= divisor;
        }
    }
    for (int bit = 31; bit >= 0; --bit) {
        remainder = (remainder << 1) | ((value.low >> bit) & 1U);
        if (remainder >= divisor) {
            remainder -= divisor;
        }
    }
    return remainder;
}

static inline bfloat128 decode_e4m3fn(ushort128 code)
{
    const ushort128 exponent = (code >> 3) & 15;
    const ushort128 mantissa = code & 7;
    ushort128 tiny = (mantissa << 5) + 0x3b80;
    tiny = v_u16_sel_less_u16_b(
        mantissa, 4, (mantissa << 6) + 0x3b00, tiny);
    tiny = v_u16_sel_eq_u16_b(mantissa, 1, 0x3b00, tiny);
    tiny = v_u16_sel_eq_u16_b(mantissa, 0, 0, tiny);
    ushort128 bits = ((exponent + 120) << 7) | (mantissa << 4);
    bits = v_u16_sel_eq_u16_b(exponent, 0, tiny, bits);
    bits |= (code & 128) << 8;
    bits = v_u16_sel_eq_u16_b(code & 127, 0, 0, bits);
    bits = v_u16_sel_eq_u16_b(code & 127, 127, 0x7fff, bits);
    return *((bfloat128*)&bits);
}

static inline bfloat128 decode_ue8m0(ushort128 code)
{
    ushort128 bits = code << 7;
    bits = v_u16_sel_eq_u16_b(code, 0, 0, bits);
    bits = v_u16_sel_eq_u16_b(code, 255, 0x7fff, bits);
    return *((bfloat128*)&bits);
}

#ifdef DSV41_ENGRAM_BATCH
// Derive each row's lookback from the fixed initial history. Work items do
// not consume another work item's history store.
static inline int compressed_token(tensor raw_token, tensor token_map, int index)
{
    const int raw = s_i32_ld_g(gen_addr((int5){index, 0, 0, 0, 0}, raw_token));
    if (raw == 129264 || raw == 129265) return -1;
    return s_i32_ld_g(gen_addr((int5){raw, 0, 0, 0, 0}, token_map));
}
static inline int lookback(tensor raw_token, tensor token_map, tensor history, int index, int slot)
{
    if (index > slot) return compressed_token(raw_token, token_map, index - slot - 1);
    return s_i32_ld_g(gen_addr((int5){slot - index, 0, 0, 0, 0}, history));
}
#endif

void main(
    tensor raw_token,
    tensor history,
    tensor token_map,
    tensor parameters,
    tensor weights,
    tensor scales,
    tensor decoded_rows,
    tensor next_history)
{
#ifdef DSV41_ENGRAM_BATCH
    const int5 offset = get_index_space_offset();
    const int5 end = offset + get_index_space_size();
    for (int token_index = offset[1]; token_index < end[1]; ++token_index) {
    for (int local_head = offset[0]; local_head < end[0]; ++local_head) {
#else
    const int token_index = 0;
    const int local_head = get_index_space_offset()[0];
#endif
    const int raw = s_i32_ld_g(
        gen_addr((int5){token_index, 0, 0, 0, 0}, raw_token));
    int compressed = s_i32_ld_g(
        gen_addr((int5){raw, 0, 0, 0, 0}, token_map));
    if (raw == 129264 || raw == 129265) {
        compressed = -1;
    }
    const int pad = s_i32_ld_g(
        gen_addr((int5){0, 0, 0, 0, 0}, parameters));
    const int head_start = s_i32_ld_g(
        gen_addr((int5){1, 0, 0, 0, 0}, parameters));
    const int row_start = s_i32_ld_g(
        gen_addr((int5){2, 0, 0, 0, 0}, parameters));
    const int global_head = head_start + local_head;
    const int selected_shift = global_head / 8 + 1;

    split_u64 state = (split_u64){0, 0};
    int blocked = compressed < 0;
    for (int shift = 0; shift <= selected_shift; ++shift) {
        int value = blocked ? pad : compressed;
        if (shift > 0) {
#ifdef DSV41_ENGRAM_BATCH
            value = lookback(raw_token, token_map, history, token_index, shift - 1);
#else
            value = s_i32_ld_g(
                gen_addr((int5){shift - 1, 0, 0, 0, 0}, history));
#endif
            blocked |= value < 0;
            value = blocked ? pad : value;
        }
        const unsigned int multiplier_low =
            (unsigned int)s_i32_ld_g(gen_addr(
                (int5){3 + shift * 2, 0, 0, 0, 0}, parameters));
        const unsigned int multiplier_high =
            (unsigned int)s_i32_ld_g(gen_addr(
                (int5){4 + shift * 2, 0, 0, 0, 0}, parameters));
        const split_u64 product = multiply_low_u64(
            (unsigned int)value, multiplier_low, multiplier_high);
        state.low ^= product.low;
        state.high ^= product.high;
    }

    const unsigned int prime = (unsigned int)s_i32_ld_g(
        gen_addr((int5){11 + local_head, 0, 0, 0, 0}, parameters));
    const unsigned int offset = (unsigned int)s_i32_ld_g(
        gen_addr((int5){23 + local_head, 0, 0, 0, 0}, parameters));
    const unsigned int two64_mod = (unsigned int)s_i32_ld_g(
        gen_addr((int5){35 + local_head, 0, 0, 0, 0}, parameters));
    unsigned int remainder = remainder_u64(state, prime);
    if ((state.high & 0x80000000U) != 0) {
        remainder = remainder >= two64_mod ?
            remainder - two64_mod : remainder + prime - two64_mod;
    }
    const int row = (int)(remainder + offset) - row_start;

    const uchar256 raw_scales = v_u8_ld_tnsr_partial_b(
        (int5){0, row, 0, 0, 0}, scales, 7, 0);
    const uchar256 row_scales = v_u8_mov_dual_group_all_b(
        raw_scales,
        0xffffffff,
        0,
        0,
        0,
        0,
        MkWrA(3, 3, 3, 3),
        (uchar256){0});
    const uchar256 lanes = V_LANE_ID_8;
    for (int chunk = 0; chunk < 2; ++chunk) {
        const uchar256 bytes = v_u8_ld_tnsr_partial_b(
            (int5){chunk * 128, row, 0, 0, 0}, weights, 127, 0);
        const ushort128 code =
            convert_uchar256_to_ushort256(bytes, SW_LINEAR).v1;
        const uchar256 directions =
            ((lanes >> 5) + chunk * 4) | 0x80;
        const uchar256 scale_bytes = v_u8_shuffle_b(
            row_scales, directions, 0, (uchar256){0});
        const ushort128 scale_code =
            convert_uchar256_to_ushort256(scale_bytes, SW_LINEAR).v1;
        const bfloat128 value = v_bf16_mul_b(
            decode_e4m3fn(code), decode_ue8m0(scale_code));
        v_bf16_st_tnsr(
            (int5){chunk * 128, local_head, token_index, 0, 0},
            decoded_rows,
            value);
    }

    if (local_head == 0) {
#ifdef DSV41_ENGRAM_BATCH
        if (token_index == 0) {
            for (int slot = 0; slot < 3; ++slot)
                s_i32_st_g(gen_addr((int5){slot, 0, 0, 0, 0}, next_history),
                           s_i32_ld_g(gen_addr((int5){slot, 0, 0, 0, 0}, history)));
        }
        s_i32_st_g(gen_addr((int5){0, token_index + 1, 0, 0, 0}, next_history), compressed);
        for (int slot = 0; slot < 2; ++slot)
            s_i32_st_g(gen_addr((int5){slot + 1, token_index + 1, 0, 0, 0}, next_history),
                       lookback(raw_token, token_map, history, token_index, slot));
#else
        s_i32_st_g(
            gen_addr((int5){0, 0, 0, 0, 0}, next_history), compressed);
        s_i32_st_g(
            gen_addr((int5){1, 0, 0, 0, 0}, next_history),
            s_i32_ld_g(gen_addr((int5){0, 0, 0, 0, 0}, history)));
        s_i32_st_g(
            gen_addr((int5){2, 0, 0, 0, 0}, next_history),
            s_i32_ld_g(gen_addr((int5){1, 0, 0, 0, 0}, history)));
    #endif
    }
#ifdef DSV41_ENGRAM_BATCH
    }
    }
#endif
}
