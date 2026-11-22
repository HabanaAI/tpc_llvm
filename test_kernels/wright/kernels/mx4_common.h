// SPDX-License-Identifier: AGPL-3.0-only
// Copyright Aaron Botsis
// Constants and tables of the MXFP4 -> FP8 (E4M3 bias 7, +-240) dequant kernels.
//   S       E8M0 scale byte, one per 32-element block; block k of a vector is in lane group k (32 lanes)
//   T0, T1  magnitude tables for E2M1 mantissa 0 and 1, indexed by e' (5 bits): value (1 + f/2) * 2^(e' - 14),
//           saturated from e' = FP8_SAT_EP; every entry has the sign bit preset (0x80 | magnitude)
//   mask    clears the preset sign for positive codes; code -0 gives 0x00
// S = 0xFF is exponent 128, like any other byte. No path writes an exponent-15 byte.
// A 32-lane group holds two 16-byte block tables (lanes 0..15 and 16..31); the output shuffle indexes a table
// with the block's codes. Every shuffle selector has a bit 5 that is the same over its destination group.
#pragma once
#define FP8_SAT_BYTE 0x77   /* +240; 0xF7 with the preset sign: the largest byte the kernel writes */
#define FP8_SAT_EP   22     /* first table index e' that saturates: 2^(22 - 14) = 256 > 240 */
#define BS 114              /* S <= BS + k: the value is zero */
#define MASK32 ((uchar256)31)

// entry e' = lane & 31
static inline uchar256 build_T0(uchar256 e)
{
    uchar256 t = v_u8_shl_b(v_u8_sub_b(e, (uchar256)7), (char256)3);          // normal: (e' - 7) * 8
    t = v_u8_sel_geq_u8_b(e, (uchar256)FP8_SAT_EP, (uchar256)FP8_SAT_BYTE, t);               // saturated
    const uchar256 sub = v_u8_sel_eq_u8_b(e, (uchar256)7, (uchar256)4,
                         v_u8_sel_eq_u8_b(e, (uchar256)6, (uchar256)2,
                         v_u8_sel_eq_u8_b(e, (uchar256)5, (uchar256)1, (uchar256)0)));
    t = v_u8_sel_less_u8_b(e, (uchar256)8, sub, t);
    return v_u8_or_b(t, (uchar256)0x80);
}
static inline uchar256 build_T1(uchar256 e)
{
    uchar256 t = v_u8_add_b(v_u8_shl_b(v_u8_sub_b(e, (uchar256)7), (char256)3), (uchar256)4); // normal: (e'-7)*8 + 4
    t = v_u8_sel_geq_u8_b(e, (uchar256)FP8_SAT_EP, (uchar256)FP8_SAT_BYTE, t);
    const uchar256 sub = v_u8_sel_eq_u8_b(e, (uchar256)7, (uchar256)6,
                         v_u8_sel_eq_u8_b(e, (uchar256)6, (uchar256)3,
                         v_u8_sel_eq_u8_b(e, (uchar256)5, (uchar256)2,
                         v_u8_sel_eq_u8_b(e, (uchar256)4, (uchar256)1, (uchar256)0))));
    t = v_u8_sel_less_u8_b(e, (uchar256)8, sub, t);
    return v_u8_or_b(t, (uchar256)0x80);
}

// lane e = lane & 31 of a group -> the base of code e & 15 (both halves of the group). Bit 5 of the result is f.
static inline uchar256 build_basef2(uchar256 e)
{
    const uchar256 m = v_u8_and_b(e, (uchar256)7);
    uchar256 b = (uchar256)0;
    b = v_u8_sel_eq_u8_b(m, (uchar256)1, (uchar256)0x80, b);
    b = v_u8_sel_eq_u8_b(m, (uchar256)2, (uchar256)0x81, b);
    b = v_u8_sel_eq_u8_b(m, (uchar256)3, (uchar256)0xA1, b);
    b = v_u8_sel_eq_u8_b(m, (uchar256)4, (uchar256)0x82, b);
    b = v_u8_sel_eq_u8_b(m, (uchar256)5, (uchar256)0xA2, b);
    b = v_u8_sel_eq_u8_b(m, (uchar256)6, (uchar256)0x83, b);
    b = v_u8_sel_eq_u8_b(m, (uchar256)7, (uchar256)0xA3, b);
    return b;
}

#define BCAST_DG0(x) v_u8_mov_dual_group_all_b((x), 0xFFFFFFFF, 0, 0, 0, 0, MkWrA(0b11, 0b11, 0b11, 0b11), (x))
