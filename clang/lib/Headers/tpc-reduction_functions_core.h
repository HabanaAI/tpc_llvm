/*****************************************************************************
* Copyright (C) 2020 HabanaLabs, Ltd.
* All Rights Reserved.
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Proprietary and confidential.
*
* Authors:
* Keren Luzon <kluzon@habana.ai>
******************************************************************************
*/
#if __TPC_DROP_VERSION >= VERSION2DEC(35, 0, 0) && defined(INCLUDE_TPC_REDUCTION_CORE_H)

////////////////////////////////////////////////////////////////////////////////////////////////////
/// SHUFFLE MAP LOOKUP CONFIG
////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(__goya__)
    #define FUNC_ID_REDUCTION_32    42
    #define FUNC_ID_REDUCTION_16    29
    #define FUNC_ID_REDUCTION_8     30
#elif (defined(__gaudi__) || defined(__gaudib__))
#define FUNC_ID_REDUCTION_32 282
#define FUNC_ID_REDUCTION_16 140
#define FUNC_ID_REDUCTION_8 141
#else /* goya2/gaudi2/doron1 */
    #define FUNC_ID_REDUCTION_32    287
    #define FUNC_ID_REDUCTION_16    140
    #define FUNC_ID_REDUCTION_8     141
#endif

#if defined( __goya__)
    #define v_f32_lookup_1c   v_f32_lookup_c0
    #define v_f32_lookup_2c   v_f32_lookup_c1c2
#endif

#define shuffle_map_32bit_lookup                                                    \
    float64  c0  = 0;                                                               \
    float128 c12 = {0};                                                             \
    c0  = v_f32_lookup_1c(read_lane_id_4b_b(), FUNC_ID_REDUCTION_32, 0, c0, 1, 0);         \
    c12 = v_f32_lookup_2c(read_lane_id_4b_b(), FUNC_ID_REDUCTION_32, 0, c12, 1, 0);        \
    const uchar256 lut1 = (uchar256)c0;                                             \
    const uchar256 lut2 = (uchar256)c12.v1;                                         \
    const uchar256 lut3 = (uchar256)c12.v2;

#define shuffle_map_16bit_lookup                                                    \
    float128 c01 = {0};                                                             \
    float128 c23 = {0};                                                             \
    c01 = v_f32_lookup_2c(read_lane_id_4b_b(), FUNC_ID_REDUCTION_16, 0, c01, 1, 0);        \
    c23 = v_f32_lookup_2c(read_lane_id_4b_b() + 64, FUNC_ID_REDUCTION_16, 0, c23, 1, 0);   \
    const uchar256 lut1 = (uchar256)c01.v1;                                         \
    const uchar256 lut2 = (uchar256)c01.v2;                                         \
    const uchar256 lut3 = (uchar256)c23.v1;                                         \
    const uchar256 lut4 = (uchar256)c23.v2;

#define shuffle_map_8bit_lookup                                                     \
    float64  c0  = 0;                                                               \
    float128 c12 = {0};                                                             \
    float128 c34 = {0};                                                             \
    c0  = v_f32_lookup_1c(read_lane_id_4b_b(), FUNC_ID_REDUCTION_8, 0, c0, 1, 0);          \
    c12 = v_f32_lookup_2c(read_lane_id_4b_b(), FUNC_ID_REDUCTION_8, 0, c12, 1, 0);         \
    c34 = v_f32_lookup_2c(read_lane_id_4b_b() + 64, FUNC_ID_REDUCTION_8, 0, c34, 1, 0);    \
    const uchar256 lut1 = (uchar256)c0;                                             \
    const uchar256 lut2 = (uchar256)c12.v1;                                         \
    const uchar256 lut3 = (uchar256)c12.v2;                                         \
    const uchar256 lut4 = (uchar256)c34.v1;                                         \
    const uchar256 lut5 = (uchar256)c34.v2;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// REDUCTION SWITCHES CONFIG
////////////////////////////////////////////////////////////////////////////////////////////////////

// Switches for mov_dual_group_all
#define SW_MDG_ALL (SW_WR_LOWER_GROUP0 | SW_WR_UPPER_GROUP0 | \
                    SW_WR_LOWER_GROUP1 | SW_WR_UPPER_GROUP1 | \
                    SW_WR_LOWER_GROUP2 | SW_WR_UPPER_GROUP2 | \
                    SW_WR_LOWER_GROUP3 | SW_WR_UPPER_GROUP3)

// Switches for mov_dual_group
#define SW_MDG (SW_WR_LOWER_GROUP | SW_WR_UPPER_GROUP)

// Switches for mov_group
#define SW_MG (SW_GROUP0_EN | SW_GROUP1_EN | \
                SW_DUAL_GROUP0_EN | SW_DUAL_GROUP1_EN | \
                SW_DUAL_GROUP2_EN | SW_DUAL_GROUP3_EN)

////////////////////////////////////////////////////////////////////////////////////////////////////
/// REDUCTION OP AND DATA-TYPE CONFIG
////////////////////////////////////////////////////////////////////////////////////////////////////

#define REDUCE_F32                  0 // f32
#define REDUCE_I32                  1 // i32
#define REDUCE_U32                  2 // u32
#define REDUCE_BF16                 3 // bf16
#define REDUCE_F16                  4 // f16
#define REDUCE_I16                  5 // i16
#define REDUCE_U16                  6 // u16
#define REDUCE_I8                   7 // i8
#define REDUCE_U8                   8 // u8

#if REDUCE_DT == REDUCE_F32 || REDUCE_DT == REDUCE_I32 || REDUCE_DT == REDUCE_U32
    #define INIT_INDEX              read_lane_id_4b_b()
#elif REDUCE_DT == REDUCE_BF16 || REDUCE_DT == REDUCE_F16 || REDUCE_DT == REDUCE_I16 || REDUCE_DT == REDUCE_U16
    #define INIT_INDEX              read_lane_id_2b_b()
#elif REDUCE_DT == REDUCE_I8 || REDUCE_DT == REDUCE_U8
    #define INIT_INDEX              read_lane_id_1b_b()
#endif

#if REDUCE_DT == REDUCE_F32
    #define T                       f32
    #define I                       u32
    #define B                       32bit
    #define VEC_TYPE                float64
    #define PAIR_VEC_TYPE           uint64_float64_pair_t
#elif REDUCE_DT == REDUCE_I32
    #define T                       i32
    #define I                       u32
    #define B                       32bit
    #define VEC_TYPE                int64
    #define PAIR_VEC_TYPE           uint64_int64_pair_t
#elif REDUCE_DT == REDUCE_U32
    #define T                       u32
    #define I                       u32
    #define B                       32bit
    #define VEC_TYPE                uint64
    #define PAIR_VEC_TYPE           uint64_uint64_pair_t
#elif REDUCE_DT == REDUCE_BF16
    #define T                       bf16
    #define I                       u16
    #define B                       16bit
    #define VEC_TYPE                bfloat128
    #define PAIR_VEC_TYPE           ushort128_bfloat128_pair_t
#elif REDUCE_DT == REDUCE_F16
    #define T                       f16
    #define I                       u16
    #define B                       16bit
    #define VEC_TYPE                half128
    #define PAIR_VEC_TYPE           ushort128_half128_pair_t
#elif REDUCE_DT == REDUCE_I16
    #define T                       i16
    #define I                       u16
    #define B                       16bit
    #define VEC_TYPE                short128
    #define PAIR_VEC_TYPE           ushort128_short128_pair_t
#elif REDUCE_DT == REDUCE_U16
    #define T                       u16
    #define I                       u16
    #define B                       16bit
    #define VEC_TYPE                ushort128
    #define PAIR_VEC_TYPE           ushort128_ushort128_pair_t
#elif REDUCE_DT == REDUCE_I8
    #define T                       i8
    #define I                       u8
    #define B                       8bit
    #define VEC_TYPE                char256
    #define PAIR_VEC_TYPE           uchar256_char256_pair_t
#elif REDUCE_DT == REDUCE_U8
    #define T                       u8
    #define I                       u8
    #define B                       8bit
    #define VEC_TYPE                uchar256
    #define PAIR_VEC_TYPE           uchar256_uchar256_pair_t
#endif

#define REDUCE_ADD                  0 // add
#define REDUCE_MUL                  1 // mul
#define REDUCE_MIN                  2 // min
#define REDUCE_MAX                  3 // max
#define REDUCE_ARGMIN               4 // argmin
#define REDUCE_ARGMAX               5 // argmax

#if REDUCE_OP == REDUCE_ADD
    #define OP                      _add
    #define NAME                    _add
#elif REDUCE_OP == REDUCE_MUL
    #define OP                      _mul
    #define NAME                    _mul
#elif REDUCE_OP == REDUCE_MIN
    #define OP                      _min
    #define NAME                    _min
#elif REDUCE_OP == REDUCE_MAX
    #define OP                      _max
    #define NAME                    _max
#elif REDUCE_OP == REDUCE_ARGMIN
    #define OP                      _sel2_less_
    #define NAME                    _argmin
#elif REDUCE_OP == REDUCE_ARGMAX
    #define OP                      _sel2_grt_
    #define NAME                    _argmax
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// REDUCTION CORE LOGIC
////////////////////////////////////////////////////////////////////////////////////////////////////

#define TOKENPASTE3(a, b, c)        a ## b ## c
#define TOKENPASTE4(a, b, c, d)     a ## b ## c ## d
#define TOKENPASTE5(a, b, c, d, e)  a ## b ## c ## d ## e

#define V_REDUCE_CORE(OP, T)        TOKENPASTE4(v_, T, _reduce, OP)
#define V_MOV_DUAL_GROUP(T)         TOKENPASTE3(v_, T, _mov_dual_group_b)
#define V_MOV_DUAL_GROUP_ALL(T)     TOKENPASTE3(v_, T, _mov_dual_group_all_b)
#define V_MOV_GROUP(T)              TOKENPASTE3(v_, T, _mov_group_b)
#define V_SHUFFLE(T)                TOKENPASTE3(v_, T, _shuffle_b)
#define SHUFFLE_MAP_LOOKUP(B)       TOKENPASTE3(shuffle_map_, B, _lookup)

#if REDUCE_OP == REDUCE_ADD || REDUCE_OP == REDUCE_MUL || REDUCE_OP == REDUCE_MIN || REDUCE_OP == REDUCE_MAX
#define V_OP(OP, T)                 TOKENPASTE4(v_, T, OP, _b)

VEC_TYPE V_REDUCE_CORE(NAME, T)(VEC_TYPE x)
{
    VEC_TYPE t = 0;
    SHUFFLE_MAP_LOOKUP(B)

    // Switch dual groups dg0 <-> dg1, dg2 <-> dg3
    #if defined(__goya__)
    t = V_MOV_DUAL_GROUP(T)(x, 0xFFFFFFFF, 1, 0, SW_MDG, t, 1, 0);
    t = V_MOV_DUAL_GROUP(T)(x, 0xFFFFFFFF, 3, 2, SW_MDG, t, 1, 0);
    #else
    t = V_MOV_DUAL_GROUP_ALL(T)(x, 0xFFFFFFFF, 1, 0, 3, 2, SW_MDG_ALL, t, 1, 0);
    #endif

    // (dg0 + dg1), (dg2 + dg3)
    x = V_OP(OP, T)(x, t, 0, x, 1, 0);

    // Switch dual groups dg0 <-> dg2, dg1 <-> dg3
    #if defined(__goya__)
    t = V_MOV_DUAL_GROUP(T)(x, 0xFFFFFFFF, 2, 0, SW_MDG, t, 1, 0);
    #else
    t = V_MOV_DUAL_GROUP_ALL(T)(x, 0xFFFFFFFF, 2, 3, 0, 1, SW_MDG_ALL, t, 1, 0);
    #endif

    // (dg0 + dg1 + dg2 + dg3)
    x = V_OP(OP, T)(x, t, 0, x, 1, 0);

    // Switch groups g0 <-> g1
    t = V_MOV_GROUP(T)(x, 0xFFFFFFFF, SW_MG, t, 1, 0);

    // (dg0 + dg1 + dg2 + dg3) + (g0 + g1)
    x = V_OP(OP, T)(x, t, 0, x, 1, 0);

    // Shuffle elements (every 2 elements)
    t = V_SHUFFLE(T)(x, lut1, 0, t, 1, 0);

    // (dg0 + dg1 + dg2 + dg3) + (g0 + g1) + (0 + 1)
    x = V_OP(OP, T)(x, t, 0, x, 1, 0);

    // Shuffle elements (every 4 elements)
    t = V_SHUFFLE(T)(x, lut2, 0, t, 1, 0);

    // (dg0 + dg1 + dg2 + dg3) + (g0 + g1) + (0 + ... + 3)
    x = V_OP(OP, T)(x, t, 0, x, 1, 0);

    // Shuffle elements (every 8 elements)
    t = V_SHUFFLE(T)(x, lut3, 0, t, 1, 0);

    // (dg0 + dg1 + dg2 + dg3) + (g0 + g1) + (0 + ... + 7)
    x = V_OP(OP, T)(x, t, 0, x, 1, 0);

    #if REDUCE_DT == REDUCE_BF16 || REDUCE_DT == REDUCE_F16 || REDUCE_DT == REDUCE_I16 || \
        REDUCE_DT == REDUCE_U16 || REDUCE_DT == REDUCE_I8 || REDUCE_DT == REDUCE_U8
    // Shuffle elements (every 16 elements)
    t = V_SHUFFLE(T)(x, lut4, 0, t, 1, 0);

    // (dg0 + dg1 + dg2 + dg3) + (g0 + g1) + (0 + ... + 15)
    x = V_OP(OP, T)(x, t, 0, x, 1, 0);
    #endif

    #if REDUCE_DT == REDUCE_I8 || REDUCE_DT == REDUCE_U8
    // Shuffle elements (every 32 elements)
    t = V_SHUFFLE(T)(x, lut5, 0, t, 1, 0);

    // (dg0 + dg1 + dg2 + dg3) + (g0 + g1) + (0 + ... + 31)
    x = V_OP(OP, T)(x, t, 0, x, 1, 0);
    #endif

    // Broadcast dual groups
    #if defined(__goya__)
    x = V_MOV_DUAL_GROUP(T)(x, 0xFFFFFFFF, 0, 1, SW_MDG, x, 1, 0);
    x = V_MOV_DUAL_GROUP(T)(x, 0xFFFFFFFF, 0, 2, SW_MDG, x, 1, 0);
    x = V_MOV_DUAL_GROUP(T)(x, 0xFFFFFFFF, 0, 3, SW_MDG, x, 1, 0);
    #endif

    return x;
}
#endif

#if REDUCE_OP == REDUCE_ARGMIN || REDUCE_OP == REDUCE_ARGMAX
#define V_SEL_OP(OP, T, I)          TOKENPASTE5(v_, I, OP, T, _b)

PAIR_VEC_TYPE V_REDUCE_CORE(NAME, T)(VEC_TYPE x)
{
    PAIR_VEC_TYPE y = {0};
    PAIR_VEC_TYPE t = {0};
    y.v1 = INIT_INDEX;
    y.v2 = x;
    SHUFFLE_MAP_LOOKUP(B)

    // Shuffle elements (every 2 elements)
    t.v1 = V_SHUFFLE(I)(y.v1, lut1, 0, t.v1, 1, 0);
    t.v2 = V_SHUFFLE(T)(y.v2, lut1, 0, t.v2, 1, 0);

    // (0 + 1)
    y = V_SEL_OP(OP, T, I)(t.v2, y.v2, t.v1, y.v1, 0, y, 1, 0);

    // Shuffle elements (every 4 elements)
    t.v1 = V_SHUFFLE(I)(y.v1, lut2, 0, t.v1, 1, 0);
    t.v2 = V_SHUFFLE(T)(y.v2, lut2, 0, t.v2, 1, 0);

    // (0 + ... + 3)
    y = V_SEL_OP(OP, T, I)(t.v2, y.v2, t.v1, y.v1, 0, y, 1, 0);

    // Shuffle elements (every 8 elements)
    t.v1 = V_SHUFFLE(I)(y.v1, lut3, 0, t.v1, 1, 0);
    t.v2 = V_SHUFFLE(T)(y.v2, lut3, 0, t.v2, 1, 0);

    // (0 + ... + 7)
    y = V_SEL_OP(OP, T, I)(t.v2, y.v2, t.v1, y.v1, 0, y, 1, 0);

    #if REDUCE_DT == REDUCE_BF16 || REDUCE_DT == REDUCE_F16 || REDUCE_DT == REDUCE_I16 || \
        REDUCE_DT == REDUCE_U16 || REDUCE_DT == REDUCE_I8 || REDUCE_DT == REDUCE_U8
    // Shuffle elements (every 16 elements)
    t.v1 = V_SHUFFLE(I)(y.v1, lut4, 0, t.v1, 1, 0);
    t.v2 = V_SHUFFLE(T)(y.v2, lut4, 0, t.v2, 1, 0);

    // (0 + ... + 15)
    y = V_SEL_OP(OP, T, I)(t.v2, y.v2, t.v1, y.v1, 0, y, 1, 0);
    #endif

    #if REDUCE_DT == REDUCE_I8 || REDUCE_DT == REDUCE_U8
    // Shuffle elements (every 32 elements)
    t.v1 = V_SHUFFLE(I)(y.v1, lut5, 0, t.v1, 1, 0);
    t.v2 = V_SHUFFLE(T)(y.v2, lut5, 0, t.v2, 1, 0);

    // (0 + ... + 31)
    y = V_SEL_OP(OP, T, I)(t.v2, y.v2, t.v1, y.v1, 0, y, 1, 0);
    #endif

    // Switch groups g0 <-> g1
    t.v1 = V_MOV_GROUP(I)(y.v1, 0xFFFFFFFF, SW_MG, t.v1, 1, 0);
    t.v2 = V_MOV_GROUP(T)(y.v2, 0xFFFFFFFF, SW_MG, t.v2, 1, 0);

    // (g0 + g1)
    y = V_SEL_OP(OP, T, I)(t.v2, y.v2, t.v1, y.v1, 0, y, 1, 0);

    // Switch dual groups dg0 <-> dg1, dg2 <-> dg3
    #if defined(__goya__)
    t.v1 = V_MOV_DUAL_GROUP(I)(y.v1, 0xFFFFFFFF, 1, 0, SW_MDG, t.v1, 1, 0);
    t.v1 = V_MOV_DUAL_GROUP(I)(y.v1, 0xFFFFFFFF, 3, 2, SW_MDG, t.v1, 1, 0);
    t.v2 = V_MOV_DUAL_GROUP(T)(y.v2, 0xFFFFFFFF, 1, 0, SW_MDG, t.v2, 1, 0);
    t.v2 = V_MOV_DUAL_GROUP(T)(y.v2, 0xFFFFFFFF, 3, 2, SW_MDG, t.v2, 1, 0);
    #else
    t.v1 = V_MOV_DUAL_GROUP_ALL(I)(y.v1, 0xFFFFFFFF, 1, 0, 3, 2, SW_MDG_ALL, t.v1, 1, 0);
    t.v2 = V_MOV_DUAL_GROUP_ALL(T)(y.v2, 0xFFFFFFFF, 1, 0, 3, 2, SW_MDG_ALL, t.v2, 1, 0);
    #endif

    // (dg0 + dg1), (dg2 + dg3)
    y = V_SEL_OP(OP, T, I)(t.v2, y.v2, t.v1, y.v1, 0, y, 1, 0);

    // Switch dual groups dg0 <-> dg2, dg1 <-> dg3
    #if defined(__goya__)
    t.v1 = V_MOV_DUAL_GROUP(I)(y.v1, 0xFFFFFFFF, 2, 0, SW_MDG, t.v1, 1, 0);
    t.v2 = V_MOV_DUAL_GROUP(T)(y.v2, 0xFFFFFFFF, 2, 0, SW_MDG, t.v2, 1, 0);
    #else
    t.v1 = V_MOV_DUAL_GROUP_ALL(I)(y.v1, 0xFFFFFFFF, 2, 3, 0, 1, SW_MDG_ALL, t.v1, 1, 0);
    t.v2 = V_MOV_DUAL_GROUP_ALL(T)(y.v2, 0xFFFFFFFF, 2, 3, 0, 1, SW_MDG_ALL, t.v2, 1, 0);
    #endif

    // (dg0 + dg1 + dg2 + dg3)
    y = V_SEL_OP(OP, T, I)(t.v2, y.v2, t.v1, y.v1, 0, y, 1, 0);

    // Broadcast group elements
    const uchar256 lutb = 0x80;
    y.v1 = V_SHUFFLE(I)(y.v1, lutb, 0, y.v1, 1, 0);
    y.v2 = V_SHUFFLE(T)(y.v2, lutb, 0, y.v2, 1, 0);

    // Broadcast dual groups
    #if defined(__goya__)
    y.v1 = V_MOV_DUAL_GROUP(I)(y.v1, 0xFFFFFFFF, 0, 1, SW_MDG, y.v1, 1, 0);
    y.v2 = V_MOV_DUAL_GROUP(T)(y.v2, 0xFFFFFFFF, 0, 1, SW_MDG, y.v2, 1, 0);
    y.v1 = V_MOV_DUAL_GROUP(I)(y.v1, 0xFFFFFFFF, 0, 2, SW_MDG, y.v1, 1, 0);
    y.v2 = V_MOV_DUAL_GROUP(T)(y.v2, 0xFFFFFFFF, 0, 2, SW_MDG, y.v2, 1, 0);
    y.v1 = V_MOV_DUAL_GROUP(I)(y.v1, 0xFFFFFFFF, 0, 3, SW_MDG, y.v1, 1, 0);
    y.v2 = V_MOV_DUAL_GROUP(T)(y.v2, 0xFFFFFFFF, 0, 3, SW_MDG, y.v2, 1, 0);
    #else
    y.v1 = V_MOV_DUAL_GROUP_ALL(I)(y.v1, 0xFFFFFFFF, 0, 0, 0, 0, SW_MDG_ALL, y.v1, 1, 0);
    y.v2 = V_MOV_DUAL_GROUP_ALL(T)(y.v2, 0xFFFFFFFF, 0, 0, 0, 0, SW_MDG_ALL, y.v2, 1, 0);
    #endif

    return y;
}
#endif

#undef T
#undef I
#undef B
#undef OP
#undef NAME
#undef INIT_INDEX
#undef VEC_TYPE
#undef PAIR_VEC_TYPE

#endif //__TPC_DROP_VERSION
