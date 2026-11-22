// SPDX-License-Identifier: Apache-2.0
// V4.1 direct indexed MXFP4 -> BF16 MAC path.
//
// Q16 is the prepared N-major layout: one 256-byte unpack feeds the same
// K pair for 128 output rows.  The kernel therefore computes a complete
// 128-row block in registers and never materializes decoded weights.

#ifndef DSV41_MXFP4_INDEXED_FC1
#define DSV41_MXFP4_INDEXED_FC1 0
#endif
#ifndef DSV41_MXFP4_INDEXED_FC2
#define DSV41_MXFP4_INDEXED_FC2 0
#endif
#ifndef DSV41_MXFP4_INDEXED_NORMAL
#define DSV41_MXFP4_INDEXED_NORMAL 0
#endif

#define DSV41_TOPK 6
#define DSV41_HIDDEN 5120
#define DSV41_INTERMEDIATE 1152
#define DSV41_W13_BLOCKS (2 * DSV41_INTERMEDIATE / 128)
#define DSV41_W2_BLOCKS (DSV41_HIDDEN / 128)

static inline ushort128 dsv41_decode_bits(ushort128 nibble, ushort128 scale)
{
    const ushort128 magnitude = v_u16_and_b(nibble, 7);
    const ushort128 sign = v_u16_shl_b(v_u16_and_b(nibble, 8), 12);
    ushort128 bits = scale + v_u16_shl_b(magnitude, 6) - 128;
    bits = v_u16_sel_eq_u16_b(magnitude, 1, bits - 64, bits);
    bits = v_u16_min_b(bits, 0x7f80);
#if !DSV41_MXFP4_INDEXED_NORMAL
    const ushort128 scale_code = v_u16_shr_b(scale, 7);
    const ushort128 subnormal = v_u16_sel_less_u16_b(
        magnitude, 4, v_u16_shl_b(magnitude, 5), bits);
    bits = v_u16_sel_eq_u16_b(scale_code, 0, subnormal, bits);
    const ushort128 half_subnormal = v_u16_sel_eq_u16_b(
        magnitude, 1, 0x0040, bits);
    bits = v_u16_sel_eq_u16_b(scale_code, 1, half_subnormal, bits);
#endif
    // Zero magnitude must be resolved before the exceptional scale code.  This
    // matches the prepared decoder's zero-times-NaN contract for MXFP4 code 0.
    bits = v_u16_sel_eq_u16_b(magnitude, 0, 0, bits);
#if !DSV41_MXFP4_INDEXED_NORMAL
    bits = v_u16_sel_eq_u16_b(scale_code, 255, 0x7fc0, bits);
#endif
    return v_u16_or_b(bits, sign);
}

static inline bfloat256 dsv41_lookup_pair(
    uchar256 unpacked, bfloat128 scale, uchar256 table)
{
    const uchar256 directions = v_u8_or_b(unpacked, 0x80);
    const uchar256 fp8_bits = v_u8_shuffle_b(table, directions, 0,
                                             (uchar256){0});
    const bfloat256 values = v_convert_f8_to_bf16_all_b(
        *((minifloat256*)&fp8_bits));
    const ushort128 negativeZeroBits = 0x8000;
    const bfloat128 negativeZero = *((bfloat128*)&negativeZeroBits);
    bfloat256 result;
    result.v1 = v_bf16_madd_b(values.v1, scale, negativeZero);
    result.v2 = v_bf16_madd_b(values.v2, scale, negativeZero);
    return result;
}

typedef struct {
    bfloat128 first;
    bfloat128 second;
} dsv41_pair;

static inline dsv41_pair dsv41_load_pair(
    tensor q16, tensor s16, tensor lookup, int pair, int block, int expert)
{
    const int group = pair / 16;
    const bfloat128 scale = v_bf16_ld_tnsr_b(
        (int5){group * 128, block, expert, 0, 0}, s16);
    const uchar256 unpacked = v_u8_ld_tnsr_b(
        (int5){pair * 64, block, expert, 0, 0}, q16,
        SW_UNPACK | SW_UNPCK_4_TO_8);
#if DSV41_MXFP4_INDEXED_NORMAL
    const bfloat256 decoded = dsv41_lookup_pair(unpacked, scale,
                                                 v_u8_ld_tnsr_b(
                                                     (int5){0, 0, 0, 0, 0},
                                                     lookup));
    dsv41_pair result = {decoded.v1, decoded.v2};
    return result;
#else
    const ushort128 words = *((ushort128*)&unpacked);
    const ushort128 nibble0 = v_u16_and_b(words, 15);
    const ushort128 nibble1 = v_u16_and_b(v_u16_shr_b(words, 8), 15);
    const ushort128 scale_bits = *((ushort128*)&scale);
    const ushort128 bits0 = dsv41_decode_bits(nibble0, scale_bits);
    const ushort128 bits1 = dsv41_decode_bits(nibble1, scale_bits);
    dsv41_pair result = {*((bfloat128*)&bits0), *((bfloat128*)&bits1)};
    return result;
#endif
}

static inline bfloat128 dsv41_hidden_broadcast(tensor hidden, int k, int token)
{
    const bf16 value = s_bf16_ld_g(
        gen_addr((int5){k, token, 0, 0, 0}, hidden));
    return (bfloat128)(float)value;
}

#if DSV41_MXFP4_INDEXED_FC1
void main(tensor hidden, tensor expert_ids, tensor q16, tensor s16,
          tensor lookup, tensor output)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int token = start[2]; token < end[2]; ++token) {
        for (int selected = start[1]; selected < end[1]; ++selected) {
            const int expert = s_i32_ld_g(gen_addr(
                (int5){selected, token, 0, 0, 0}, expert_ids));
            if (expert < 0 || expert >= get_dim_size(q16, 2)) {
                for (int block = start[0]; block < end[0]; ++block) {
                    const int row = block * 128;
                    v_bf16_st_tnsr((int5){row, selected, token, 0, 0},
                                   output, (bfloat128){0});
                    v_bf16_st_tnsr((int5){row + DSV41_INTERMEDIATE, selected,
                                          token, 0, 0}, output, (bfloat128){0});
                }
                continue;
            }
            for (int block = start[0]; block < end[0]; ++block) {
                float128 gate_accum = {0};
                float128 up_accum = {0};
                const int up_block = block + DSV41_INTERMEDIATE / 128;
                for (int pair = 0; pair < DSV41_HIDDEN / 2; ++pair) {
                    const dsv41_pair gate = dsv41_load_pair(
                        q16, s16, lookup, pair, block, expert);
                    const dsv41_pair up = dsv41_load_pair(
                        q16, s16, lookup, pair, up_block, expert);
                    gate_accum = v_bf16_mac_acc32_b(
                        dsv41_hidden_broadcast(hidden, pair * 2, token),
                        gate.first, gate_accum, 0);
                    gate_accum = v_bf16_mac_acc32_b(
                        dsv41_hidden_broadcast(hidden, pair * 2 + 1, token),
                        gate.second, gate_accum, 0);
                    up_accum = v_bf16_mac_acc32_b(
                        dsv41_hidden_broadcast(hidden, pair * 2, token),
                        up.first, up_accum, 0);
                    up_accum = v_bf16_mac_acc32_b(
                        dsv41_hidden_broadcast(hidden, pair * 2 + 1, token),
                        up.second, up_accum, 0);
                }
                const int row = block * 128;
                const bfloat128 gate = v_convert_f32_to_bf16_all_b(
                    gate_accum, SW_RHNE);
                const bfloat128 up = v_convert_f32_to_bf16_all_b(
                    up_accum, SW_RHNE);
                v_bf16_st_tnsr((int5){row, selected, token, 0, 0}, output, gate);
                v_bf16_st_tnsr((int5){row + DSV41_INTERMEDIATE, selected,
                                      token, 0, 0}, output, up);
            }
        }
    }
}
#elif DSV41_MXFP4_INDEXED_FC2
void main(tensor intermediate, tensor expert_ids, tensor q16, tensor s16,
          tensor lookup, tensor output)
{
    const int5 start = get_index_space_offset();
    const int5 end = start + get_index_space_size();
    for (int token = start[2]; token < end[2]; ++token) {
        for (int selected = start[1]; selected < end[1]; ++selected) {
            const int expert = s_i32_ld_g(gen_addr(
                (int5){selected, token, 0, 0, 0}, expert_ids));
            if (expert < 0 || expert >= get_dim_size(q16, 2)) {
                for (int block = start[0]; block < end[0]; ++block) {
                    v_bf16_st_tnsr((int5){block * 128, selected, token, 0, 0},
                                   output, (bfloat128){0});
                }
                continue;
            }
            for (int block = start[0]; block < end[0]; ++block) {
                float128 accum = {0};
                for (int pair = 0; pair < DSV41_INTERMEDIATE / 2; ++pair) {
                    const dsv41_pair weights = dsv41_load_pair(
                        q16, s16, lookup, pair, block, expert);
                    const bfloat128 x0 = dsv41_hidden_broadcast(
                        intermediate, pair * 2, token);
                    const bfloat128 x1 = dsv41_hidden_broadcast(
                        intermediate, pair * 2 + 1, token);
                    accum = v_bf16_mac_acc32_b(x0, weights.first, accum, 0);
                    accum = v_bf16_mac_acc32_b(x1, weights.second, accum, 0);
                }
                const bfloat128 result = v_convert_f32_to_bf16_all_b(
                    accum, SW_RHNE);
                v_bf16_st_tnsr((int5){block * 128, selected, token, 0, 0},
                               output, result);
            }
        }
    }
}
#endif
