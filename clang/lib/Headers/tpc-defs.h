//===--- tpc-defs.h --------------------------------------------*- 	C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// tpc-defs.h
//
// This file contains definitions vital for TPC C/C++ compiler (the compiler
// crashes without them). Any code definitions that must be available without
// inclusion of any header file, but is not necessary for compiler functioning,
// should be placed into 'tpc-special.h'.
//------------------------------------------------------------------------------

#ifndef TPC_DEFS_H_INCLUDED
#define TPC_DEFS_H_INCLUDED

// Standard vector types
typedef unsigned char __attribute__((ext_vector_type(32)))
                      __attribute__((aligned(256)))           bool256;
typedef unsigned char __attribute__((ext_vector_type(16)))
                      __attribute__((aligned(256)))           bool128;
typedef unsigned char __attribute__((ext_vector_type(8)))
                      __attribute__((aligned(256)))           bool64;

typedef float         __attribute__((ext_vector_type(64)))    float64;
typedef int           __attribute__((ext_vector_type(64)))    int64;
typedef unsigned int  __attribute__((ext_vector_type(64)))    uint64;
typedef short         __attribute__((ext_vector_type(128)))   short128;
typedef unsigned short __attribute__((ext_vector_type(128)))  ushort128;
typedef char          __attribute__((ext_vector_type(256)))   char256;
typedef unsigned char __attribute__((ext_vector_type(256)))   uchar256;
typedef int           __attribute__((ext_vector_type(5)))     int5;
#if defined(__gaudi__)
typedef _BFloat16 bf16;
typedef _BFloat16 bfloat;
typedef _BFloat16     __attribute__((ext_vector_type(128)))   bfloat128;
#endif

// OpenCL v1.1/1.2/2.0 s6.2.4.2 - as_type operators
// Reinterprets a data type as another data type of the same size

#define as_char(x)      __builtin_astype((x), char)
#define as_char256(x)   __builtin_astype((x), char256)

#define as_uchar(x)     __builtin_astype((x), unsigned char)
#define as_uchar256(x)  __builtin_astype((x), uchar256)

#define as_short(x)     __builtin_astype((x), short)
#define as_short128(x)  __builtin_astype((x), short128)

#define as_ushort(x)    __builtin_astype((x), unsigned short)
#define as_ushort128(x) __builtin_astype((x), ushort128)

#define as_int(x)       __builtin_astype((x), int)
#define as_int64(x)     __builtin_astype((x), int64)

#define as_uint(x)      __builtin_astype((x), unsigned int)
#define as_uint64(x)    __builtin_astype((x), uint64)

#define as_float(x)     __builtin_astype((x), float)
#define as_float64(x)   __builtin_astype((x), float64)

#if defined(__gaudi__)
#define as_bf16(x)      __builtin_astype((x), bf16)
#define as_bfloat(x)    __builtin_astype((x), bfloat)
#define as_bfloat128(x) __builtin_astype((x), bfloat128)
#endif

// Structures mapped to registers

// float
typedef struct _float64_pair_t {
  float64 v1;
  float64 v2;
} float64_pair_t;
typedef struct _float64_pair_t float64_float64_pair_t;
typedef struct _float64_pair_t float128;

typedef struct _float256 {
  float64 v1;
  float64 v2;
  float64 v3;
  float64 v4;
} float256;

typedef struct _float64_int64_pair_t {
  float64 v1;
  int64 v2;
} float64_int64_pair_t;

typedef struct _float64_uint64_pair_t {
  float64 v1;
  uint64 v2;
} float64_uint64_pair_t;


// int
typedef struct _int64_float64_pair_t {
  int64 v1;
  float64 v2;
} int64_float64_pair_t;

typedef struct _int64_pair_t {
  int64 v1;
  int64 v2;
} int64_pair_t;
typedef struct _int64_pair_t int64_int64_pair_t;
typedef struct _int64_pair_t int128;

typedef struct _int64_uint64_pair_t {
  int64 v1;
  uint64 v2;
} int64_uint64_pair_t;


// uint
typedef struct _uint64_float64_pair_t {
  uint64 v1;
  float64 v2;
} uint64_float64_pair_t;

typedef struct _uint64_int64_pair_t {
  uint64 v1;
  int64 v2;
} uint64_int64_pair_t;

typedef struct _uint64_pair_t {
  uint64 v1;
  uint64 v2;
} uint64_pair_t;
typedef struct _uint64_pair_t uint64_uint64_pair_t;
typedef struct _uint64_pair_t uint128;

#if defined(__gaudi__)
// _BFloat16
typedef struct _bfloat128_pair_t {
  bfloat128 v1;
  bfloat128 v2;
} bfloat128_pair_t;
typedef struct _bfloat128_pair_t bfloat128_bfloat128_pair_t;
typedef struct _bfloat128_pair_t bfloat256;

typedef struct _bfloat128_short128_pair_t {
  bfloat128 v1;
  short128 v2;
} bfloat128_short128_pair_t;

typedef struct _bfloat128_ushort128_pair_t {
  bfloat128 v1;
  ushort128 v2;
} bfloat128_ushort128_pair_t;
#endif

typedef struct _short128_pair_t {
  short128 v1;
  short128 v2;
} short128_pair_t;
typedef struct _short128_pair_t short128_short128_pair_t;
typedef struct _short128_pair_t short256;

typedef struct _short128_ushort128_pair_t {
  short128 v1;
  ushort128 v2;
} short128_ushort128_pair_t;


// short/ushort
#if defined(__gaudi__)
typedef struct _short128_bfloat128_pair_t {
  short128 v1;
  bfloat128 v2;
} short128_bfloat128_pair_t;

typedef struct _ushort128_bfloat128_pair_t {
  ushort128 v1;
  bfloat128 v2;
} ushort128_bfloat128_pair_t;
#endif

typedef struct _ushort128_short128_pair_t {
  ushort128 v1;
  short128 v2;
} ushort128_short128_pair_t;

typedef struct _ushort128_pair_t {
  ushort128 v1;
  ushort128 v2;
} ushort128_pair_t;
typedef struct _ushort128_pair_t ushort128_ushort128_pair_t;
typedef struct _ushort128_pair_t ushort256;


// char
typedef struct _char256_pair_t {
  char256 v1;
  char256 v2;
} char256_pair_t;
typedef struct _char256_pair_t char256_char256_pair_t;
typedef struct _char256_pair_t char512;

typedef struct _char256_uchar256_pair_t {
  char256 v1;
  uchar256 v2;
} char256_uchar256_pair_t;

// uchar
typedef struct _uchar256_char256_pair_t {
  uchar256 v1;
  char256 v2;
} uchar256_char256_pair_t;

typedef struct _uchar256_pair_t {
  uchar256 v1;
  uchar256 v2;
} uchar256_pair_t;
typedef struct _uchar256_pair_t uchar256_uchar256_pair_t;
typedef struct _uchar256_pair_t uchar512;

typedef struct _uint32_t_pair_t {
  unsigned int v1;
  unsigned int v2;
} uint32_t_pair_t;

typedef struct _uint16_t_pair_t {
  unsigned int v1;
  unsigned int v2;
} uint16_t_pair_t;

typedef struct _uint8_t_pair_t {
  unsigned int v1;
  unsigned int v2;
} uint8_t_pair_t;


typedef struct _int32_t_pair_t {
  int v1;
  int v2;
} int32_t_pair_t;

typedef struct _int256 {
  int64 v1;
  int64 v2;
  int64 v3;
  int64 v4;
} int256;

typedef struct _uint256 {
  uint64 v1;
  uint64 v2;
  uint64 v3;
  uint64 v4;
} uint256;

// Address space specifiers
#define __local__  __local
#define __global__ __global


// Internal functions.
void static inline __attribute__((always_inline)) __initialize();

// It would be nice to remove this definition some day.
typedef int tensor;


// Preloaded registers
#define LFSR            read_lfsr()
#define LFSR_NO_CHANGE  read_lfsrnc()
#define S_LFSR            s_read_lfsr()
#define S_LFSR_NO_CHANGE  s_read_lfsrnc()
#ifdef __cplusplus
register          uint64    V_LANE_ID_32    __asm__("v_lane_id_32");
register          ushort128 V_LANE_ID_16    __asm__("v_lane_id_16");
register          uchar256  V_LANE_ID_8     __asm__("v_lane_id_8");
#else
register const    uint64    V_LANE_ID_32    __asm__("v_lane_id_32");
register const    ushort128 V_LANE_ID_16    __asm__("v_lane_id_16");
register const    uchar256  V_LANE_ID_8     __asm__("v_lane_id_8");
#endif


#ifndef NO_TPC_PRINTF
// Index needed for printf intrinsic.
register int5 __PrintfTensorIndex __asm__("i2");
#endif

// Instruction switches
enum {
  // ABS
  SW_NO_SAT             = 0x1 << 0,
  // ADD
  SW_SAT                = 0x1 << 0,
  SW_CARRY              = 0x1 << 1,
  SW_NO_CARRY_GEN       = 0x1 << 2,
  // ASH
  /* SW_SAT - see previous definition */
  SW_RHU                = 0x1 << 1,
  // ASO / EVENT
  SW_INC                = 0x0 << 0,
  SW_DEC                = 0x1 << 0,
  SW_SPU                = 0x0 << 1,
  SW_VPU                = 0x1 << 1,
  // CACHE_INVALIDATE
  SW_SB                 = 0x1 << 0,
  SW_D                  = 0x1 << 1,
  SW_LU                 = 0x1 << 2,
  SW_RST_LU             = 0x1 << 3,
  SW_RST_D_PREF         = 0x1 << 4,
  // CALC_FP_SPECIAL
  SW_RECIP              = 0x0 << 0,
  SW_RSQRT              = 0x1 << 0,
  SW_SQRT               = 0x2 << 0,
  SW_LOG                = 0x3 << 0,
  SW_EXP                = 0x4 << 0,
  SW_TANH               = 0x5 << 0,
  SW_DIV                = 0x6 << 0,
  SW_POW                = 0x7 << 0,
  // CMP_EQ / SEL_EQ
  SW_MASK_EQ_ZERO       = 0x1 << 0,
  // CONVERT
  // CONVERT_INT32 / CONVERT_UINT32
  // CONVERT_INT16 / CONVERT_UINT16
  // CONVERT_INT8  / CONVERT_UINT8
  // NEARBYINT
  SW_RHNE               = 0x0 << (0 + 16),
  SW_RZ                 = 0x1 << (0 + 16),
  SW_RU                 = 0x2 << (0 + 16),
  SW_RD                 = 0x3 << (0 + 16),
  SW_SR                 = 0x4 << (0 + 16),
  SW_CSR                = 0x5 << (0 + 16),
  SW_RHAZ               = 0x6 << (0 + 16),
  SW_SR_RNE             = 0x7 << (0 + 16),
  SW_CLIP_FP            = 0x1 << (4 + 16),
  // GEN_ADDR
  SW_DT_INT8            = 0x0 << 0,
  SW_DT_INT16           = 0x1 << 0,
  SW_DT_INT32           = 0x2 << 0,
  SW_DT_UINT8           = 0x3 << 0,
  SW_DT_UINT16          = 0x4 << 0,
  SW_DT_UINT32          = 0x5 << 0,
  SW_DT_BF16            = 0x6 << 0,
  SW_DT_FP32            = 0x7 << 0,
  SW_DT_FP16            = 0x8 << 0,
  SW_DT_OVERRIDE        = 0x1 << 4,
  // EXTRACT_EXP
  SW_BIASED             = 0x1 << 0,
  // FIND_FIRST
  SW_FIND_ZERO          = 0x0 << 0,
  SW_FIND_ONE           = 0x1 << 0,
  SW_LSB                = 0x0 << 1,
  SW_MSB                = 0x1 << 1,
  // FORM_FP_NUMBER
  SW_ADD_BIAS           = 0x1 << (0 + 8),
  SW_FORCE_SIGN0        = 0x1 << (1 + 8),
  SW_FORCE_SIGN1        = 0x1 << (2 + 8),
  SW_EXP_IS_NUM         = 0x1 << (3 + 8),
  SW_SIGN_LSB           = 0x1 << (4 + 8),
  SW_PRE_SQRT_RSQRT     = (0x1 << (5 + 8)) | SW_FORCE_SIGN0,
  SW_POST_SQRT          = (0x2 << (5 + 8)) | SW_FORCE_SIGN0,
  SW_POST_RSQRT         = (0x3 << (5 + 8)) | SW_FORCE_SIGN0,
  SW_POST_RECIP         = (0x4 << (5 + 8)) | SW_ADD_BIAS,
  // GET_LUT_ENTRY_AND_INTERVAL_START
  SW_LUT_TANH           = 0x1 << (5 + 8),
  SW_LUT_SQRT_RSQRT     = 0x2 << (5 + 8),
  SW_LUT_SIN_COS        = 0x3 << (5 + 8),
  SW_LUT_LOG            = 0x4 << (5 + 8),
  // LD_L / ST_L
  SW_MMIO               = 0x1 << 0,
  SW_LOCK               = 0x1 << 1,
  SW_UNLOCK             = 0x1 << 1,
  // LD_L_V / ST_L_V
  SW_AUTO_INC_1_V       = 0x1 << 0,
  SW_AUTO_INC_2_V       = 0x2 << 0,
  SW_AUTO_INC_4_V       = 0x3 << 0,
  // LD_G / ST_G
  SW_INC_1              = 0x1,
  SW_INC_2              = 0x2,
  SW_INC_4              = 0x3,
  SW_INC_8              = 0x4,

  SW_BV64               = 0x1 << 4,
  SW_L0CD               = 0x1 << 5,
  SW_EV_HINT            = 0x1 << 6,
  SW_PD                 = 0x1 << (7 + 8),
  // LD_TNSR
  SW_UNPCK_16_TO_32     = 0x0 << 1,
  SW_UNPCK_8_TO_16      = 0x1 << 1,
  SW_UNPCK_8_TO_32      = 0x2 << 1,
  SW_UNPCK_4_TO_8       = 0x3 << 1,
  SW_UNPACK             = 0x1 << 4,
  /* SW_L0CD - see previous definition */
  // LOOKUP
  SW_BV32               = 0x0 << 0,
  SW_BV16               = 0x1 << 0,
  #if defined(__goya__)
  SW_BV16_LOW           = 0x2 << 0,
  SW_BV16_HIGH          = 0x3 << 0,
  SW_BV8_0              = 0x4 << 0,
  SW_BV8_1              = 0x5 << 0,
  SW_BV8_2              = 0x6 << 0,
  SW_BV8_3              = 0x7 << 0,
  #else
  SW_BV8_0              = 0x2 << 0,
  SW_BV8_1              = 0x3 << 0,
  #endif
  SW_UPPER_HALF         = 0x1 << 2,
  SW_LUT_PTR            = 0x1 << 3,
  SW_X2                 = 0x1 << 4,
  SW_SBCD               = 0x1 << 5,
  // MAC / MADD
  /* SW_SAT - see previous definition */
  SW_NEG                = 0x1 << 1,
  SW_NEG_ZP             = 0x1 << 6,
  // MOV_DUAL_GROUP
  SW_WR_LOWER_GROUP     = 0x1 << (4 + 8),
  SW_WR_UPPER_GROUP     = 0x1 << (5 + 8),
  SW_WR_LOWER_GROUP0    = 0x1 << (0 + 16),
  SW_WR_UPPER_GROUP0    = 0x1 << (1 + 16),
  SW_WR_LOWER_GROUP1    = 0x1 << (2 + 16),
  SW_WR_UPPER_GROUP1    = 0x1 << (3 + 16),
  SW_WR_LOWER_GROUP2    = 0x1 << (4 + 16),
  SW_WR_UPPER_GROUP2    = 0x1 << (5 + 16),
  SW_WR_LOWER_GROUP3    = 0x1 << (6 + 16),
  SW_WR_UPPER_GROUP3    = 0x1 << (7 + 16),
  SW_PACK21             = 0x0 << 2,
  SW_PACK41             = 0x1 << 2,
  SW_UNPACK_EVEN_LANES  = 0x0 << 2,
  SW_UNPACK_ODD_LANES   = 0x1 << 2,
  // MOV_GROUP
  SW_GROUP0_EN          = 0x1 << 0,
  SW_GROUP1_EN          = 0x1 << 1,
  SW_DUAL_GROUP0_EN     = 0x1 << 2,
  SW_DUAL_GROUP1_EN     = 0x1 << 3,
  SW_DUAL_GROUP2_EN     = 0x1 << 4,
  SW_DUAL_GROUP3_EN     = 0x1 << 5,
  // MSAC
  /* SW_RHU - see previous definition */
  SW_NORMALIZE_AB       = 0x0 << 2,
  SW_NORMALIZE_C        = 0x1 << 2,
  // MUL
  SW_KEEP_RS            = 0x2 << 0,
  SW_KEEP_RS_FOR_ADD    = 0x3 << 0,
  SW_UPPER32            = 0x1 << 2,
  // PACK
  SW_GROUP_0            = 0x0 << (0 + 8),
  SW_GROUP_1            = 0x1 << (0 + 8),
  SW_STRIDE_2           = 0x0 << (1 + 8),
  SW_STRIDE_4           = 0x1 << (1 + 8),
  // POPCNT
  SW_COUNT_ZEROS        = 0x0 << 0,
  SW_COUNT_ONES         = 0x1 << 0,
  // ST_TNSR_S
  SW_ST_TNSR_S_BV64     = 0x1 << 2,
  // ST_TNSR
  SW_PACK               = 0x1 << 2,
  SW_PCK_32_TO_16       = 0x0 << 4,
  SW_PCK_16_TO_8        = 0x1 << 4,
  SW_PCK_32_TO_8        = 0x2 << 4,
  SW_PCK_8_TO_4         = 0x3 << 4,
  // RMW_SEL
  RMW_DT_INT16          = 0x1 << 0,
  RMW_DT_INT32          = 0x2 << 0,
  RMW_DT_INT8           = 0x0 << 0,
  RMW_DT_UINT8          = 0x3 << 0,
  RMW_DT_UINT16         = 0x4 << 0,
  RMW_DT_UINT32         = 0x5 << 0,
  RMW_DT_BF16           = 0x6 << 0,
  RMW_DT_FP32           = 0x7 << 0,
  RMW_DT_FP16           = 0x8 << 0,
  RMW_DT_FP8_152        = 0x9 << 0,
  RMW_OP_ADD            = 0x0 << 4,
  RMW_OP_SUB            = 0x1 << 4,
  RMW_OP_MIN            = 0x2 << 4,
  RMW_OP_MAX            = 0x3 << 4,
  RMW_OP_MAX_0_ADD      = 0x4 << 4,
  RMW_TNSR_DT           = 0x1 << 7,
  RMW_SET               = 0x1 << 6,
  // SUB
  /* SW_SAT - see previous definition */
  /* SW_NEG - see previous definition */
  SW_NO_BORROW_GEN      = 0x1 << 2,
  SW_BORROW             = 0x1 << 3,
  // UNPACK
  /* SW_GROUP_0 - see previous definition */
  /* SW_GROUP_1 - see previous definition */
  /* SW_STRIDE_2 - see previous definition */
  /* SW_STRIDE_4 - see previous definition */
  SW_GROUP_HALF_0       = 0x0 << (2 + 8),
  SW_GROUP_HALF_1       = 0x1 << (2 + 8),
  SW_UNPACK_LANE_0      = 0x0 << (3 + 8),
  SW_UNPACK_LANE_1      = 0x1 << (3 + 8),
  SW_UNPACK_LANE_2      = 0x2 << (3 + 8),
  SW_UNPACK_LANE_3      = 0x3 << (3 + 8),
  // UDIV_4STEP
  SW_X2_UDIV_4STEP      = 0x1 << 6,
  // UDIV
  SW_DIV_MODE_DIV       = 0x0 << 0,
  SW_DIV_MODE_MOD       = 0x1 << 0,
  SW_DIV_MODE_BOTH      = 0x2 << 0,

};

// Instruction switches. Must agree with definitions in 'lib/Target/TPC/Utils/InstructionDB.h'.
enum {
  SW_GROUP_HALF      = 1U << 10,
  SW_SUBTRACT_BIAS   = 1U << 0,
  SW_GROUP_RND32     = 0x03,
  SW_NO_ROUND        = 0,
  SW_PACK_DT         = 0x03 << 4,
  SW_GROUP_SOURCE    = 1 << 8,
  SW_ELEMENT_STRIDE  = 1 << 9,
  SW_LANE_0          = 0,
  SW_LANE_1          = 0x1,
  SW_LANE_2          = 0x2,
  SW_LANE_3          = 0x3,
  SW_LD_G_PARTIAL    = 1 << 7,
  SW_NUM_LANES_SRCB     = 1U << 6,
  SW_ALL_LANES_SRCB     = 0,
  SW_SINGLE_LANE_SRCB   = SW_NUM_LANES_SRCB,
};

// Data types.
enum {
  SW_TYPE     = 0x0f,
  SW_FP32     = 0,
  SW_BF16     = 1,
  SW_INT32    = 2,
  SW_UINT32   = 3,
  SW_INT8     = 4,
  SW_UINT8    = 5,
  SW_BOOL     = 6,
  SW_INT16    = 7,
  SW_UINT16   = 8
};

// Intrinsic aliases for backword compatibility.
#if !defined(NO_TPC_INTRINSICS_ALIASES)

// Load slot.

#define a_gen_addr_i(i, t)                gen_addr(i, t, 0, 0, 1, 0)

#define i_prmt_indx_i_s_b(c, t, i, p, o)  prmt_indx(c, t, 0, i, p, o)
#define i_prmt_indx_i_s(c, t)             i_prmt_indx_i_s_b(c, t, 0, 1, 0)

#define i_i32_set_indx_s_b(v, n, m, p, o) set_indx(v, n, m, 0, p, o)
#define i_i32_set_indx_s(v, n, m)         i_i32_set_indx_s_b(v, n, m, 1, 0)


#define s_f32_ld_l_s_b(a, i, s, p, o)   s_f32_ld_l(a, s, i, p, o)
#define s_bf16_ld_l_s_b(a, i, s, p, o)  s_bf16_ld_l(a, s, i, p, o)
#define s_i32_ld_l_s_b(a, i, s, p, o)   s_i32_ld_l(a, s, i, p, o)
#define s_u32_ld_l_s_b(a, i, s, p, o)   s_u32_ld_l(a, s, i, p, o)
#define s_i16_ld_l_s_b(a, i, s, p, o)   s_i16_ld_l(a, s, i, p, o)
#define s_u16_ld_l_s_b(a, i, s, p, o)   s_u16_ld_l(a, s, i, p, o)
#define s_i8_ld_l_s_b(a, i, s, p, o)    s_i8_ld_l(a, s, i, p, o)
#define s_u8_ld_l_s_b(a, i, s, p, o)    s_u8_ld_l(a, s, i, p, o)
#define s_u8_ld_l_s_b(a, i, s, p, o)    s_u8_ld_l(a, s, i, p, o)
#define b_b_ld_l_s_b(a, i, s, p, o)     s_i1_ld_l(a, s, i, p, o)

#define s_f32_ld_l_s(a, s)              s_f32_ld_l_s_b(a, 0, s, 1, 0)
#define s_bf16_ld_l_s(a, s)             s_bf16_ld_l_s_b(a, 0, s, 1, 0)
#define s_i32_ld_l_s(a, s)              s_i32_ld_l_s_b(a, 0, s, 1, 0)
#define s_u32_ld_l_s(a, s)              s_u32_ld_l_s_b(a, 0, s, 1, 0)
#define s_i16_ld_l_s(a, s)              s_i16_ld_l_s_b(a, 0, s, 1, 0)
#define s_u16_ld_l_s(a, s)              s_u16_ld_l_s_b(a, 0, s, 1, 0)
#define s_i8_ld_l_s(a, s)               s_i8_ld_l_s_b(a, 0, s, 1, 0)
#define s_u8_ld_l_s(a, s)               s_u8_ld_l_s_b(a, 0, s, 1, 0)
#define b_b_ld_l_s(a, s)                b_b_ld_l_s_b(a, 0, s, 1, 0)


#define s_f32_ld_g_a_b(a, i, p, o)      s_f32_ld_g(a, 0, i, p, o)
#define s_bf16_ld_g_a_b(a, i, p, o)     s_bf16_ld_g(a, 0, i, p, o)
#define s_i32_ld_g_a_b(a, i, p, o)      s_i32_ld_g(a, 0, i, p, o)
#define s_u32_ld_g_a_b(a, i, p, o)      s_u32_ld_g(a, 0, i, p, o)
#define s_i16_ld_g_a_b(a, i, p, o)      s_i16_ld_g(a, 0, i, p, o)
#define s_u16_ld_g_a_b(a, i, p, o)      s_u16_ld_g(a, 0, i, p, o)
#define s_i8_ld_g_a_b(a, i, p, o)       s_i8_ld_g(a, 0, i, p, o)
#define s_u8_ld_g_a_b(a, i, p, o)       s_u8_ld_g(a, 0, i, p, o)
#define b_b_ld_g_a_b(a, i, p, o)        s_i1_ld_g(a, 0, i, p, o)

#define v_f32_ld_g_a_b(a, i, p, o)      v_f32_ld_g(a, 0, i, p, o)
#define v_bf16_ld_g_a_b(a, i, p, o)     v_bf16_ld_g(a, 0, i, p, o)
#define v_i32_ld_g_a_b(a, i, p, o)      v_i32_ld_g(a, 0, i, p, o)
#define v_u32_ld_g_a_b(a, i, p, o)      v_u32_ld_g(a, 0, i, p, o)
#define v_i16_ld_g_a_b(a, i, p, o)      v_i16_ld_g(a, 0, i, p, o)
#define v_u16_ld_g_a_b(a, i, p, o)      v_u16_ld_g(a, 0, i, p, o)
#define v_i8_ld_g_a_b(a, i, p, o)       v_i8_ld_g(a, 0, i, p, o)
#define v_u8_ld_g_a_b(a, i, p, o)       v_u8_ld_g(a, 0, i, p, o)

#define s_f32_ld_g_a(a)                 s_f32_ld_g_a_b(a, 0, 1, 0)
#define s_bf16_ld_g_a(a)                s_bf16_ld_g_a_b(a, 0, 1, 0)
#define s_i32_ld_g_a(a)                 s_i32_ld_g_a_b(a, 0, 1, 0)
#define s_u32_ld_g_a(a)                 s_u32_ld_g_a_b(a, 0, 1, 0)
#define s_i16_ld_g_a(a)                 s_i16_ld_g_a_b(a, 0, 1, 0)
#define s_u16_ld_g_a(a)                 s_u16_ld_g_a_b(a, 0, 1, 0)
#define s_i8_ld_g_a(a)                  s_i8_ld_g_a_b(a, 0, 1, 0)
#define s_u8_ld_g_a(a)                  s_u8_ld_g_a_b(a, 0, 1, 0)
#define b_b_ld_g_a(a)                   b_b_ld_g_a_b(a, 0, 1, 0)

#define v_f32_ld_g_a(a)                 v_f32_ld_g_a_b(a, 0, 1, 0)
#define v_bf16_ld_g_a(a)                v_bf16_ld_g_a_b(a, 0, 1, 0)
#define v_i32_ld_g_a(a)                 v_i32_ld_g_a_b(a, 0, 1, 0)
#define v_u32_ld_g_a(a)                 v_u32_ld_g_a_b(a, 0, 1, 0)
#define v_i16_ld_g_a(a)                 v_i16_ld_g_a_b(a, 0, 1, 0)
#define v_u16_ld_g_a(a)                 v_u16_ld_g_a_b(a, 0, 1, 0)
#define v_i8_ld_g_a(a)                  v_i8_ld_g_a_b(a, 0, 1, 0)
#define v_u8_ld_g_a(a)                  v_u8_ld_g_a_b(a, 0, 1, 0)


#define v_f32_ld_l_v_s_vb(a, i, p, o)   v_f32_ld_l_v_vb(a, 0, i, to_bool64(p), o)
#define v_bf16_ld_l_v_s_vb(a, i, p, o)  v_bf16_ld_l_v_vb(a, 0, i, to_bool128(p), o)
#define v_i32_ld_l_v_s_vb(a, i, p, o)   v_i32_ld_l_v_vb(a, 0, i, to_bool64(p), o)
#define v_u32_ld_l_v_s_vb(a, i, p, o)   v_u32_ld_l_v_vb(a, 0, i, to_bool64(p), o)
#define v_i16_ld_l_v_s_vb(a, i, p, o)   v_i16_ld_l_v_vb(a, 0, i, to_bool128(p), o)
#define v_u16_ld_l_v_s_vb(a, i, p, o)   v_u16_ld_l_v_vb(a, 0, i, to_bool128(p), o)
#define v_i8_ld_l_v_s_vb(a, i, p, o)    v_i8_ld_l_v_vb(a, 0, i, p, o)
#define v_u8_ld_l_v_s_vb(a, i, p, o)    v_u8_ld_l_v_vb(a, 0, i, p, o)
#define bv_ld_l_v_s_vb(a, i, p, o)      v_i1_ld_l_v_vb(a, 0, i, p, o)

#define v_f32_ld_l_v_s_b(a, i, p, o)    v_f32_ld_l_v_b(a, 0, i, p, o)
#define v_bf16_ld_l_v_s_b(a, i, p, o)   v_bf16_ld_l_v_b(a, 0, i, p, o)
#define v_i32_ld_l_v_s_b(a, i, p, o)    v_i32_ld_l_v_b(a, 0, i, p, o)
#define v_u32_ld_l_v_s_b(a, i, p, o)    v_u32_ld_l_v_b(a, 0, i, p, o)
#define v_i16_ld_l_v_s_b(a, i, p, o)    v_i16_ld_l_v_b(a, 0, i, p, o)
#define v_u16_ld_l_v_s_b(a, i, p, o)    v_u16_ld_l_v_b(a, 0, i, p, o)
#define v_i8_ld_l_v_s_b(a, i, p, o)     v_i8_ld_l_v_b(a, 0, i, p, o)
#define v_u8_ld_l_v_s_b(a, i, p, o)     v_u8_ld_l_v_b(a, 0, i, p, o)
#define bv_ld_l_v_s_b(a, i, p, o)       v_i1_ld_l_v_b(a, 0, i, p, o)

#define v_f32_ld_l_v_s(a)               v_f32_ld_l_v_s_b(a, 0, 1, 0)
#define v_bf16_ld_l_v_s(a)              v_bf16_ld_l_v_s_b(a, 0, 1, 0)
#define v_i32_ld_l_v_s(a)               v_i32_ld_l_v_s_b(a, 0, 1, 0)
#define v_u32_ld_l_v_s(a)               v_u32_ld_l_v_s_b(a, 0, 1, 0)
#define v_i16_ld_l_v_s(a)               v_i16_ld_l_v_s_b(a, 0, 1, 0)
#define v_u16_ld_l_v_s(a)               v_u16_ld_l_v_s_b(a, 0, 1, 0)
#define v_i8_ld_l_v_s(a)                v_i8_ld_l_v_s_b(a, 0, 1, 0)
#define v_u8_ld_l_v_s(a)                v_u8_ld_l_v_s_b(a, 0, 1, 0)
#define bv_ld_l_v_s(a)                  bv_ld_l_v_s_b(a, (bool256){0}, 1, 0)

#define v_f32_ld_l_v_high_s_vb(a, i, p, o)   v_f32_ld_l_v_high_vb(a, 0, i, to_bool64(p), o)
#define v_bf16_ld_l_v_high_s_vb(a, i, p, o)  v_bf16_ld_l_v_high_vb(a, 0, i, to_bool128(p), o)
#define v_i32_ld_l_v_high_s_vb(a, i, p, o)   v_i32_ld_l_v_high_vb(a, 0, i, to_bool64(p), o)
#define v_u32_ld_l_v_high_s_vb(a, i, p, o)   v_u32_ld_l_v_high_vb(a, 0, i, to_bool64(p), o)
#define v_i16_ld_l_v_high_s_vb(a, i, p, o)   v_i16_ld_l_v_high_vb(a, 0, i, to_bool128(p), o)
#define v_u16_ld_l_v_high_s_vb(a, i, p, o)   v_u16_ld_l_v_high_vb(a, 0, i, to_bool128(p), o)
#define v_i8_ld_l_v_high_s_vb(a, i, p, o)    v_i8_ld_l_v_high_vb(a, 0, i, p, o)
#define v_u8_ld_l_v_high_s_vb(a, i, p, o)    v_u8_ld_l_v_high_vb(a, 0, i, p, o)
#define bv_ld_l_v_high_s_vb(a, i, p, o)      v_i1_ld_l_v_high_vb(a, 0, i, p, o)

#define v_f32_ld_l_v_high_s_b(a, i, p, o)    v_f32_ld_l_v_high_b(a, 0, i, p, o)
#define v_bf16_ld_l_v_high_s_b(a, i, p, o)   v_bf16_ld_l_v_high_b(a, 0, i, p, o)
#define v_i32_ld_l_v_high_s_b(a, i, p, o)    v_i32_ld_l_v_high_b(a, 0, i, p, o)
#define v_u32_ld_l_v_high_s_b(a, i, p, o)    v_u32_ld_l_v_high_b(a, 0, i, p, o)
#define v_i16_ld_l_v_high_s_b(a, i, p, o)    v_i16_ld_l_v_high_b(a, 0, i, p, o)
#define v_u16_ld_l_v_high_s_b(a, i, p, o)    v_u16_ld_l_v_high_b(a, 0, i, p, o)
#define v_i8_ld_l_v_high_s_b(a, i, p, o)     v_i8_ld_l_v_high_b(a, 0, i, p, o)
#define v_u8_ld_l_v_high_s_b(a, i, p, o)     v_u8_ld_l_v_high_b(a, 0, i, p, o)
#define bv_ld_l_v_high_s_b(a, i, p, o)       v_i1_ld_l_v_high_b(a, 0, i, p, o)

#define v_f32_ld_l_v_high_s(a)               v_f32_ld_l_v_high_s_b(a, 0, 1, 0)
#define v_bf16_ld_l_v_high_s(a)              v_bf16_ld_l_v_high_s_b(a, 0, 1, 0)
#define v_i32_ld_l_v_high_s(a)               v_i32_ld_l_v_high_s_b(a, 0, 1, 0)
#define v_u32_ld_l_v_high_s(a)               v_u32_ld_l_v_high_s_b(a, 0, 1, 0)
#define v_i16_ld_l_v_high_s(a)               v_i16_ld_l_v_high_s_b(a, 0, 1, 0)
#define v_u16_ld_l_v_high_s(a)               v_u16_ld_l_v_high_s_b(a, 0, 1, 0)
#define v_i8_ld_l_v_high_s(a)                v_i8_ld_l_v_high_s_b(a, 0, 1, 0)
#define v_u8_ld_l_v_high_s(a)                v_u8_ld_l_v_high_s_b(a, 0, 1, 0)
#define bv_ld_l_v_high_s(a)                  bv_ld_l_v_high_s_b(a, (bool256){0}, 1, 0)

#define v_f32_ld_l_v_low_s_vb(a, i, p, o)   v_f32_ld_l_v_low_vb(a, 0, i, to_bool64(p), o)
#define v_bf16_ld_l_v_low_s_vb(a, i, p, o)  v_bf16_ld_l_v_low_vb(a, 0, i, to_bool128(p), o)
#define v_i32_ld_l_v_low_s_vb(a, i, p, o)   v_i32_ld_l_v_low_vb(a, 0, i, to_bool64(p), o)
#define v_u32_ld_l_v_low_s_vb(a, i, p, o)   v_u32_ld_l_v_low_vb(a, 0, i, to_bool64(p), o)
#define v_i16_ld_l_v_low_s_vb(a, i, p, o)   v_i16_ld_l_v_low_vb(a, 0, i, to_bool128(p), o)
#define v_u16_ld_l_v_low_s_vb(a, i, p, o)   v_u16_ld_l_v_low_vb(a, 0, i, to_bool128(p), o)
#define v_i8_ld_l_v_low_s_vb(a, i, p, o)    v_i8_ld_l_v_low_vb(a, 0, i, p, o)
#define v_u8_ld_l_v_low_s_vb(a, i, p, o)    v_u8_ld_l_v_low_vb(a, 0, i, p, o)
#define bv_ld_l_v_low_s_vb(a, i, p, o)      v_i1_ld_l_v_low_vb(a, 0, i, p, o)

#define v_f32_ld_l_v_low_s_b(a, i, p, o)    v_f32_ld_l_v_low_b(a, 0, i, p, o)
#define v_bf16_ld_l_v_low_s_b(a, i, p, o)   v_bf16_ld_l_v_low_b(a, 0, i, p, o)
#define v_i32_ld_l_v_low_s_b(a, i, p, o)    v_i32_ld_l_v_low_b(a, 0, i, p, o)
#define v_u32_ld_l_v_low_s_b(a, i, p, o)    v_u32_ld_l_v_low_b(a, 0, i, p, o)
#define v_i16_ld_l_v_low_s_b(a, i, p, o)    v_i16_ld_l_v_low_b(a, 0, i, p, o)
#define v_u16_ld_l_v_low_s_b(a, i, p, o)    v_u16_ld_l_v_low_b(a, 0, i, p, o)
#define v_i8_ld_l_v_low_s_b(a, i, p, o)     v_i8_ld_l_v_low_b(a, 0, i, p, o)
#define v_u8_ld_l_v_low_s_b(a, i, p, o)     v_u8_ld_l_v_low_b(a, 0, i, p, o)
#define bv_ld_l_v_low_s_b(a, i, p, o)       v_i1_ld_l_v_low_b(a, 0, i, p, o)

#define v_f32_ld_l_v_low_s(a)               v_f32_ld_l_v_low_s_b(a, 0, 1, 0)
#define v_bf16_ld_l_v_low_s(a)              v_bf16_ld_l_v_low_s_b(a, 0, 1, 0)
#define v_i32_ld_l_v_low_s(a)               v_i32_ld_l_v_low_s_b(a, 0, 1, 0)
#define v_u32_ld_l_v_low_s(a)               v_u32_ld_l_v_low_s_b(a, 0, 1, 0)
#define v_i16_ld_l_v_low_s(a)               v_i16_ld_l_v_low_s_b(a, 0, 1, 0)
#define v_u16_ld_l_v_low_s(a)               v_u16_ld_l_v_low_s_b(a, 0, 1, 0)
#define v_i8_ld_l_v_low_s(a)                v_i8_ld_l_v_low_s_b(a, 0, 1, 0)
#define v_u8_ld_l_v_low_s(a)                v_u8_ld_l_v_low_s_b(a, 0, 1, 0)
#define bv_ld_l_v_low_s(a)                  bv_ld_l_v_low_s_b(a, (bool256){0}, 1, 0)

#define prefetch_a_b(a, p, o)               prefetch(a, 0, p, o)
#define prefetch_a(a)                       prefetch_a_b(a, 1, 0)

// LD_TNSR

#define v_f32_ld_tnsr_i_b(n, t, i, p, o)    v_f32_ld_tnsr_b(n, t, 0, i, p, o)
#define v_bf16_ld_tnsr_i_b(n, t, i, p, o)   v_bf16_ld_tnsr_b(n, t, 0, i, p, o)
#define v_i32_ld_tnsr_i_b(n, t, i, p, o)    v_i32_ld_tnsr_b(n, t, 0, i, p, o)
#define v_u32_ld_tnsr_i_b(n, t, i, p, o)    v_u32_ld_tnsr_b(n, t, 0, i, p, o)
#define v_i16_ld_tnsr_i_b(n, t, i, p, o)    v_i16_ld_tnsr_b(n, t, 0, i, p, o)
#define v_u16_ld_tnsr_i_b(n, t, i, p, o)    v_u16_ld_tnsr_b(n, t, 0, i, p, o)
#define v_i8_ld_tnsr_i_b(n, t, i, p, o)     v_i8_ld_tnsr_b(n, t, 0, i, p, o)
#define v_u8_ld_tnsr_i_b(n, t, i, p, o)     v_u8_ld_tnsr_b(n, t, 0, i, p, o)
#define bv_ld_tnsr_i_b(n, t, i, p, o)       v_i1_ld_tnsr_b(n, t, 0, i, p, o)

#define v_f32_ld_tnsr_i_vb(n, t, i, p, o)   v_f32_ld_tnsr_vb(n, t, 0, i, to_bool64(p), o)
#define v_bf16_ld_tnsr_i_vb(n, t, i, p, o)  v_bf16_ld_tnsr_vb(n, t, 0, i, to_bool128(p), o)
#define v_i32_ld_tnsr_i_vb(n, t, i, p, o)   v_i32_ld_tnsr_vb(n, t, 0, i, to_bool64(p), o)
#define v_u32_ld_tnsr_i_vb(n, t, i, p, o)   v_u32_ld_tnsr_vb(n, t, 0, i, to_bool64(p), o)
#define v_i16_ld_tnsr_i_vb(n, t, i, p, o)   v_i16_ld_tnsr_vb(n, t, 0, i, to_bool128(p), o)
#define v_u16_ld_tnsr_i_vb(n, t, i, p, o)   v_u16_ld_tnsr_vb(n, t, 0, i, to_bool128(p), o)
#define v_i8_ld_tnsr_i_vb(n, t, i, p, o)    v_i8_ld_tnsr_vb(n, t, 0, i, p, o)
#define v_u8_ld_tnsr_i_vb(n, t, i, p, o)    v_u8_ld_tnsr_vb(n, t, 0, i, p, o)
#define bv_ld_tnsr_i_vb(n, t, i, p, o)      v_i1_ld_tnsr_vb(n, t, 0, i, p, o)

#define v_f32_ld_tnsr_i(n, t)               v_f32_ld_tnsr_i_b(n, t, 0, 1, 0)
#define v_bf16_ld_tnsr_i(n, t)              v_bf16_ld_tnsr_i_b(n, t, 0, 1, 0)
#define v_i32_ld_tnsr_i(n, t)               v_i32_ld_tnsr_i_b(n, t, 0, 1, 0)
#define v_u32_ld_tnsr_i(n, t)               v_u32_ld_tnsr_i_b(n, t, 0, 1, 0)
#define v_i16_ld_tnsr_i(n, t)               v_i16_ld_tnsr_i_b(n, t, 0, 1, 0)
#define v_u16_ld_tnsr_i(n, t)               v_u16_ld_tnsr_i_b(n, t, 0, 1, 0)
#define v_i8_ld_tnsr_i(n, t)                v_i8_ld_tnsr_i_b(n, t, 0, 1, 0)
#define v_u8_ld_tnsr_i(n, t)                v_u8_ld_tnsr_i_b(n, t, 0, 1, 0)
#define bv_ld_tnsr_i(n, t)                  bv_ld_tnsr_i_b(n, t, (bool256){0}, 1, 0)

// TODO: Deprecate them vvv
#define v_f32_ld_tnsr_reg_i_s_b     v_f32_ld_tnsr_i_b
#define v_bf16_ld_tnsr_reg_i_s_b    v_bf16_ld_tnsr_i_b
#define v_i32_ld_tnsr_reg_i_s_b     v_i32_ld_tnsr_i_b
#define v_u32_ld_tnsr_reg_i_s_b     v_u32_ld_tnsr_i_b
#define v_i16_ld_tnsr_reg_i_s_b     v_i16_ld_tnsr_i_b
#define v_u16_ld_tnsr_reg_i_s_b     v_u16_ld_tnsr_i_b
#define v_i8_ld_tnsr_reg_i_s_b      v_i8_ld_tnsr_i_b
#define v_u8_ld_tnsr_reg_i_s_b      v_u8_ld_tnsr_i_b
#define bv_ld_tnsr_reg_i_s_b        bv_ld_tnsr_i_b

#define v_f32_ld_tnsr_reg_i_s_vb    v_f32_ld_tnsr_i_vb
#define v_bf16_ld_tnsr_reg_i_s_vb   v_bf16_ld_tnsr_i_vb
#define v_i32_ld_tnsr_reg_i_s_vb    v_i32_ld_tnsr_i_vb
#define v_u32_ld_tnsr_reg_i_s_vb    v_u32_ld_tnsr_i_vb
#define v_i16_ld_tnsr_reg_i_s_vb    v_i16_ld_tnsr_i_vb
#define v_u16_ld_tnsr_reg_i_s_vb    v_u16_ld_tnsr_i_vb
#define v_i8_ld_tnsr_reg_i_s_vb     v_i8_ld_tnsr_i_vb
#define v_u8_ld_tnsr_reg_i_s_vb     v_u8_ld_tnsr_i_vb
#define bv_ld_tnsr_reg_i_s_vb       bv_ld_tnsr_i_vb

#define v_f32_ld_tnsr_reg_i_s       v_f32_ld_tnsr_i
#define v_bf16_ld_tnsr_reg_i_s      v_bf16_ld_tnsr_i
#define v_i32_ld_tnsr_reg_i_s       v_i32_ld_tnsr_i
#define v_u32_ld_tnsr_reg_i_s       v_u32_ld_tnsr_i
#define v_i16_ld_tnsr_reg_i_s       v_i16_ld_tnsr_i
#define v_u16_ld_tnsr_reg_i_s       v_u16_ld_tnsr_i
#define v_i8_ld_tnsr_reg_i_s        v_i8_ld_tnsr_i
#define v_u8_ld_tnsr_reg_i_s        v_u8_ld_tnsr_i
#define bv_ld_tnsr_reg_i_s          bv_ld_tnsr_i
// TODO: Deprecate them ^^^

#define v_f32_ld_tnsr_partial_i_b(n, t, i, s, f, p, o)    v_f32_ld_tnsr_partial_b(n, t, s, f, 0, i, p, o)
#define v_bf16_ld_tnsr_partial_i_b(n, t, i, s, f, p, o)   v_bf16_ld_tnsr_partial_b(n, t, s, f, 0, i, p, o)
#define v_i32_ld_tnsr_partial_i_b(n, t, i, s, f, p, o)    v_i32_ld_tnsr_partial_b(n, t, s, f, 0, i, p, o)
#define v_u32_ld_tnsr_partial_i_b(n, t, i, s, f, p, o)    v_u32_ld_tnsr_partial_b(n, t, s, f, 0, i, p, o)
#define v_i16_ld_tnsr_partial_i_b(n, t, i, s, f, p, o)    v_i16_ld_tnsr_partial_b(n, t, s, f, 0, i, p, o)
#define v_u16_ld_tnsr_partial_i_b(n, t, i, s, f, p, o)    v_u16_ld_tnsr_partial_b(n, t, s, f, 0, i, p, o)
#define v_i8_ld_tnsr_partial_i_b(n, t, i, s, f, p, o)     v_i8_ld_tnsr_partial_b(n, t, s, f, 0, i, p, o)
#define v_u8_ld_tnsr_partial_i_b(n, t, i, s, f, p, o)     v_u8_ld_tnsr_partial_b(n, t, s, f, 0, i, p, o)
#define bv_ld_tnsr_partial_i_b(n, t, i, s, f, p, o)       v_i1_ld_tnsr_partial_b(n, t, s, f, 0, i, p, o)

#define v_f32_ld_tnsr_partial_i_vb(n, t, i, s, f, p, o)   v_f32_ld_tnsr_partial_vb(n, t, s, f, 0, i, to_bool64(p), o)
#define v_bf16_ld_tnsr_partial_i_vb(n, t, i, s, f, p, o)  v_bf16_ld_tnsr_partial_vb(n, t, s, f, 0, i, to_bool128(p), o)
#define v_i32_ld_tnsr_partial_i_vb(n, t, i, s, f, p, o)   v_i32_ld_tnsr_partial_vb(n, t, s, f, 0, i, to_bool64(p), o)
#define v_u32_ld_tnsr_partial_i_vb(n, t, i, s, f, p, o)   v_u32_ld_tnsr_partial_vb(n, t, s, f, 0, i, to_bool64(p), o)
#define v_i16_ld_tnsr_partial_i_vb(n, t, i, s, f, p, o)   v_i16_ld_tnsr_partial_vb(n, t, s, f, 0, i, to_bool128(p), o)
#define v_u16_ld_tnsr_partial_i_vb(n, t, i, s, f, p, o)   v_u16_ld_tnsr_partial_vb(n, t, s, f, 0, i, to_bool128(p), o)
#define v_i8_ld_tnsr_partial_i_vb(n, t, i, s, f, p, o)    v_i8_ld_tnsr_partial_vb(n, t, s, f, 0, i, p, o)
#define v_u8_ld_tnsr_partial_i_vb(n, t, i, s, f, p, o)    v_u8_ld_tnsr_partial_vb(n, t, s, f, 0, i, p, o)
#define bv_ld_tnsr_partial_i_vb(n, t, i, s, f, p, o)      v_i1_ld_tnsr_partial_vb(n, t, s, f, 0, i, p, o)

#define v_f32_ld_tnsr_partial_i(n, t, s, f)               v_f32_ld_tnsr_partial_i_b(n, t, 0, s, f, 1, 0)
#define v_bf16_ld_tnsr_partial_i(n, t, s, f)              v_bf16_ld_tnsr_partial_i_b(n, t, 0, s, f, 1, 0)
#define v_i32_ld_tnsr_partial_i(n, t, s, f)               v_i32_ld_tnsr_partial_i_b(n, t, 0, s, f, 1, 0)
#define v_u32_ld_tnsr_partial_i(n, t, s, f)               v_u32_ld_tnsr_partial_i_b(n, t, 0, s, f, 1, 0)
#define v_i16_ld_tnsr_partial_i(n, t, s, f)               v_i16_ld_tnsr_partial_i_b(n, t, 0, s, f, 1, 0)
#define v_u16_ld_tnsr_partial_i(n, t, s, f)               v_u16_ld_tnsr_partial_i_b(n, t, 0, s, f, 1, 0)
#define v_i8_ld_tnsr_partial_i(n, t, s, f)                v_i8_ld_tnsr_partial_i_b(n, t, 0, s, f, 1, 0)
#define v_u8_ld_tnsr_partial_i(n, t, s, f)                v_u8_ld_tnsr_partial_i_b(n, t, 0, s, f, 1, 0)
#define bv_ld_tnsr_partial_i(n, t, s, f)                  bv_ld_tnsr_partial_i_b(n, t, (bool256){0}, s, f, 1, 0)

// TODO: Deprecate them vvv
#define v_f32_ld_tnsr_partial_reg_i_s_b     v_f32_ld_tnsr_partial_i_b
#define v_bf16_ld_tnsr_partial_reg_i_s_b    v_bf16_ld_tnsr_partial_i_b
#define v_i32_ld_tnsr_partial_reg_i_s_b     v_i32_ld_tnsr_partial_i_b
#define v_u32_ld_tnsr_partial_reg_i_s_b     v_u32_ld_tnsr_partial_i_b
#define v_i16_ld_tnsr_partial_reg_i_s_b     v_i16_ld_tnsr_partial_i_b
#define v_u16_ld_tnsr_partial_reg_i_s_b     v_u16_ld_tnsr_partial_i_b
#define v_i8_ld_tnsr_partial_reg_i_s_b      v_i8_ld_tnsr_partial_i_b
#define v_u8_ld_tnsr_partial_reg_i_s_b      v_u8_ld_tnsr_partial_i_b
#define bv_ld_tnsr_partial_reg_i_s_b        bv_ld_tnsr_partial_i_b

#define v_f32_ld_tnsr_partial_reg_i_s_vb    v_f32_ld_tnsr_partial_i_vb
#define v_bf16_ld_tnsr_partial_reg_i_s_vb   v_bf16_ld_tnsr_partial_i_vb
#define v_i32_ld_tnsr_partial_reg_i_s_vb    v_i32_ld_tnsr_partial_i_vb
#define v_u32_ld_tnsr_partial_reg_i_s_vb    v_u32_ld_tnsr_partial_i_vb
#define v_i16_ld_tnsr_partial_reg_i_s_vb    v_i16_ld_tnsr_partial_i_vb
#define v_u16_ld_tnsr_partial_reg_i_s_vb    v_u16_ld_tnsr_partial_i_vb
#define v_i8_ld_tnsr_partial_reg_i_s_vb     v_i8_ld_tnsr_partial_i_vb
#define v_u8_ld_tnsr_partial_reg_i_s_vb     v_u8_ld_tnsr_partial_i_vb
#define bv_ld_tnsr_partial_reg_i_s_vb       bv_ld_tnsr_partial_i_vb

#define v_f32_ld_tnsr_partial_reg_i_s       v_f32_ld_tnsr_partial_i
#define v_bf16_ld_tnsr_partial_reg_i_s      v_bf16_ld_tnsr_partial_i
#define v_i32_ld_tnsr_partial_reg_i_s       v_i32_ld_tnsr_partial_i
#define v_u32_ld_tnsr_partial_reg_i_s       v_u32_ld_tnsr_partial_i
#define v_i16_ld_tnsr_partial_reg_i_s       v_i16_ld_tnsr_partial_i
#define v_u16_ld_tnsr_partial_reg_i_s       v_u16_ld_tnsr_partial_i
#define v_i8_ld_tnsr_partial_reg_i_s        v_i8_ld_tnsr_partial_i
#define v_u8_ld_tnsr_partial_reg_i_s        v_u8_ld_tnsr_partial_i
#define bv_ld_tnsr_partial_reg_i_s          bv_ld_tnsr_partial_i
// TODO: Deprecate them ^^^


// LD_TNSR_LOW

#define v_f32_ld_tnsr_low_i_b(n, t, i, p, o)    v_f32_ld_tnsr_low_b(n, t, 0, i, p, o)
#define v_bf16_ld_tnsr_low_i_b(n, t, i, p, o)   v_bf16_ld_tnsr_low_b(n, t, 0, i, p, o)
#define v_i32_ld_tnsr_low_i_b(n, t, i, p, o)    v_i32_ld_tnsr_low_b(n, t, 0, i, p, o)
#define v_u32_ld_tnsr_low_i_b(n, t, i, p, o)    v_u32_ld_tnsr_low_b(n, t, 0, i, p, o)
#define v_i16_ld_tnsr_low_i_b(n, t, i, p, o)    v_i16_ld_tnsr_low_b(n, t, 0, i, p, o)
#define v_u16_ld_tnsr_low_i_b(n, t, i, p, o)    v_u16_ld_tnsr_low_b(n, t, 0, i, p, o)
#define v_i8_ld_tnsr_low_i_b(n, t, i, p, o)     v_i8_ld_tnsr_low_b(n, t, 0, i, p, o)
#define v_u8_ld_tnsr_low_i_b(n, t, i, p, o)     v_u8_ld_tnsr_low_b(n, t, 0, i, p, o)
#define bv_ld_tnsr_low_i_b(n, t, i, p, o)       v_i1_ld_tnsr_low_b(n, t, 0, i, p, o)

#define v_f32_ld_tnsr_low_i_vb(n, t, i, p, o)   v_f32_ld_tnsr_low_vb(n, t, 0, i, to_bool64(p), o)
#define v_bf16_ld_tnsr_low_i_vb(n, t, i, p, o)  v_bf16_ld_tnsr_low_vb(n, t, 0, i, to_bool128(p), o)
#define v_i32_ld_tnsr_low_i_vb(n, t, i, p, o)   v_i32_ld_tnsr_low_vb(n, t, 0, i, to_bool64(p), o)
#define v_u32_ld_tnsr_low_i_vb(n, t, i, p, o)   v_u32_ld_tnsr_low_vb(n, t, 0, i, to_bool64(p), o)
#define v_i16_ld_tnsr_low_i_vb(n, t, i, p, o)   v_i16_ld_tnsr_low_vb(n, t, 0, i, to_bool128(p), o)
#define v_u16_ld_tnsr_low_i_vb(n, t, i, p, o)   v_u16_ld_tnsr_low_vb(n, t, 0, i, to_bool128(p), o)
#define v_i8_ld_tnsr_low_i_vb(n, t, i, p, o)    v_i8_ld_tnsr_low_vb(n, t, 0, i, p, o)
#define v_u8_ld_tnsr_low_i_vb(n, t, i, p, o)    v_u8_ld_tnsr_low_vb(n, t, 0, i, p, o)
#define bv_ld_tnsr_low_i_vb(n, t, i, p, o)      v_i1_ld_tnsr_low_vb(n, t, 0, i, p, o)

#define v_f32_ld_tnsr_low_i(n, t)               v_f32_ld_tnsr_low_i_b(n, t, 0, 1, 0)
#define v_bf16_ld_tnsr_low_i(n, t)              v_bf16_ld_tnsr_low_i_b(n, t, 0, 1, 0)
#define v_i32_ld_tnsr_low_i(n, t)               v_i32_ld_tnsr_low_i_b(n, t, 0, 1, 0)
#define v_u32_ld_tnsr_low_i(n, t)               v_u32_ld_tnsr_low_i_b(n, t, 0, 1, 0)
#define v_i16_ld_tnsr_low_i(n, t)               v_i16_ld_tnsr_low_i_b(n, t, 0, 1, 0)
#define v_u16_ld_tnsr_low_i(n, t)               v_u16_ld_tnsr_low_i_b(n, t, 0, 1, 0)
#define v_i8_ld_tnsr_low_i(n, t)                v_i8_ld_tnsr_low_i_b(n, t, 0, 1, 0)
#define v_u8_ld_tnsr_low_i(n, t)                v_u8_ld_tnsr_low_i_b(n, t, 0, 1, 0)
#define bv_ld_tnsr_low_i(n, t)                  bv_ld_tnsr_low_i_b(n, t, (bool256){0}, 1, 0)

// TODO: Deprecate them vvv
#define v_f32_ld_tnsr_low_reg_i_s_b     v_f32_ld_tnsr_low_i_b
#define v_bf16_ld_tnsr_low_reg_i_s_b    v_bf16_ld_tnsr_low_i_b
#define v_i32_ld_tnsr_low_reg_i_s_b     v_i32_ld_tnsr_low_i_b
#define v_u32_ld_tnsr_low_reg_i_s_b     v_u32_ld_tnsr_low_i_b
#define v_i16_ld_tnsr_low_reg_i_s_b     v_i16_ld_tnsr_low_i_b
#define v_u16_ld_tnsr_low_reg_i_s_b     v_u16_ld_tnsr_low_i_b
#define v_i8_ld_tnsr_low_reg_i_s_b      v_i8_ld_tnsr_low_i_b
#define v_u8_ld_tnsr_low_reg_i_s_b      v_u8_ld_tnsr_low_i_b
#define bv_ld_tnsr_low_reg_i_s_b        bv_ld_tnsr_low_i_b

#define v_f32_ld_tnsr_low_reg_i_s_vb    v_f32_ld_tnsr_low_i_vb
#define v_bf16_ld_tnsr_low_reg_i_s_vb   v_bf16_ld_tnsr_low_i_vb
#define v_i32_ld_tnsr_low_reg_i_s_vb    v_i32_ld_tnsr_low_i_vb
#define v_u32_ld_tnsr_low_reg_i_s_vb    v_u32_ld_tnsr_low_i_vb
#define v_i16_ld_tnsr_low_reg_i_s_vb    v_i16_ld_tnsr_low_i_vb
#define v_u16_ld_tnsr_low_reg_i_s_vb    v_u16_ld_tnsr_low_i_vb
#define v_i8_ld_tnsr_low_reg_i_s_vb     v_i8_ld_tnsr_low_i_vb
#define v_u8_ld_tnsr_low_reg_i_s_vb     v_u8_ld_tnsr_low_i_vb
#define bv_ld_tnsr_low_reg_i_s_vb       bv_ld_tnsr_low_i_vb

#define v_f32_ld_tnsr_low_reg_i_s       v_f32_ld_tnsr_low_i
#define v_bf16_ld_tnsr_low_reg_i_s      v_bf16_ld_tnsr_low_i
#define v_i32_ld_tnsr_low_reg_i_s       v_i32_ld_tnsr_low_i
#define v_u32_ld_tnsr_low_reg_i_s       v_u32_ld_tnsr_low_i
#define v_i16_ld_tnsr_low_reg_i_s       v_i16_ld_tnsr_low_i
#define v_u16_ld_tnsr_low_reg_i_s       v_u16_ld_tnsr_low_i
#define v_i8_ld_tnsr_low_reg_i_s        v_i8_ld_tnsr_low_i
#define v_u8_ld_tnsr_low_reg_i_s        v_u8_ld_tnsr_low_i
#define bv_ld_tnsr_low_reg_i_s          bv_ld_tnsr_low_i
// TODO: Deprecate them ^^^


// LD_TNSR_HIGH

#define v_f32_ld_tnsr_high_i_b(n, t, i, p, o)    v_f32_ld_tnsr_high_b(n, t, 0, i, p, o)
#define v_bf16_ld_tnsr_high_i_b(n, t, i, p, o)   v_bf16_ld_tnsr_high_b(n, t, 0, i, p, o)
#define v_i32_ld_tnsr_high_i_b(n, t, i, p, o)    v_i32_ld_tnsr_high_b(n, t, 0, i, p, o)
#define v_u32_ld_tnsr_high_i_b(n, t, i, p, o)    v_u32_ld_tnsr_high_b(n, t, 0, i, p, o)
#define v_i16_ld_tnsr_high_i_b(n, t, i, p, o)    v_i16_ld_tnsr_high_b(n, t, 0, i, p, o)
#define v_u16_ld_tnsr_high_i_b(n, t, i, p, o)    v_u16_ld_tnsr_high_b(n, t, 0, i, p, o)
#define v_i8_ld_tnsr_high_i_b(n, t, i, p, o)     v_i8_ld_tnsr_high_b(n, t, 0, i, p, o)
#define v_u8_ld_tnsr_high_i_b(n, t, i, p, o)     v_u8_ld_tnsr_high_b(n, t, 0, i, p, o)
#define bv_ld_tnsr_high_i_b(n, t, i, p, o)       v_i1_ld_tnsr_high_b(n, t, 0, i, p, o)

#define v_f32_ld_tnsr_high_i_vb(n, t, i, p, o)   v_f32_ld_tnsr_high_vb(n, t, 0, i, to_bool64(p), o)
#define v_bf16_ld_tnsr_high_i_vb(n, t, i, p, o)  v_bf16_ld_tnsr_high_vb(n, t, 0, i, to_bool128(p), o)
#define v_i32_ld_tnsr_high_i_vb(n, t, i, p, o)   v_i32_ld_tnsr_high_vb(n, t, 0, i, to_bool64(p), o)
#define v_u32_ld_tnsr_high_i_vb(n, t, i, p, o)   v_u32_ld_tnsr_high_vb(n, t, 0, i, to_bool64(p), o)
#define v_i16_ld_tnsr_high_i_vb(n, t, i, p, o)   v_i16_ld_tnsr_high_vb(n, t, 0, i, to_bool128(p), o)
#define v_u16_ld_tnsr_high_i_vb(n, t, i, p, o)   v_u16_ld_tnsr_high_vb(n, t, 0, i, to_bool128(p), o)
#define v_i8_ld_tnsr_high_i_vb(n, t, i, p, o)    v_i8_ld_tnsr_high_vb(n, t, 0, i, p, o)
#define v_u8_ld_tnsr_high_i_vb(n, t, i, p, o)    v_u8_ld_tnsr_high_vb(n, t, 0, i, p, o)
#define bv_ld_tnsr_high_i_vb(n, t, i, p, o)      v_i1_ld_tnsr_high_vb(n, t, 0, i, p, o)

#define v_f32_ld_tnsr_high_i(n, t)               v_f32_ld_tnsr_high_i_b(n, t, 0, 1, 0)
#define v_bf16_ld_tnsr_high_i(n, t)              v_bf16_ld_tnsr_high_i_b(n, t, 0, 1, 0)
#define v_i32_ld_tnsr_high_i(n, t)               v_i32_ld_tnsr_high_i_b(n, t, 0, 1, 0)
#define v_u32_ld_tnsr_high_i(n, t)               v_u32_ld_tnsr_high_i_b(n, t, 0, 1, 0)
#define v_i16_ld_tnsr_high_i(n, t)               v_i16_ld_tnsr_high_i_b(n, t, 0, 1, 0)
#define v_u16_ld_tnsr_high_i(n, t)               v_u16_ld_tnsr_high_i_b(n, t, 0, 1, 0)
#define v_i8_ld_tnsr_high_i(n, t)                v_i8_ld_tnsr_high_i_b(n, t, 0, 1, 0)
#define v_u8_ld_tnsr_high_i(n, t)                v_u8_ld_tnsr_high_i_b(n, t, 0, 1, 0)
#define bv_ld_tnsr_high_i(n, t)                  bv_ld_tnsr_high_i_b(n, t, (bool256){0}, 1, 0)

// TODO: Deprecate them vvv
#define v_f32_ld_tnsr_high_reg_i_s_b     v_f32_ld_tnsr_high_i_b
#define v_bf16_ld_tnsr_high_reg_i_s_b    v_bf16_ld_tnsr_high_i_b
#define v_i32_ld_tnsr_high_reg_i_s_b     v_i32_ld_tnsr_high_i_b
#define v_u32_ld_tnsr_high_reg_i_s_b     v_u32_ld_tnsr_high_i_b
#define v_i16_ld_tnsr_high_reg_i_s_b     v_i16_ld_tnsr_high_i_b
#define v_u16_ld_tnsr_high_reg_i_s_b     v_u16_ld_tnsr_high_i_b
#define v_i8_ld_tnsr_high_reg_i_s_b      v_i8_ld_tnsr_high_i_b
#define v_u8_ld_tnsr_high_reg_i_s_b      v_u8_ld_tnsr_high_i_b
#define bv_ld_tnsr_high_reg_i_s_b        bv_ld_tnsr_high_i_b

#define v_f32_ld_tnsr_high_reg_i_s_vb    v_f32_ld_tnsr_high_i_vb
#define v_bf16_ld_tnsr_high_reg_i_s_vb   v_bf16_ld_tnsr_high_i_vb
#define v_i32_ld_tnsr_high_reg_i_s_vb    v_i32_ld_tnsr_high_i_vb
#define v_u32_ld_tnsr_high_reg_i_s_vb    v_u32_ld_tnsr_high_i_vb
#define v_i16_ld_tnsr_high_reg_i_s_vb    v_i16_ld_tnsr_high_i_vb
#define v_u16_ld_tnsr_high_reg_i_s_vb    v_u16_ld_tnsr_high_i_vb
#define v_i8_ld_tnsr_high_reg_i_s_vb     v_i8_ld_tnsr_high_i_vb
#define v_u8_ld_tnsr_high_reg_i_s_vb     v_u8_ld_tnsr_high_i_vb
#define bv_ld_tnsr_high_reg_i_s_vb       bv_ld_tnsr_high_i_vb

#define v_f32_ld_tnsr_high_reg_i_s       v_f32_ld_tnsr_high_i
#define v_bf16_ld_tnsr_high_reg_i_s      v_bf16_ld_tnsr_high_i
#define v_i32_ld_tnsr_high_reg_i_s       v_i32_ld_tnsr_high_i
#define v_u32_ld_tnsr_high_reg_i_s       v_u32_ld_tnsr_high_i
#define v_i16_ld_tnsr_high_reg_i_s       v_i16_ld_tnsr_high_i
#define v_u16_ld_tnsr_high_reg_i_s       v_u16_ld_tnsr_high_i
#define v_i8_ld_tnsr_high_reg_i_s        v_i8_ld_tnsr_high_i
#define v_u8_ld_tnsr_high_reg_i_s        v_u8_ld_tnsr_high_i
#define bv_ld_tnsr_high_reg_i_s          bv_ld_tnsr_high_i
// TODO: Deprecate them ^^^

// LOOKUP
#if defined(__gaudi__)
#define UPPER_HALF (1 << 2)
#endif

//--- f32 ---
#define v_f32_lookup_v_b(a, i, t, f, p, o)                  v_f32_lookup(a, f, t, i, p, o)
#define v_f32_lookup_v(a, t, f)                             v_f32_lookup_v_b(a, 0, t, f, 1, 0)
#define v_i16_lookup_v_b(a, i, t, f, p, o)					v_i16_lookup(a, f, t, i, p, o)
#define v_i16_lookup_v(a, i, t, f)							v_i16_lookup_v_b(a, i, t, f, 1, 0)
#define v_i8_lookup_v_b(a, i, t, f, p, o)					v_i8_lookup(a, f, t, i, p, o)
#define v_i8_lookup_v(a, i, t, f)							v_i8_lookup_v_b(a, i, t, f, 1, 0)
#define v_i32_lookup_v_b(a, i, t, f, p, o)					v_i32_lookup(a, f, t, i, p, o)
#define v_i32_lookup_v(a, t, f)								v_i32_lookup_v_b(a, 0, t, f, 1, 0)
#define v_u32_lookup_v_b(a, i, t, f, p, o)					v_u32_lookup(a, f, t, i, p, o)
#define v_u32_lookup_v(a, t, f)								v_u32_lookup_v_b(a, 0, t, f, 1, 0)
#define v_u16_lookup_v_b(a, i, t, f, p, o)					v_u16_lookup(a, f, t, i, p, o)
#define v_u16_lookup_v(a, i, t, f)							v_u16_lookup_v_b(a, i, t, f, 1, 0)

#if defined(__goya__)
#define v_f32_lookup_c0_v_b(a, i, t, f, p, o)				v_f32_lookup_c0(a, f, t, i, p, o)
#define v_f32_lookup_c0_v(a, t, f)							v_f32_lookup_c0_v_b(a, 0, t, f, 1, 0)
#define v_f32_lookup_c1c2_v_b(a, i, t, f, p, o)             v_f32_lookup_c1c2(a, f, t, i, p, o)
#define v_f32_lookup_c1c2_v(a, t, f)                        v_f32_lookup_c1c2_v_b(a, (float64_pair_t){0}, t, f, 1, 0)
#define v_i16_lookup_c0_v_b(a, i, t, f, p, o)				v_i16_lookup_c0(a, f, t, i, p, o)
#define v_i16_lookup_c0_v(a, i, t, f)						v_i16_lookup_c0_v_b(a, i, t, f, 1, 0)
#define v_i16_lookup_c1c2_v_b(a, i, t, f, p, o)             v_i16_lookup_c1c2(a, f, t, i, p, o)
#define v_i16_lookup_c1c2_v(a, i, t, f)						v_i16_lookup_c1c2_v_b(a, i, t, f, 1, 0)
#define v_i8_lookup_c0_v_b(a, i, t, f, p, o)				v_i8_lookup_c0(a, f, t, i, p, o)
#define v_i8_lookup_c0_v(a, i, t, f)						v_i8_lookup_c0_v_b(a, i, t, f, 1, 0)
#define v_i8_lookup_c1c2_v_b(a, i, t, f, p, o)              v_i8_lookup_c1c2(a, f, t, i, p, o)
#define v_i8_lookup_c1c2_v(a, i, t, f)						v_i8_lookup_c1c2_v_b(a, i, t, f, 1, 0)
#define v_u16_lookup_c0_v_b(a, i, t, f, p, o)				v_u16_lookup_c0(a, f, t, i, p, o)
#define v_u16_lookup_c0_v(a, i, t, f)						v_u16_lookup_c0_v_b(a, i, t, f, 1, 0)
#define v_u16_lookup_c1c2_v_b(a, i, t, f, p, o)             v_u16_lookup_c1c2(a, f, t, i, p, o)
#define v_u16_lookup_c1c2_v(a, i, t, f)						v_u16_lookup_c1c2_v_b(a, i, t, f, 1, 0)
#endif

#if defined(__gaudi__)
#define v_f32_lookup_upper_half_v_b(a, i, t, f, p, o)       v_f32_lookup(a, f, (UPPER_HALF | t), i, p, o)
#define v_f32_lookup_upper_half_v(a, t, f)                  v_f32_lookup_upper_half_v_b(a, 0, t, f, 1, 0)
#define v_f32_lookup_1c_v_b(a, i, t, f, p, o)				v_f32_lookup_1c(a, f, t, i, p, o)
#define v_f32_lookup_1c_v(a, t, f)                          v_f32_lookup_1c_v_b(a, 0, t, f, 1, 0)
#define v_f32_lookup_1c_upper_half_v_b(a, i, t, f, p, o)    v_f32_lookup_1c(a, f, UPPER_HALF | t, i, p, o)
#define v_f32_lookup_1c_upper_half_v(a, t, f)               v_f32_lookup_1c_upper_half_v_b(a, 0, t, f, 1, 0)
#define v_f32_lookup_2c_v_b(a, i, t, f, p, o)               v_f32_lookup_2c(a, f, t, i, p, o)
#define v_f32_lookup_2c_v(a, t, f)                          v_f32_lookup_2c_v_b(a, (float64_pair_t){0}, t, f, 1, 0)
#define v_f32_lookup_2c_upper_half_v_b(a, i, t, f, p, o)    v_f32_lookup_2c(a, f, UPPER_HALF | t, i, p, o)
#define v_f32_lookup_2c_upper_half_v(a, t, f)               v_f32_lookup_2c_upper_half_v_b(a, (float64_pair_t){0}, t, f, 1, 0)
#define v_bf16_lookup_1c_v_b(a, i, t, f, p, o)              v_bf16_lookup_1c(a, f, t, i, p, o)
#define v_bf16_lookup_1c_v(a, i, t, f)                      v_bf16_lookup_1c_v_b(a, 0, t, f, 1, 0)
#define v_bf16_lookup_1c_upper_half_v_b(a, i, t, f, p, o)   v_bf16_lookup_1c(a, f, UPPER_HALF | t, i, p, o)
#define v_bf16_lookup_1c_upper_half_v(a, i, t, f)           v_bf16_lookup_1c_upper_half_v_b(a, 0, t, f, 1, 0)
#define v_bf16_lookup_2c_v_b(a, i, t, f, p, o)              v_bf16_lookup_2c(a, f, t, i, p, o)
#define v_bf16_lookup_2c_v(a, i, t, f)                      v_bf16_lookup_2c_v_b(a, i, t, f, 1, 0)
#define v_bf16_lookup_2c_upper_half_v_b(a, i, t, f, p, o)   v_bf16_lookup_2c(a, f, UPPER_HALF | t, i, p, o)
#define v_bf16_lookup_2c_upper_half_v(a, i, t, f)           v_bf16_lookup_2c_upper_half_v_b(a, i, t, f, 1, 0)
#define v_i32_lookup_upper_half_v_b(a, i, t, f, p, o)       v_i32_lookup(a, f, UPPER_HALF | t, i, p, o)
#define v_i32_lookup_upper_half_v(a, t, f)                  v_i32_lookup_upper_half_v_b(a, 0, t, f, 1, 0)
#define v_i32_lookup_1c_v_b(a, i, t, f, p, o)				v_i32_lookup_1c(a, f, t, i, p, o)
#define v_i32_lookup_1c_v(a, t, f)							v_i32_lookup_1c_v_b(a, 0, t, f, 1, 0)
#define v_i32_lookup_1c_upper_half_v_b(a, i, t, f, p, o)    v_i32_lookup_1c(a, f, UPPER_HALF | t, i, p, o)
#define v_i32_lookup_1c_upper_half_v(a, t, f)               v_i32_lookup_1c_upper_half_v_b(a, 0, t, f, 1, 0)
#define v_i32_lookup_2c_v_b(a, i, t, f, p, o)				v_i32_lookup_2c(a, f, t, i, p, o)
#define v_i32_lookup_2c_v(a, t, f)                          v_i32_lookup_2c_v_b(a, (int64_pair_t){0}, t, f, 1, 0)
#define v_i32_lookup_2c_upper_half_v_b(a, i, t, f, p, o)    v_i32_lookup_2c(a, f, UPPER_HALF | t, i, p, o)
#define v_i32_lookup_2c_upper_half_v(a, t, f)               v_i32_lookup_2c_upper_half_v_b(a, (int64_pair_t){0}, t, f, 1, 0)
#define v_i16_lookup_1c_v_b(a, i, t, f, p, o)				v_i16_lookup_1c(a, f, t, i, p, o)
#define v_i16_lookup_1c_v(a, i, t, f)						v_i16_lookup_1c_v_b(a, i, t, f, 1, 0)
#define v_i16_lookup_1c_upper_half_v_b(a, i, t, f, p, o)    v_i16_lookup_1c(a, f, UPPER_HALF | t, i, p, o)
#define v_i16_lookup_1c_upper_half_v(a, i, t, f)            v_i16_lookup_1c_upper_half_v_b(a, i, t, f, 1, 0)
#define v_i16_lookup_2c_v_b(a, i, t, f, p, o)				v_i16_lookup_2c(a, f, t, i, p, o)
#define v_i16_lookup_2c_v(a, i, t, f)						v_i16_lookup_2c_v_b(a, i, t, f, 1, 0)
#define v_i16_lookup_2c_upper_half_v_b(a, i, t, f, p, o)    v_i16_lookup_2c(a, f, UPPER_HALF | t, i, p, o)
#define v_i16_lookup_2c_upper_half_v(a, i, t, f)            v_i16_lookup_2c_upper_half_v_b(a, i, t, f, 1, 0)
#define v_u32_lookup_1c_v_b(a, i, t, f, p, o)				v_u32_lookup_1c(a, f, t, i, p, o)
#define v_u32_lookup_1c_v(a, t, f)							v_u32_lookup_1c_v_b(a, 0, t, f, 1, 0)
#define v_u32_lookup_1c_upper_half_v_b(a, i, t, f, p, o)    v_u32_lookup_1c(a, f, UPPER_HALF | t, i, p, o)
#define v_u32_lookup_1c_upper_half_v(a, t, f)               v_u32_lookup_1c_upper_half_v_b(a, 0, t, f, 1, 0)
#define v_u32_lookup_2c_v_b(a, i, t, f, p, o)				v_u32_lookup_2c(a, f, t, i, p, o)
#define v_u32_lookup_2c_v(a, t, f)                          v_u32_lookup_2c_v_b(a, (uint64_pair_t){0}, t, f, 1, 0)
#define v_u32_lookup_2c_upper_half_v_b(a, i, t, f, p, o)    v_u32_lookup_2c(a, f, UPPER_HALF | t, i, p, o)
#define v_u32_lookup_2c_upper_half_v(a, t, f)               v_u32_lookup_2c_upper_half_v_b(a, (uint64_pair_t){0}, t, f, 1, 0)
#define v_u32_lookup_upper_half_v_b(a, i, t, f, p, o)       v_u32_lookup(a, f, UPPER_HALF | t, i, p, o)
#define v_u32_lookup_upper_half_v(a, t, f)                  v_u32_lookup_upper_half_v_b(a, 0, t, f, 1, 0)
#define v_u16_lookup_1c_v_b(a, i, t, f, p, o)				v_u16_lookup_1c(a, f, t, i, p, o)
#define v_u16_lookup_1c_v(a, i, t, f)						v_u16_lookup_1c_v_b(a, i, t, f, 1, 0)
#define v_u16_lookup_1c_upper_half_v_b(a, i, t, f, p, o)    v_u16_lookup_1c(a, f, UPPER_HALF | t, i, p, o)
#define v_u16_lookup_1c_upper_half_v(a, i, t, f)            v_u16_lookup_1c_upper_half_v_b(a, i, t, f, 1, 0)
#define v_u16_lookup_2c_v_b(a, i, t, f, p, o)				v_u16_lookup_2c(a, f, t, i, p, o)
#define v_u16_lookup_2c_v(a, i, t, f)						v_u16_lookup_2c_v_b(a, i, t, f, 1, 0)
#define v_u16_lookup_2c_upper_half_v_b(a, i, t, f, p, o)    v_u16_lookup_2c(a, f, UPPER_HALF | t, i, p, o)
#define v_u16_lookup_2c_upper_half_v(a, i, t, f)            v_u16_lookup_2c_upper_half_v_b(a, i, t, f, 1, 0)
#endif

// MSAC
#if defined(__goya__)
#define v_i8_msac_v_v_v_v_b(a, b, c, d, i, r, n, p, o)   v_i8_msac_b(a, b, c, d, (n << 2) | (r << 1), i, p, o)
#define v_i8_msac_v_s_v_v_b                              v_i8_msac_v_v_v_v_b
#define v_i8_msac_v_v_v_s_b                              v_i8_msac_v_v_v_v_b
#define v_i8_msac_v_v_v_v(a, b, c, d, i, r, n)           v_i8_msac_v_v_v_v_b(a, b, c, d, i, r, n, 1, 0)
#define v_i8_msac_v_s_v_v                                v_i8_msac_v_v_v_v
#define v_i8_msac_v_v_v_s                                v_i8_msac_v_v_v_v
#define v_i8_msac_v_v_v_v_vb(a, b, c, d, i, r, n, p, o)  v_i8_msac_vb(a, b, c, d, (n << 2) | (r << 1), i, p, o)
#define v_i8_msac_v_s_v_v_vb                             v_i8_msac_v_v_v_v_vb
#define v_i8_msac_v_v_v_s_vb                             v_i8_msac_v_v_v_v_vb

#define v_u8_msac_v_v_v_v_b(a, b, c, d, i, r, n, p, o)   v_u8_msac_b(a, b, c, d, (n << 2) | (r << 1), i, p, o)
#define v_u8_msac_v_s_v_v_b                              v_u8_msac_v_v_v_v_b
#define v_u8_msac_v_v_v_s_b                              v_u8_msac_v_v_v_v_b
#define v_u8_msac_v_v_v_v(a, b, c, d, i, r, n)           v_u8_msac_v_v_v_v_b(a, b, c, d, i, r, n, 1, 0)
#define v_u8_msac_v_s_v_v                                v_u8_msac_v_v_v_v
#define v_u8_msac_v_v_v_s                                v_u8_msac_v_v_v_v
#define v_u8_msac_v_v_v_v_vb(a, b, c, d, i, r, n, p, o)  v_u8_msac_vb(a, b, c, d, (n << 2) | (r << 1), i, p, o)
#define v_u8_msac_v_s_v_v_vb                             v_u8_msac_v_v_v_v_vb
#define v_u8_msac_v_v_v_s_vb                             v_u8_msac_v_v_v_v_vb

#define v_i16_msac_v_v_v_v_b(a, b, c, d, i, r, n, p, o)   v_i16_msac_b(a, b, c, d, (n << 2) | (r << 1), i, p, o)
#define v_i16_msac_v_s_v_v_b                              v_i16_msac_v_v_v_v_b
#define v_i16_msac_v_v_v_s_b                              v_i16_msac_v_v_v_v_b
#define v_i16_msac_v_v_v_v(a, b, c, d, i, r, n)           v_i16_msac_v_v_v_v_b(a, b, c, d, i, r, n, 1, 0)
#define v_i16_msac_v_s_v_v                                v_i16_msac_v_v_v_v
#define v_i16_msac_v_v_v_s                                v_i16_msac_v_v_v_v
#define v_i16_msac_v_v_v_v_vb(a, b, c, d, i, r, n, p, o)  v_i16_msac_vb(a, b, c, d, (n << 2) | (r << 1), i, to_bool128(p), o)
#define v_i16_msac_v_s_v_v_vb                             v_i16_msac_v_v_v_v_vb
#define v_i16_msac_v_v_v_s_vb                             v_i16_msac_v_v_v_v_vb

#define v_u16_msac_v_v_v_v_b(a, b, c, d, i, r, n, p, o)   v_u16_msac_b(a, b, c, d, (n << 2) | (r << 1), i, p, o)
#define v_u16_msac_v_s_v_v_b                              v_u16_msac_v_v_v_v_b
#define v_u16_msac_v_v_v_s_b                              v_u16_msac_v_v_v_v_b
#define v_u16_msac_v_v_v_v(a, b, c, d, i, r, n)           v_u16_msac_v_v_v_v_b(a, b, c, d, i, r, n, 1, 0)
#define v_u16_msac_v_s_v_v                                v_u16_msac_v_v_v_v
#define v_u16_msac_v_v_v_s                                v_u16_msac_v_v_v_v
#define v_u16_msac_v_v_v_v_vb(a, b, c, d, i, r, n, p, o)  v_u16_msac_vb(a, b, c, d, (n << 2) | (r << 1), i, to_bool128(p), o)
#define v_u16_msac_v_s_v_v_vb                             v_u16_msac_v_v_v_v_vb
#define v_u16_msac_v_v_v_s_vb                             v_u16_msac_v_v_v_v_vb
#endif


// MOV_GROUP
#define v_f32_mov_group_v_vb(a, b, i, g, d, p, o)   v_f32_mov_group_vb(a, b, (d << 2) | g, i, to_bool64(p), o)
#define v_bf16_mov_group_v_vb(a, b, i, g, d, p, o)  v_bf16_mov_group_vb(a, b, (d << 2) | g, i, to_bool128(p), o)
#define v_i32_mov_group_v_vb(a, b, i, g, d, p, o)   v_i32_mov_group_vb(a, b, (d << 2) | g, i, to_bool64(p), o)
#define v_u32_mov_group_v_vb(a, b, i, g, d, p, o)   v_u32_mov_group_vb(a, b, (d << 2) | g, i, to_bool64(p), o)
#define v_i16_mov_group_v_vb(a, b, i, g, d, p, o)   v_i16_mov_group_vb(a, b, (d << 2) | g, i, to_bool128(p), o)
#define v_u16_mov_group_v_vb(a, b, i, g, d, p, o)   v_u16_mov_group_vb(a, b, (d << 2) | g, i, to_bool128(p), o)
#define v_i8_mov_group_v_vb(a, b, i, g, d, p, o)    v_i8_mov_group_vb(a, b, (d << 2) | g, i, p, o)
#define v_u8_mov_group_v_vb(a, b, i, g, d, p, o)    v_u8_mov_group_vb(a, b, (d << 2) | g, i, p, o)

#define v_f32_mov_group_v_b(a, b, i, g, d, p, o)    v_f32_mov_group_b(a, b, (d << 2) | g, i, p, o)
#define v_bf16_mov_group_v_b(a, b, i, g, d, p, o)   v_bf16_mov_group_b(a, b, (d << 2) | g, i, p, o)
#define v_i32_mov_group_v_b(a, b, i, g, d, p, o)    v_i32_mov_group_b(a, b, (d << 2) | g, i, p, o)
#define v_u32_mov_group_v_b(a, b, i, g, d, p, o)    v_u32_mov_group_b(a, b, (d << 2) | g, i, p, o)
#define v_i16_mov_group_v_b(a, b, i, g, d, p, o)    v_i16_mov_group_b(a, b, (d << 2) | g, i, p, o)
#define v_u16_mov_group_v_b(a, b, i, g, d, p, o)    v_u16_mov_group_b(a, b, (d << 2) | g, i, p, o)
#define v_i8_mov_group_v_b(a, b, i, g, d, p, o)     v_i8_mov_group_b(a, b, (d << 2) | g, i, p, o)
#define v_u8_mov_group_v_b(a, b, i, g, d, p, o)     v_u8_mov_group_b(a, b, (d << 2) | g, i, p, o)

#define v_f32_mov_group_v(a, b, i, g, d)            v_f32_mov_group_v_b(a, b, i, g, d, 1, 0)
#define v_bf16_mov_group_v(a, b, i, g, d)           v_bf16_mov_group_v_b(a, b, i, g, d, 1, 0)
#define v_i32_mov_group_v(a, b, i, g, d)            v_i32_mov_group_v_b(a, b, i, g, d, 1, 0)
#define v_u32_mov_group_v(a, b, i, g, d)            v_u32_mov_group_v_b(a, b, i, g, d, 1, 0)
#define v_i16_mov_group_v(a, b, i, g, d)            v_i16_mov_group_v_b(a, b, i, g, d, 1, 0)
#define v_u16_mov_group_v(a, b, i, g, d)            v_u16_mov_group_v_b(a, b, i, g, d, 1, 0)
#define v_i8_mov_group_v(a, b, i, g, d)             v_i8_mov_group_v_b(a, b, i, g, d, 1, 0)
#define v_u8_mov_group_v(a, b, i, g, d)             v_u8_mov_group_v_b(a, b, i, g, d, 1, 0)

// MOV_DUAL_GROUP
#define MkWr(l, u)  (((l) ? SW_WR_LOWER_GROUP : 0) | ((u) ? SW_WR_UPPER_GROUP : 0))
#define v_f32_mov_dual_group_v_vb(a, b, i, s, d, l, u, p, o)    v_f32_mov_dual_group_vb(a, b, s, d, MkWr(l, u), i, to_bool64(p), o)
#define v_f32_mov_dual_group_v_b(a, b, i, s, d, l, u, p, o)     v_f32_mov_dual_group_b(a, b, s, d, MkWr(l, u), i, p, o)
#define v_bf16_mov_dual_group_v_vb(a, b, i, s, d, l, u, p, o)   v_bf16_mov_dual_group_vb(a, b, s, d, MkWr(l, u), i, to_bool128(p), o)
#define v_bf16_mov_dual_group_v_b(a, b, i, s, d, l, u, p, o)    v_bf16_mov_dual_group_b(a, b, s, d, MkWr(l, u), i, p, o)
#define v_i32_mov_dual_group_v_vb(a, b, i, s, d, l, u, p, o)    v_i32_mov_dual_group_vb(a, b, s, d, MkWr(l, u), i, to_bool64(p), o)
#define v_i32_mov_dual_group_v_b(a, b, i, s, d, l, u, p, o)     v_i32_mov_dual_group_b(a, b, s, d, MkWr(l, u), i, p, o)
#define v_u32_mov_dual_group_v_vb(a, b, i, s, d, l, u, p, o)    v_u32_mov_dual_group_vb(a, b, s, d, MkWr(l, u), i, to_bool64(p), o)
#define v_u32_mov_dual_group_v_b(a, b, i, s, d, l, u, p, o)     v_u32_mov_dual_group_b(a, b, s, d, MkWr(l, u), i, p, o)
#define v_i16_mov_dual_group_v_vb(a, b, i, s, d, l, u, p, o)    v_i16_mov_dual_group_vb(a, b, s, d, MkWr(l, u), i, to_bool128(p), o)
#define v_i16_mov_dual_group_v_b(a, b, i, s, d, l, u, p, o)     v_i16_mov_dual_group_b(a, b, s, d, MkWr(l, u), i, p, o)
#define v_u16_mov_dual_group_v_vb(a, b, i, s, d, l, u, p, o)    v_u16_mov_dual_group_vb(a, b, s, d, MkWr(l, u), i, to_bool128(p), o)
#define v_u16_mov_dual_group_v_b(a, b, i, s, d, l, u, p, o)     v_u16_mov_dual_group_b(a, b, s, d, MkWr(l, u), i, p, o)
#define v_i8_mov_dual_group_v_vb(a, b, i, s, d, l, u, p, o)     v_i8_mov_dual_group_vb(a, b, s, d, MkWr(l, u), i, p, o)
#define v_i8_mov_dual_group_v_b(a, b, i, s, d, l, u, p, o)      v_i8_mov_dual_group_b(a, b, s, d, MkWr(l, u), i, p, o)
#define v_u8_mov_dual_group_v_vb(a, b, i, s, d, l, u, p, o)     v_u8_mov_dual_group_vb(a, b, s, d, MkWr(l, u), i, p, o)
#define v_u8_mov_dual_group_v_b(a, b, i, s, d, l, u, p, o)      v_u8_mov_dual_group_b(a, b, s, d, MkWr(l, u), i, p, o)

#define v_f32_mov_dual_group_v(a, b, i, s, d, l, u)             v_f32_mov_dual_group_v_b(a, b, i, s, d, l, u, 1, 0)
#define v_bf16_mov_dual_group_v(a, b, i, s, d, l, u)            v_bf16_mov_dual_group_v_b(a, b, i, s, d, l, u, 1, 0)
#define v_i32_mov_dual_group_v(a, b, i, s, d, l, u)             v_i32_mov_dual_group_v_b(a, b, i, s, d, l, u, 1, 0)
#define v_u32_mov_dual_group_v(a, b, i, s, d, l, u)             v_u32_mov_dual_group_v_b(a, b, i, s, d, l, u, 1, 0)
#define v_i16_mov_dual_group_v(a, b, i, s, d, l, u)             v_i16_mov_dual_group_v_b(a, b, i, s, d, l, u, 1, 0)
#define v_u16_mov_dual_group_v(a, b, i, s, d, l, u)             v_u16_mov_dual_group_v_b(a, b, i, s, d, l, u, 1, 0)
#define v_i8_mov_dual_group_v(a, b, i, s, d, l, u)              v_i8_mov_dual_group_v_b(a, b, i, s, d, l, u, 1, 0)
#define v_u8_mov_dual_group_v(a, b, i, s, d, l, u)              v_u8_mov_dual_group_v_b(a, b, i, s, d, l, u, 1, 0)

#define MkWrA(w0, w1, w2, w3)  (((w0) << 16) | ((w1) << 18) | ((w2) << 20) | ((w3) << 22))
#define v_f32_mov_dual_group_all_v_vb(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)    v_f32_mov_dual_group_all_vb(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, to_bool64(p), o)
#define v_f32_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)     v_f32_mov_dual_group_all_b(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, p, o)
#define v_bf16_mov_dual_group_all_v_vb(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)   v_bf16_mov_dual_group_all_vb(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, to_bool128(p), o)
#define v_bf16_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)    v_bf16_mov_dual_group_all_b(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, p, o)
#define v_i32_mov_dual_group_all_v_vb(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)    v_i32_mov_dual_group_all_vb(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, to_bool64(p), o)
#define v_i32_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)     v_i32_mov_dual_group_all_b(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, p, o)
#define v_u32_mov_dual_group_all_v_vb(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)    v_u32_mov_dual_group_all_vb(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, to_bool64(p), o)
#define v_u32_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)     v_u32_mov_dual_group_all_b(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, p, o)
#define v_i16_mov_dual_group_all_v_vb(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)    v_i16_mov_dual_group_all_vb(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, to_bool128(p), o)
#define v_i16_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)     v_i16_mov_dual_group_all_b(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, p, o)
#define v_u16_mov_dual_group_all_v_vb(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)    v_u16_mov_dual_group_all_vb(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, to_bool128(p), o)
#define v_u16_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)     v_u16_mov_dual_group_all_b(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, p, o)
#define v_i8_mov_dual_group_all_v_vb(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)     v_i8_mov_dual_group_all_vb(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, p, o)
#define v_i8_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)      v_i8_mov_dual_group_all_b(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, p, o)
#define v_u8_mov_dual_group_all_v_vb(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)     v_u8_mov_dual_group_all_vb(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, p, o)
#define v_u8_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, p, o)      v_u8_mov_dual_group_all_b(a, b, s0, s1, s2, s3, MkWrA(w0, w1, w2, w3), i, p, o)

#define v_f32_mov_dual_group_all_v(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3)             v_f32_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, 1, 0)
#define v_bf16_mov_dual_group_all_v(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3)            v_bf16_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, 1, 0)
#define v_i32_mov_dual_group_all_v(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3)             v_i32_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, 1, 0)
#define v_u32_mov_dual_group_all_v(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3)             v_u32_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, 1, 0)
#define v_i16_mov_dual_group_all_v(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3)             v_i16_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, 1, 0)
#define v_u16_mov_dual_group_all_v(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3)             v_u16_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, 1, 0)
#define v_i8_mov_dual_group_all_v(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3)              v_i8_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, 1, 0)
#define v_u8_mov_dual_group_all_v(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3)              v_u8_mov_dual_group_all_v_b(a, b, i, s0, s1, s2, s3, w0, w1, w2, w3, 1, 0)


// ST_TNSR

#define f32_st_tnsr_i_v_b(n, t, v, p, o)    v_f32_st_tnsr(n, t, v, 0, p, o)
#define bf16_st_tnsr_i_v_b(n, t, v, p, o)   v_bf16_st_tnsr(n, t, v, 0, p, o)
#define i32_st_tnsr_i_v_b(n, t, v, p, o)    v_i32_st_tnsr(n, t, v, 0, p, o)
#define u32_st_tnsr_i_v_b(n, t, v, p, o)    v_u32_st_tnsr(n, t, v, 0, p, o)
#define i16_st_tnsr_i_v_b(n, t, v, p, o)    v_i16_st_tnsr(n, t, v, 0, p, o)
#define u16_st_tnsr_i_v_b(n, t, v, p, o)    v_u16_st_tnsr(n, t, v, 0, p, o)
#define i8_st_tnsr_i_v_b(n, t, v, p, o)     v_i8_st_tnsr(n, t, v, 0, p, o)
#define u8_st_tnsr_i_v_b(n, t, v, p, o)     v_u8_st_tnsr(n, t, v, 0, p, o)
#define st_tnsr_i_bv_b(n, t, v, p, o)       v_i1_st_tnsr(n, t, v, 0, p, o)

#define f32_st_tnsr_i_v(n, t, v)            f32_st_tnsr_i_v_b(n, t, v, 1, 0)
#define bf16_st_tnsr_i_v(n, t, v)           bf16_st_tnsr_i_v_b(n, t, v, 1, 0)
#define f16_st_tnsr_i_v(n, t, v)            f16_st_tnsr_i_v_b(n, t, v, 1, 0)
#define i32_st_tnsr_i_v(n, t, v)            i32_st_tnsr_i_v_b(n, t, v, 1, 0)
#define u32_st_tnsr_i_v(n, t, v)            u32_st_tnsr_i_v_b(n, t, v, 1, 0)
#define i16_st_tnsr_i_v(n, t, v)            i16_st_tnsr_i_v_b(n, t, v, 1, 0)
#define u16_st_tnsr_i_v(n, t, v)            u16_st_tnsr_i_v_b(n, t, v, 1, 0)
#define i8_st_tnsr_i_v(n, t, v)             i8_st_tnsr_i_v_b(n, t, v, 1, 0)
#define u8_st_tnsr_i_v(n, t, v)             u8_st_tnsr_i_v_b(n, t, v, 1, 0)
#define st_tnsr_i_bv(n, t, v)               st_tnsr_i_bv_b(n, t, v, 1, 0)

#define f32_st_tnsr_pack_i_v_b(n, t, v, p, o)     v_f32_st_tnsr(n, t, v, SW_PACK, p, o)
#define bf16_st_tnsr_pack_i_v_b(n, t, v, p, o)    v_bf16_st_tnsr(n, t, v, SW_PACK, p, o)
#define i32_st_tnsr_pack_i_v_b(n, t, v, p, o)     v_i32_st_tnsr(n, t, v, SW_PACK, p, o)
#define u32_st_tnsr_pack_i_v_b(n, t, v, p, o)     v_u32_st_tnsr(n, t, v, SW_PACK, p, o)
#define i16_st_tnsr_pack_i_v_b(n, t, v, p, o)     v_i16_st_tnsr(n, t, v, SW_PACK, p, o)
#define u16_st_tnsr_pack_i_v_b(n, t, v, p, o)     v_u16_st_tnsr(n, t, v, SW_PACK, p, o)
#define i8_st_tnsr_pack_i_v_b(n, t, v, p, o)      v_i8_st_tnsr(n, t, v, SW_PACK, p, o)
#define u8_st_tnsr_pack_i_v_b(n, t, v, p, o)      v_u8_st_tnsr(n, t, v, SW_PACK, p, o)

#define f32_st_tnsr_pack_i_v(n, t, v)             f32_st_tnsr_pack_i_v_b(n, t, v, 1, 0)
#define bf16_st_tnsr_pack_i_v(n, t, v)            bf16_st_tnsr_pack_i_v_b(n, t, v, 1, 0)
#define f16_st_tnsr_pack_i_v(n, t, v)             f16_st_tnsr_pack_i_v_b(n, t, v, 1, 0)
#define i32_st_tnsr_pack_i_v(n, t, v)             i32_st_tnsr_pack_i_v_b(n, t, v, 1, 0)
#define u32_st_tnsr_pack_i_v(n, t, v)             u32_st_tnsr_pack_i_v_b(n, t, v, 1, 0)
#define i16_st_tnsr_pack_i_v(n, t, v)             i16_st_tnsr_pack_i_v_b(n, t, v, 1, 0)
#define u16_st_tnsr_pack_i_v(n, t, v)             u16_st_tnsr_pack_i_v_b(n, t, v, 1, 0)
#define i8_st_tnsr_pack_i_v(n, t, v)              i8_st_tnsr_pack_i_v_b(n, t, v, 1, 0)
#define u8_st_tnsr_pack_i_v(n, t, v)              u8_st_tnsr_pack_i_v_b(n, t, v, 1, 0)

#define f32_st_tnsr_reg_i_s_v_b         f32_st_tnsr_i_v_b
#define bf16_st_tnsr_reg_i_s_v_b        bf16_st_tnsr_i_v_b
#define f16_st_tnsr_reg_i_s_v_b         f16_st_tnsr_i_v_b
#define i32_st_tnsr_reg_i_s_v_b         i32_st_tnsr_i_v_b
#define u32_st_tnsr_reg_i_s_v_b         u32_st_tnsr_i_v_b
#define i16_st_tnsr_reg_i_s_v_b         i16_st_tnsr_i_v_b
#define u16_st_tnsr_reg_i_s_v_b         u16_st_tnsr_i_v_b
#define i8_st_tnsr_reg_i_s_v_b          i8_st_tnsr_i_v_b
#define u8_st_tnsr_reg_i_s_v_b          u8_st_tnsr_i_v_b
#define st_tnsr_reg_i_s_bv_b            st_tnsr_i_bv_b

#define f32_st_tnsr_reg_i_s_v           f32_st_tnsr_i_v
#define bf16_st_tnsr_reg_i_s_v          bf16_st_tnsr_i_v
#define f16_st_tnsr_reg_i_s_v           f16_st_tnsr_i_v
#define i32_st_tnsr_reg_i_s_v           i32_st_tnsr_i_v
#define u32_st_tnsr_reg_i_s_v           u32_st_tnsr_i_v
#define i16_st_tnsr_reg_i_s_v           i16_st_tnsr_i_v
#define u16_st_tnsr_reg_i_s_v           u16_st_tnsr_i_v
#define i8_st_tnsr_reg_i_s_v            i8_st_tnsr_i_v
#define u8_st_tnsr_reg_i_s_v            u8_st_tnsr_i_v
#define st_tnsr_reg_i_s_bv              st_tnsr_i_bv

#define f32_st_tnsr_reg_pack_i_s_v_b    f32_st_tnsr_pack_i_v_b
#define bf16_st_tnsr_reg_pack_i_s_v_b   bf16_st_tnsr_pack_i_v_b
#define f16_st_tnsr_reg_pack_i_s_v_b    f16_st_tnsr_pack_i_v_b
#define i32_st_tnsr_reg_pack_i_s_v_b    i32_st_tnsr_pack_i_v_b
#define u32_st_tnsr_reg_pack_i_s_v_b    u32_st_tnsr_pack_i_v_b
#define i16_st_tnsr_reg_pack_i_s_v_b    i16_st_tnsr_pack_i_v_b
#define u16_st_tnsr_reg_pack_i_s_v_b    u16_st_tnsr_pack_i_v_b
#define i8_st_tnsr_reg_pack_i_s_v_b     i8_st_tnsr_pack_i_v_b
#define u8_st_tnsr_reg_pack_i_s_v_b     u8_st_tnsr_pack_i_v_b

#define f32_st_tnsr_reg_pack_i_s_v      f32_st_tnsr_pack_i_v
#define bf16_st_tnsr_reg_pack_i_s_v     bf16_st_tnsr_pack_i_v
#define f16_st_tnsr_reg_pack_i_s_v      f16_st_tnsr_pack_i_v
#define i32_st_tnsr_reg_pack_i_s_v      i32_st_tnsr_pack_i_v
#define u32_st_tnsr_reg_pack_i_s_v      u32_st_tnsr_pack_i_v
#define i16_st_tnsr_reg_pack_i_s_v      i16_st_tnsr_pack_i_v
#define u16_st_tnsr_reg_pack_i_s_v      u16_st_tnsr_pack_i_v
#define i8_st_tnsr_reg_pack_i_s_v       i8_st_tnsr_pack_i_v
#define u8_st_tnsr_reg_pack_i_s_v       u8_st_tnsr_pack_i_v

#define MkRMW(dt, op, rmw, dl)  ((dt) | ((op) << 4) | ((rmw) << 6) | ((dl) << 7))

#define f32_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_f32_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define bf16_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)  v_bf16_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define i32_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_i32_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define u32_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_u32_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define i16_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_i16_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define u16_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_u16_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define i8_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)    v_i8_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define u8_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)    v_u8_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)

#define f32_st_tnsr_rmw_i_v(n, t, v, dt, op, rmw, dl)   f32_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define bf16_st_tnsr_rmw_i_v(n, t, v, dt, op, rmw, dl)  bf16_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define f16_st_tnsr_rmw_i_v(n, t, v, dt, op, rmw, dl)   f16_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i32_st_tnsr_rmw_i_v(n, t, v, dt, op, rmw, dl)   i32_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u32_st_tnsr_rmw_i_v(n, t, v, dt, op, rmw, dl)   u32_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i16_st_tnsr_rmw_i_v(n, t, v, dt, op, rmw, dl)   i16_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u16_st_tnsr_rmw_i_v(n, t, v, dt, op, rmw, dl)   u16_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i8_st_tnsr_rmw_i_v(n, t, v, dt, op, rmw, dl)    i8_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u8_st_tnsr_rmw_i_v(n, t, v, dt, op, rmw, dl)    u8_st_tnsr_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)

#define f32_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_f32_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define bf16_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)  v_bf16_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define i32_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_i32_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define u32_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_u32_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define i16_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_i16_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define u16_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_u16_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define i8_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)    v_i8_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define u8_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)    v_u8_st_tnsr_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)

#define f32_st_tnsr_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    f32_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define bf16_st_tnsr_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)   bf16_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define f16_st_tnsr_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    f16_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i32_st_tnsr_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    i32_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u32_st_tnsr_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    u32_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i16_st_tnsr_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    i16_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u16_st_tnsr_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    u16_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i8_st_tnsr_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)     i8_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u8_st_tnsr_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)     u8_st_tnsr_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)

#define f32_st_tnsr_reg_rmw_i_s_v_b       f32_st_tnsr_rmw_i_v_b
#define bf16_st_tnsr_reg_rmw_i_s_v_b      bf16_st_tnsr_rmw_i_v_b
#define f16_st_tnsr_reg_rmw_i_s_v_b       f16_st_tnsr_rmw_i_v_b
#define i32_st_tnsr_reg_rmw_i_s_v_b       i32_st_tnsr_rmw_i_v_b
#define u32_st_tnsr_reg_rmw_i_s_v_b       u32_st_tnsr_rmw_i_v_b
#define i16_st_tnsr_reg_rmw_i_s_v_b       i16_st_tnsr_rmw_i_v_b
#define u16_st_tnsr_reg_rmw_i_s_v_b       u16_st_tnsr_rmw_i_v_b
#define i8_st_tnsr_reg_rmw_i_s_v_b        i8_st_tnsr_rmw_i_v_b
#define u8_st_tnsr_reg_rmw_i_s_v_b        u8_st_tnsr_rmw_i_v_b

#define f32_st_tnsr_reg_rmw_i_s_v         f32_st_tnsr_rmw_i_v
#define bf16_st_tnsr_reg_rmw_i_s_v        bf16_st_tnsr_rmw_i_v
#define f16_st_tnsr_reg_rmw_i_s_v         f16_st_tnsr_rmw_i_v
#define i32_st_tnsr_reg_rmw_i_s_v         i32_st_tnsr_rmw_i_v
#define u32_st_tnsr_reg_rmw_i_s_v         u32_st_tnsr_rmw_i_v
#define i16_st_tnsr_reg_rmw_i_s_v         i16_st_tnsr_rmw_i_v
#define u16_st_tnsr_reg_rmw_i_s_v         u16_st_tnsr_rmw_i_v
#define i8_st_tnsr_reg_rmw_i_s_v          i8_st_tnsr_rmw_i_v
#define u8_st_tnsr_reg_rmw_i_s_v          u8_st_tnsr_rmw_i_v

#define f32_st_tnsr_reg_rmw_pack_i_s_v_b    f32_st_tnsr_rmw_pack_i_v_b
#define bf16_st_tnsr_reg_rmw_pack_i_s_v_b   bf16_st_tnsr_rmw_pack_i_v_b
#define f16_st_tnsr_reg_rmw_pack_i_s_v_b    f16_st_tnsr_rmw_pack_i_v_b
#define i32_st_tnsr_reg_rmw_pack_i_s_v_b    i32_st_tnsr_rmw_pack_i_v_b
#define u32_st_tnsr_reg_rmw_pack_i_s_v_b    u32_st_tnsr_rmw_pack_i_v_b
#define i16_st_tnsr_reg_rmw_pack_i_s_v_b    i16_st_tnsr_rmw_pack_i_v_b
#define u16_st_tnsr_reg_rmw_pack_i_s_v_b    u16_st_tnsr_rmw_pack_i_v_b
#define i8_st_tnsr_reg_rmw_pack_i_s_v_b     i8_st_tnsr_rmw_pack_i_v_b
#define u8_st_tnsr_reg_rmw_pack_i_s_v_b     u8_st_tnsr_rmw_pack_i_v_b

#define f32_st_tnsr_reg_rmw_pack_i_s_v      f32_st_tnsr_rmw_pack_i_v
#define bf16_st_tnsr_reg_rmw_pack_i_s_v     bf16_st_tnsr_rmw_pack_i_v
#define f16_st_tnsr_reg_rmw_pack_i_s_v      f16_st_tnsr_rmw_pack_i_v
#define i32_st_tnsr_reg_rmw_pack_i_s_v      i32_st_tnsr_rmw_pack_i_v
#define u32_st_tnsr_reg_rmw_pack_i_s_v      u32_st_tnsr_rmw_pack_i_v
#define i16_st_tnsr_reg_rmw_pack_i_s_v      i16_st_tnsr_rmw_pack_i_v
#define u16_st_tnsr_reg_rmw_pack_i_s_v      u16_st_tnsr_rmw_pack_i_v
#define i8_st_tnsr_reg_rmw_pack_i_s_v       i8_st_tnsr_rmw_pack_i_v
#define u8_st_tnsr_reg_rmw_pack_i_s_v       u8_st_tnsr_rmw_pack_i_v

#define f32_st_tnsr_partial_i_v_b(n, t, v, s, f, p, o)        v_f32_st_tnsr_partial(n, t, v, s, f, 0, p, o)
#define bf16_st_tnsr_partial_i_v_b(n, t, v, s, f, p, o)       v_bf16_st_tnsr_partial(n, t, v, s, f, 0, p, o)
#define i32_st_tnsr_partial_i_v_b(n, t, v, s, f, p, o)        v_i32_st_tnsr_partial(n, t, v, s, f, 0, p, o)
#define u32_st_tnsr_partial_i_v_b(n, t, v, s, f, p, o)        v_u32_st_tnsr_partial(n, t, v, s, f, 0, p, o)
#define i16_st_tnsr_partial_i_v_b(n, t, v, s, f, p, o)        v_i16_st_tnsr_partial(n, t, v, s, f, 0, p, o)
#define u16_st_tnsr_partial_i_v_b(n, t, v, s, f, p, o)        v_u16_st_tnsr_partial(n, t, v, s, f, 0, p, o)
#define i8_st_tnsr_partial_i_v_b(n, t, v, s, f, p, o)         v_i8_st_tnsr_partial(n, t, v, s, f, 0, p, o)
#define u8_st_tnsr_partial_i_v_b(n, t, v, s, f, p, o)         v_u8_st_tnsr_partial(n, t, v, s, f, 0, p, o)

#define f32_st_tnsr_partial_i_v(n, t, v, s, f)                f32_st_tnsr_partial_i_v_b(n, t, v,s, f, 1, 0)
#define bf16_st_tnsr_partial_i_v(n, t, v, s, f)               bf16_st_tnsr_partial_i_v_b(n, t, v,s, f, 1, 0)
#define f16_st_tnsr_partial_i_v(n, t, v, s, f)                f16_st_tnsr_partial_i_v_b(n, t, v,s, f, 1, 0)
#define i32_st_tnsr_partial_i_v(n, t, v, s, f)                i32_st_tnsr_partial_i_v_b(n, t, v,s, f, 1, 0)
#define u32_st_tnsr_partial_i_v(n, t, v, s, f)                u32_st_tnsr_partial_i_v_b(n, t, v,s, f, 1, 0)
#define i16_st_tnsr_partial_i_v(n, t, v, s, f)                i16_st_tnsr_partial_i_v_b(n, t, v,s, f, 1, 0)
#define u16_st_tnsr_partial_i_v(n, t, v, s, f)                u16_st_tnsr_partial_i_v_b(n, t, v,s, f, 1, 0)
#define i8_st_tnsr_partial_i_v(n, t, v, s, f)                 i8_st_tnsr_partial_i_v_b(n, t, v,s, f, 1, 0)
#define u8_st_tnsr_partial_i_v(n, t, v, s, f)                 u8_st_tnsr_partial_i_v_b(n, t, v,s, f, 1, 0)

#define f32_st_tnsr_partial_pack_i_v_b(n, t, v, s, f, p, o)   v_f32_st_tnsr_partial(n, t, v, s, f, SW_PACK, p, o)
#define bf16_st_tnsr_partial_pack_i_v_b(n, t, v, s, f, p, o)  v_bf16_st_tnsr_partial(n, t, v, s, f, SW_PACK, p, o)
#define i32_st_tnsr_partial_pack_i_v_b(n, t, v, s, f, p, o)   v_i32_st_tnsr_partial(n, t, v, s, f, SW_PACK, p, o)
#define u32_st_tnsr_partial_pack_i_v_b(n, t, v, s, f, p, o)   v_u32_st_tnsr_partial(n, t, v, s, f, SW_PACK, p, o)
#define i16_st_tnsr_partial_pack_i_v_b(n, t, v, s, f, p, o)   v_i16_st_tnsr_partial(n, t, v, s, f, SW_PACK, p, o)
#define u16_st_tnsr_partial_pack_i_v_b(n, t, v, s, f, p, o)   v_u16_st_tnsr_partial(n, t, v, s, f, SW_PACK, p, o)
#define i8_st_tnsr_partial_pack_i_v_b(n, t, v, s, f, p, o)    v_i8_st_tnsr_partial(n, t, v, s, f, SW_PACK, p, o)
#define u8_st_tnsr_partial_pack_i_v_b(n, t, v, s, f, p, o)    v_u8_st_tnsr_partial(n, t, v, s, f, SW_PACK, p, o)

#define f32_st_tnsr_partial_pack_i_v(n, t, v, s, f)           f32_st_tnsr_partial_pack_i_v_b(n, t, v,s, f, 1, 0)
#define bf16_st_tnsr_partial_pack_i_v(n, t, v, s, f)          bf16_st_tnsr_partial_pack_i_v_b(n, t, v,s, f, 1, 0)
#define f16_st_tnsr_partial_pack_i_v(n, t, v, s, f)           f16_st_tnsr_partial_pack_i_v_b(n, t, v,s, f, 1, 0)
#define i32_st_tnsr_partial_pack_i_v(n, t, v, s, f)           i32_st_tnsr_partial_pack_i_v_b(n, t, v,s, f, 1, 0)
#define u32_st_tnsr_partial_pack_i_v(n, t, v, s, f)           u32_st_tnsr_partial_pack_i_v_b(n, t, v,s, f, 1, 0)
#define i16_st_tnsr_partial_pack_i_v(n, t, v, s, f)           i16_st_tnsr_partial_pack_i_v_b(n, t, v,s, f, 1, 0)
#define u16_st_tnsr_partial_pack_i_v(n, t, v, s, f)           u16_st_tnsr_partial_pack_i_v_b(n, t, v,s, f, 1, 0)
#define i8_st_tnsr_partial_pack_i_v(n, t, v, s, f)            i8_st_tnsr_partial_pack_i_v_b(n, t, v,s, f, 1, 0)
#define u8_st_tnsr_partial_pack_i_v(n, t, v, s, f)            u8_st_tnsr_partial_pack_i_v_b(n, t, v,s, f, 1, 0)

#define f32_st_tnsr_partial_reg_i_s_v_b     f32_st_tnsr_partial_i_v_b
#define bf16_st_tnsr_partial_reg_i_s_v_b    bf16_st_tnsr_partial_i_v_b
#define f16_st_tnsr_partial_reg_i_s_v_b     f16_st_tnsr_partial_i_v_b
#define i32_st_tnsr_partial_reg_i_s_v_b     i32_st_tnsr_partial_i_v_b
#define u32_st_tnsr_partial_reg_i_s_v_b     u32_st_tnsr_partial_i_v_b
#define i16_st_tnsr_partial_reg_i_s_v_b     i16_st_tnsr_partial_i_v_b
#define u16_st_tnsr_partial_reg_i_s_v_b     u16_st_tnsr_partial_i_v_b
#define i8_st_tnsr_partial_reg_i_s_v_b      i8_st_tnsr_partial_i_v_b
#define u8_st_tnsr_partial_reg_i_s_v_b      u8_st_tnsr_partial_i_v_b

#define f32_st_tnsr_partial_reg_i_s_v       f32_st_tnsr_partial_i_v
#define bf16_st_tnsr_partial_reg_i_s_v      bf16_st_tnsr_partial_i_v
#define f16_st_tnsr_partial_reg_i_s_v       f16_st_tnsr_partial_i_v
#define i32_st_tnsr_partial_reg_i_s_v       i32_st_tnsr_partial_i_v
#define u32_st_tnsr_partial_reg_i_s_v       u32_st_tnsr_partial_i_v
#define i16_st_tnsr_partial_reg_i_s_v       i16_st_tnsr_partial_i_v
#define u16_st_tnsr_partial_reg_i_s_v       u16_st_tnsr_partial_i_v
#define i8_st_tnsr_partial_reg_i_s_v        i8_st_tnsr_partial_i_v
#define u8_st_tnsr_partial_reg_i_s_v        u8_st_tnsr_partial_i_v

#define f32_st_tnsr_partial_reg_pack_i_s_v_b      f32_st_tnsr_partial_pack_i_v_b
#define bf16_st_tnsr_partial_reg_pack_i_s_v_b     bf16_st_tnsr_partial_pack_i_v_b
#define f16_st_tnsr_partial_reg_pack_i_s_v_b      f16_st_tnsr_partial_pack_i_v_b
#define i32_st_tnsr_partial_reg_pack_i_s_v_b      i32_st_tnsr_partial_pack_i_v_b
#define u32_st_tnsr_partial_reg_pack_i_s_v_b      u32_st_tnsr_partial_pack_i_v_b
#define i16_st_tnsr_partial_reg_pack_i_s_v_b      i16_st_tnsr_partial_pack_i_v_b
#define u16_st_tnsr_partial_reg_pack_i_s_v_b      u16_st_tnsr_partial_pack_i_v_b
#define i8_st_tnsr_partial_reg_pack_i_s_v_b       i8_st_tnsr_partial_pack_i_v_b
#define u8_st_tnsr_partial_reg_pack_i_s_v_b       u8_st_tnsr_partial_pack_i_v_b

#define f32_st_tnsr_partial_reg_pack_i_s_v        f32_st_tnsr_partial_pack_i_v
#define bf16_st_tnsr_partial_reg_pack_i_s_v       bf16_st_tnsr_partial_pack_i_v
#define f16_st_tnsr_partial_reg_pack_i_s_v        f16_st_tnsr_partial_pack_i_v
#define i32_st_tnsr_partial_reg_pack_i_s_v        i32_st_tnsr_partial_pack_i_v
#define u32_st_tnsr_partial_reg_pack_i_s_v        u32_st_tnsr_partial_pack_i_v
#define i16_st_tnsr_partial_reg_pack_i_s_v        i16_st_tnsr_partial_pack_i_v
#define u16_st_tnsr_partial_reg_pack_i_s_v        u16_st_tnsr_partial_pack_i_v
#define i8_st_tnsr_partial_reg_pack_i_s_v         i8_st_tnsr_partial_pack_i_v
#define u8_st_tnsr_partial_reg_pack_i_s_v         u8_st_tnsr_partial_pack_i_v

#define f32_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)   v_f32_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, 0, p, o)
#define bf16_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)  v_bf16_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, 0, p, o)
#define i32_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)   v_i32_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, 0, p, o)
#define u32_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)   v_u32_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, 0, p, o)
#define i16_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)   v_i16_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, 0, p, o)
#define u16_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)   v_u16_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, 0, p, o)
#define i8_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)    v_i8_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, 0, p, o)
#define u8_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)    v_u8_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, 0, p, o)

#define f32_st_tnsr_partial_rmw_i_v(n, t, v, dt, op, rmw, dl, s, f)   f32_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define bf16_st_tnsr_partial_rmw_i_v(n, t, v, dt, op, rmw, dl, s, f)  bf16_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define f16_st_tnsr_partial_rmw_i_v(n, t, v, dt, op, rmw, dl, s, f)   f16_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define i32_st_tnsr_partial_rmw_i_v(n, t, v, dt, op, rmw, dl, s, f)   i32_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define u32_st_tnsr_partial_rmw_i_v(n, t, v, dt, op, rmw, dl, s, f)   u32_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define i16_st_tnsr_partial_rmw_i_v(n, t, v, dt, op, rmw, dl, s, f)   i16_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define u16_st_tnsr_partial_rmw_i_v(n, t, v, dt, op, rmw, dl, s, f)   u16_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define i8_st_tnsr_partial_rmw_i_v(n, t, v, dt, op, rmw, dl, s, f)    i8_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define u8_st_tnsr_partial_rmw_i_v(n, t, v, dt, op, rmw, dl, s, f)    u8_st_tnsr_partial_rmw_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)

#define f32_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)   v_f32_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, SW_PACK, p, o)
#define bf16_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)  v_bf16_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, SW_PACK, p, o)
#define i32_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)   v_i32_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, SW_PACK, p, o)
#define u32_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)   v_u32_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, SW_PACK, p, o)
#define i16_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)   v_i16_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, SW_PACK, p, o)
#define u16_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)   v_u16_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, SW_PACK, p, o)
#define i8_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)    v_i8_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, SW_PACK, p, o)
#define u8_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, p, o)    v_u8_st_tnsr_partial_rmw(n, t, v, MkRMW(dt, op, rmw, dl), s, f, SW_PACK, p, o)

#define f32_st_tnsr_partial_rmw_pack_i_v(n, t, v, dt, op, rmw, dl, s, f)    f32_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define bf16_st_tnsr_partial_rmw_pack_i_v(n, t, v, dt, op, rmw, dl, s, f)   bf16_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define f16_st_tnsr_partial_rmw_pack_i_v(n, t, v, dt, op, rmw, dl, s, f)    f16_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define i32_st_tnsr_partial_rmw_pack_i_v(n, t, v, dt, op, rmw, dl, s, f)    i32_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define u32_st_tnsr_partial_rmw_pack_i_v(n, t, v, dt, op, rmw, dl, s, f)    u32_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define i16_st_tnsr_partial_rmw_pack_i_v(n, t, v, dt, op, rmw, dl, s, f)    i16_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define u16_st_tnsr_partial_rmw_pack_i_v(n, t, v, dt, op, rmw, dl, s, f)    u16_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define i8_st_tnsr_partial_rmw_pack_i_v(n, t, v, dt, op, rmw, dl, s, f)     i8_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)
#define u8_st_tnsr_partial_rmw_pack_i_v(n, t, v, dt, op, rmw, dl, s, f)     u8_st_tnsr_partial_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, s, f, 1, 0)

#define f32_st_tnsr_partial_reg_rmw_i_s_v_b         f32_st_tnsr_partial_rmw_i_v_b
#define bf16_st_tnsr_partial_reg_rmw_i_s_v_b        bf16_st_tnsr_partial_rmw_i_v_b
#define f16_st_tnsr_partial_reg_rmw_i_s_v_b         f16_st_tnsr_partial_rmw_i_v_b
#define i32_st_tnsr_partial_reg_rmw_i_s_v_b         i32_st_tnsr_partial_rmw_i_v_b
#define u32_st_tnsr_partial_reg_rmw_i_s_v_b         u32_st_tnsr_partial_rmw_i_v_b
#define i16_st_tnsr_partial_reg_rmw_i_s_v_b         i16_st_tnsr_partial_rmw_i_v_b
#define u16_st_tnsr_partial_reg_rmw_i_s_v_b         u16_st_tnsr_partial_rmw_i_v_b
#define i8_st_tnsr_partial_reg_rmw_i_s_v_b          i8_st_tnsr_partial_rmw_i_v_b
#define u8_st_tnsr_partial_reg_rmw_i_s_v_b          u8_st_tnsr_partial_rmw_i_v_b

#define f32_st_tnsr_partial_reg_rmw_i_s_v           f32_st_tnsr_partial_rmw_i_v
#define bf16_st_tnsr_partial_reg_rmw_i_s_v          bf16_st_tnsr_partial_rmw_i_v
#define f16_st_tnsr_partial_reg_rmw_i_s_v           f16_st_tnsr_partial_rmw_i_v
#define i32_st_tnsr_partial_reg_rmw_i_s_v           i32_st_tnsr_partial_rmw_i_v
#define u32_st_tnsr_partial_reg_rmw_i_s_v           u32_st_tnsr_partial_rmw_i_v
#define i16_st_tnsr_partial_reg_rmw_i_s_v           i16_st_tnsr_partial_rmw_i_v
#define u16_st_tnsr_partial_reg_rmw_i_s_v           u16_st_tnsr_partial_rmw_i_v
#define i8_st_tnsr_partial_reg_rmw_i_s_v            i8_st_tnsr_partial_rmw_i_v
#define u8_st_tnsr_partial_reg_rmw_i_s_v            u8_st_tnsr_partial_rmw_i_v

#define f32_st_tnsr_partial_reg_rmw_pack_i_s_v_b    f32_st_tnsr_partial_rmw_pack_i_v_b
#define bf16_st_tnsr_partial_reg_rmw_pack_i_s_v_b   bf16_st_tnsr_partial_rmw_pack_i_v_b
#define f16_st_tnsr_partial_reg_rmw_pack_i_s_v_b    f16_st_tnsr_partial_rmw_pack_i_v_b
#define i32_st_tnsr_partial_reg_rmw_pack_i_s_v_b    i32_st_tnsr_partial_rmw_pack_i_v_b
#define u32_st_tnsr_partial_reg_rmw_pack_i_s_v_b    u32_st_tnsr_partial_rmw_pack_i_v_b
#define i16_st_tnsr_partial_reg_rmw_pack_i_s_v_b    i16_st_tnsr_partial_rmw_pack_i_v_b
#define u16_st_tnsr_partial_reg_rmw_pack_i_s_v_b    u16_st_tnsr_partial_rmw_pack_i_v_b
#define i8_st_tnsr_partial_reg_rmw_pack_i_s_v_b     i8_st_tnsr_partial_rmw_pack_i_v_b
#define u8_st_tnsr_partial_reg_rmw_pack_i_s_v_b     u8_st_tnsr_partial_rmw_pack_i_v_b

#define f32_st_tnsr_partial_reg_rmw_pack_i_s_v      f32_st_tnsr_partial_rmw_pack_i_v
#define bf16_st_tnsr_partial_reg_rmw_pack_i_s_v     bf16_st_tnsr_partial_rmw_pack_i_v
#define i32_st_tnsr_partial_reg_rmw_pack_i_s_v      i32_st_tnsr_partial_rmw_pack_i_v
#define u32_st_tnsr_partial_reg_rmw_pack_i_s_v      u32_st_tnsr_partial_rmw_pack_i_v
#define i16_st_tnsr_partial_reg_rmw_pack_i_s_v      i16_st_tnsr_partial_rmw_pack_i_v
#define u16_st_tnsr_partial_reg_rmw_pack_i_s_v      u16_st_tnsr_partial_rmw_pack_i_v
#define i8_st_tnsr_partial_reg_rmw_pack_i_s_v       i8_st_tnsr_partial_rmw_pack_i_v
#define u8_st_tnsr_partial_reg_rmw_pack_i_s_v       u8_st_tnsr_partial_rmw_pack_i_v

// ST_TNSR_LOW


#define f32_st_tnsr_low_i_v_b(n, t, v, p, o)    v_f32_st_tnsr_low(n, t, v, 0, p, o)
#define bf16_st_tnsr_low_i_v_b(n, t, v, p, o)   v_bf16_st_tnsr_low(n, t, v, 0, p, o)
#define i32_st_tnsr_low_i_v_b(n, t, v, p, o)    v_i32_st_tnsr_low(n, t, v, 0, p, o)
#define u32_st_tnsr_low_i_v_b(n, t, v, p, o)    v_u32_st_tnsr_low(n, t, v, 0, p, o)
#define i16_st_tnsr_low_i_v_b(n, t, v, p, o)    v_i16_st_tnsr_low(n, t, v, 0, p, o)
#define u16_st_tnsr_low_i_v_b(n, t, v, p, o)    v_u16_st_tnsr_low(n, t, v, 0, p, o)
#define i8_st_tnsr_low_i_v_b(n, t, v, p, o)     v_i8_st_tnsr_low(n, t, v, 0, p, o)
#define u8_st_tnsr_low_i_v_b(n, t, v, p, o)     v_u8_st_tnsr_low(n, t, v, 0, p, o)
#define st_tnsr_low_i_bv_b(n, t, v, p, o)       v_i1_st_tnsr_low(n, t, v, 0, p, o)

#define f32_st_tnsr_low_i_v(n, t, v)            f32_st_tnsr_low_i_v_b(n, t, v, 1, 0)
#define bf16_st_tnsr_low_i_v(n, t, v)           bf16_st_tnsr_low_i_v_b(n, t, v, 1, 0)
#define f16_st_tnsr_low_i_v(n, t, v)            f16_st_tnsr_low_i_v_b(n, t, v, 1, 0)
#define i32_st_tnsr_low_i_v(n, t, v)            i32_st_tnsr_low_i_v_b(n, t, v, 1, 0)
#define u32_st_tnsr_low_i_v(n, t, v)            u32_st_tnsr_low_i_v_b(n, t, v, 1, 0)
#define i16_st_tnsr_low_i_v(n, t, v)            i16_st_tnsr_low_i_v_b(n, t, v, 1, 0)
#define u16_st_tnsr_low_i_v(n, t, v)            u16_st_tnsr_low_i_v_b(n, t, v, 1, 0)
#define i8_st_tnsr_low_i_v(n, t, v)             i8_st_tnsr_low_i_v_b(n, t, v, 1, 0)
#define u8_st_tnsr_low_i_v(n, t, v)             u8_st_tnsr_low_i_v_b(n, t, v, 1, 0)
#define st_tnsr_low_i_bv(n, t, v)               st_tnsr_low_i_bv_b(n, t, v, 1, 0)

#define f32_st_tnsr_low_pack_i_v_b(n, t, v, p, o)     v_f32_st_tnsr_low(n, t, v, SW_PACK, p, o)
#define bf16_st_tnsr_low_pack_i_v_b(n, t, v, p, o)    v_bf16_st_tnsr_low(n, t, v, SW_PACK, p, o)
#define i32_st_tnsr_low_pack_i_v_b(n, t, v, p, o)     v_i32_st_tnsr_low(n, t, v, SW_PACK, p, o)
#define u32_st_tnsr_low_pack_i_v_b(n, t, v, p, o)     v_u32_st_tnsr_low(n, t, v, SW_PACK, p, o)
#define i16_st_tnsr_low_pack_i_v_b(n, t, v, p, o)     v_i16_st_tnsr_low(n, t, v, SW_PACK, p, o)
#define u16_st_tnsr_low_pack_i_v_b(n, t, v, p, o)     v_u16_st_tnsr_low(n, t, v, SW_PACK, p, o)
#define i8_st_tnsr_low_pack_i_v_b(n, t, v, p, o)      v_i8_st_tnsr_low(n, t, v, SW_PACK, p, o)
#define u8_st_tnsr_low_pack_i_v_b(n, t, v, p, o)      v_u8_st_tnsr_low(n, t, v, SW_PACK, p, o)

#define f32_st_tnsr_low_pack_i_v(n, t, v)             f32_st_tnsr_low_pack_i_v_b(n, t, v, 1, 0)
#define bf16_st_tnsr_low_pack_i_v(n, t, v)            bf16_st_tnsr_low_pack_i_v_b(n, t, v, 1, 0)
#define f16_st_tnsr_low_pack_i_v(n, t, v)             f16_st_tnsr_low_pack_i_v_b(n, t, v, 1, 0)
#define i32_st_tnsr_low_pack_i_v(n, t, v)             i32_st_tnsr_low_pack_i_v_b(n, t, v, 1, 0)
#define u32_st_tnsr_low_pack_i_v(n, t, v)             u32_st_tnsr_low_pack_i_v_b(n, t, v, 1, 0)
#define i16_st_tnsr_low_pack_i_v(n, t, v)             i16_st_tnsr_low_pack_i_v_b(n, t, v, 1, 0)
#define u16_st_tnsr_low_pack_i_v(n, t, v)             u16_st_tnsr_low_pack_i_v_b(n, t, v, 1, 0)
#define i8_st_tnsr_low_pack_i_v(n, t, v)              i8_st_tnsr_low_pack_i_v_b(n, t, v, 1, 0)
#define u8_st_tnsr_low_pack_i_v(n, t, v)              u8_st_tnsr_low_pack_i_v_b(n, t, v, 1, 0)

#define f32_st_tnsr_low_reg_i_s_v_b         f32_st_tnsr_low_i_v_b
#define bf16_st_tnsr_low_reg_i_s_v_b        bf16_st_tnsr_low_i_v_b
#define f16_st_tnsr_low_reg_i_s_v_b         f16_st_tnsr_low_i_v_b
#define i32_st_tnsr_low_reg_i_s_v_b         i32_st_tnsr_low_i_v_b
#define u32_st_tnsr_low_reg_i_s_v_b         u32_st_tnsr_low_i_v_b
#define i16_st_tnsr_low_reg_i_s_v_b         i16_st_tnsr_low_i_v_b
#define u16_st_tnsr_low_reg_i_s_v_b         u16_st_tnsr_low_i_v_b
#define i8_st_tnsr_low_reg_i_s_v_b          i8_st_tnsr_low_i_v_b
#define u8_st_tnsr_low_reg_i_s_v_b          u8_st_tnsr_low_i_v_b
#define st_tnsr_low_reg_i_s_bv_b            st_tnsr_low_i_bv_b

#define f32_st_tnsr_low_reg_i_s_v           f32_st_tnsr_low_i_v
#define bf16_st_tnsr_low_reg_i_s_v          bf16_st_tnsr_low_i_v
#define f16_st_tnsr_low_reg_i_s_v           f16_st_tnsr_low_i_v
#define i32_st_tnsr_low_reg_i_s_v           i32_st_tnsr_low_i_v
#define u32_st_tnsr_low_reg_i_s_v           u32_st_tnsr_low_i_v
#define i16_st_tnsr_low_reg_i_s_v           i16_st_tnsr_low_i_v
#define u16_st_tnsr_low_reg_i_s_v           u16_st_tnsr_low_i_v
#define i8_st_tnsr_low_reg_i_s_v            i8_st_tnsr_low_i_v
#define u8_st_tnsr_low_reg_i_s_v            u8_st_tnsr_low_i_v
#define st_tnsr_low_reg_i_s_bv              st_tnsr_low_i_bv

#define f32_st_tnsr_low_reg_pack_i_s_v_b    f32_st_tnsr_low_pack_i_v_b
#define bf16_st_tnsr_low_reg_pack_i_s_v_b   bf16_st_tnsr_low_pack_i_v_b
#define f16_st_tnsr_low_reg_pack_i_s_v_b    f16_st_tnsr_low_pack_i_v_b
#define i32_st_tnsr_low_reg_pack_i_s_v_b    i32_st_tnsr_low_pack_i_v_b
#define u32_st_tnsr_low_reg_pack_i_s_v_b    u32_st_tnsr_low_pack_i_v_b
#define i16_st_tnsr_low_reg_pack_i_s_v_b    i16_st_tnsr_low_pack_i_v_b
#define u16_st_tnsr_low_reg_pack_i_s_v_b    u16_st_tnsr_low_pack_i_v_b
#define i8_st_tnsr_low_reg_pack_i_s_v_b     i8_st_tnsr_low_pack_i_v_b
#define u8_st_tnsr_low_reg_pack_i_s_v_b     u8_st_tnsr_low_pack_i_v_b

#define f32_st_tnsr_low_reg_pack_i_s_v      f32_st_tnsr_low_pack_i_v
#define bf16_st_tnsr_low_reg_pack_i_s_v     bf16_st_tnsr_low_pack_i_v
#define f16_st_tnsr_low_reg_pack_i_s_v      f16_st_tnsr_low_pack_i_v
#define i32_st_tnsr_low_reg_pack_i_s_v      i32_st_tnsr_low_pack_i_v
#define u32_st_tnsr_low_reg_pack_i_s_v      u32_st_tnsr_low_pack_i_v
#define i16_st_tnsr_low_reg_pack_i_s_v      i16_st_tnsr_low_pack_i_v
#define u16_st_tnsr_low_reg_pack_i_s_v      u16_st_tnsr_low_pack_i_v
#define i8_st_tnsr_low_reg_pack_i_s_v       i8_st_tnsr_low_pack_i_v
#define u8_st_tnsr_low_reg_pack_i_s_v       u8_st_tnsr_low_pack_i_v

#define f32_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_f32_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define bf16_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)  v_bf16_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define i32_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_i32_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define u32_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_u32_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define i16_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_i16_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define u16_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_u16_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define i8_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)    v_i8_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define u8_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)    v_u8_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)

#define f32_st_tnsr_low_rmw_i_v(n, t, v, dt, op, rmw, dl)   f32_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define bf16_st_tnsr_low_rmw_i_v(n, t, v, dt, op, rmw, dl)  bf16_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define f16_st_tnsr_low_rmw_i_v(n, t, v, dt, op, rmw, dl)   f16_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i32_st_tnsr_low_rmw_i_v(n, t, v, dt, op, rmw, dl)   i32_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u32_st_tnsr_low_rmw_i_v(n, t, v, dt, op, rmw, dl)   u32_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i16_st_tnsr_low_rmw_i_v(n, t, v, dt, op, rmw, dl)   i16_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u16_st_tnsr_low_rmw_i_v(n, t, v, dt, op, rmw, dl)   u16_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i8_st_tnsr_low_rmw_i_v(n, t, v, dt, op, rmw, dl)    i8_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u8_st_tnsr_low_rmw_i_v(n, t, v, dt, op, rmw, dl)    u8_st_tnsr_low_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)

#define f32_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_f32_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define bf16_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)  v_bf16_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define i32_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_i32_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define u32_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_u32_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define i16_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_i16_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define u16_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_u16_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define i8_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)    v_i8_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define u8_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)    v_u8_st_tnsr_low_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)

#define f32_st_tnsr_low_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    f32_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define bf16_st_tnsr_low_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)   bf16_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define f16_st_tnsr_low_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    f16_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i32_st_tnsr_low_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    i32_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u32_st_tnsr_low_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    u32_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i16_st_tnsr_low_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    i16_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u16_st_tnsr_low_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    u16_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i8_st_tnsr_low_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)     i8_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u8_st_tnsr_low_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)     u8_st_tnsr_low_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)

#define f32_st_tnsr_low_reg_rmw_i_s_v_b       f32_st_tnsr_low_rmw_i_v_b
#define bf16_st_tnsr_low_reg_rmw_i_s_v_b      bf16_st_tnsr_low_rmw_i_v_b
#define f16_st_tnsr_low_reg_rmw_i_s_v_b       f16_st_tnsr_low_rmw_i_v_b
#define i32_st_tnsr_low_reg_rmw_i_s_v_b       i32_st_tnsr_low_rmw_i_v_b
#define u32_st_tnsr_low_reg_rmw_i_s_v_b       u32_st_tnsr_low_rmw_i_v_b
#define i16_st_tnsr_low_reg_rmw_i_s_v_b       i16_st_tnsr_low_rmw_i_v_b
#define u16_st_tnsr_low_reg_rmw_i_s_v_b       u16_st_tnsr_low_rmw_i_v_b
#define i8_st_tnsr_low_reg_rmw_i_s_v_b        i8_st_tnsr_low_rmw_i_v_b
#define u8_st_tnsr_low_reg_rmw_i_s_v_b        u8_st_tnsr_low_rmw_i_v_b

#define f32_st_tnsr_low_reg_rmw_i_s_v         f32_st_tnsr_low_rmw_i_v
#define bf16_st_tnsr_low_reg_rmw_i_s_v        bf16_st_tnsr_low_rmw_i_v
#define f16_st_tnsr_low_reg_rmw_i_s_v         f16_st_tnsr_low_rmw_i_v
#define i32_st_tnsr_low_reg_rmw_i_s_v         i32_st_tnsr_low_rmw_i_v
#define u32_st_tnsr_low_reg_rmw_i_s_v         u32_st_tnsr_low_rmw_i_v
#define i16_st_tnsr_low_reg_rmw_i_s_v         i16_st_tnsr_low_rmw_i_v
#define u16_st_tnsr_low_reg_rmw_i_s_v         u16_st_tnsr_low_rmw_i_v
#define i8_st_tnsr_low_reg_rmw_i_s_v          i8_st_tnsr_low_rmw_i_v
#define u8_st_tnsr_low_reg_rmw_i_s_v          u8_st_tnsr_low_rmw_i_v

#define f32_st_tnsr_low_reg_rmw_pack_i_s_v_b    f32_st_tnsr_low_rmw_pack_i_v_b
#define bf16_st_tnsr_low_reg_rmw_pack_i_s_v_b   bf16_st_tnsr_low_rmw_pack_i_v_b
#define f16_st_tnsr_low_reg_rmw_pack_i_s_v_b    f16_st_tnsr_low_rmw_pack_i_v_b
#define i32_st_tnsr_low_reg_rmw_pack_i_s_v_b    i32_st_tnsr_low_rmw_pack_i_v_b
#define u32_st_tnsr_low_reg_rmw_pack_i_s_v_b    u32_st_tnsr_low_rmw_pack_i_v_b
#define i16_st_tnsr_low_reg_rmw_pack_i_s_v_b    i16_st_tnsr_low_rmw_pack_i_v_b
#define u16_st_tnsr_low_reg_rmw_pack_i_s_v_b    u16_st_tnsr_low_rmw_pack_i_v_b
#define i8_st_tnsr_low_reg_rmw_pack_i_s_v_b     i8_st_tnsr_low_rmw_pack_i_v_b
#define u8_st_tnsr_low_reg_rmw_pack_i_s_v_b     u8_st_tnsr_low_rmw_pack_i_v_b

#define f32_st_tnsr_low_reg_rmw_pack_i_s_v      f32_st_tnsr_low_rmw_pack_i_v
#define bf16_st_tnsr_low_reg_rmw_pack_i_s_v     bf16_st_tnsr_low_rmw_pack_i_v
#define i32_st_tnsr_low_reg_rmw_pack_i_s_v      i32_st_tnsr_low_rmw_pack_i_v
#define u32_st_tnsr_low_reg_rmw_pack_i_s_v      u32_st_tnsr_low_rmw_pack_i_v
#define i16_st_tnsr_low_reg_rmw_pack_i_s_v      i16_st_tnsr_low_rmw_pack_i_v
#define u16_st_tnsr_low_reg_rmw_pack_i_s_v      u16_st_tnsr_low_rmw_pack_i_v
#define i8_st_tnsr_low_reg_rmw_pack_i_s_v       i8_st_tnsr_low_rmw_pack_i_v
#define u8_st_tnsr_low_reg_rmw_pack_i_s_v       u8_st_tnsr_low_rmw_pack_i_v

// ST_TNSR_HIGH
#define f32_st_tnsr_high_i_v_b(n, t, v, p, o)    v_f32_st_tnsr_high(n, t, v, 0, p, o)
#define bf16_st_tnsr_high_i_v_b(n, t, v, p, o)   v_bf16_st_tnsr_high(n, t, v, 0, p, o)
#define i32_st_tnsr_high_i_v_b(n, t, v, p, o)    v_i32_st_tnsr_high(n, t, v, 0, p, o)
#define u32_st_tnsr_high_i_v_b(n, t, v, p, o)    v_u32_st_tnsr_high(n, t, v, 0, p, o)
#define i16_st_tnsr_high_i_v_b(n, t, v, p, o)    v_i16_st_tnsr_high(n, t, v, 0, p, o)
#define u16_st_tnsr_high_i_v_b(n, t, v, p, o)    v_u16_st_tnsr_high(n, t, v, 0, p, o)
#define i8_st_tnsr_high_i_v_b(n, t, v, p, o)     v_i8_st_tnsr_high(n, t, v, 0, p, o)
#define u8_st_tnsr_high_i_v_b(n, t, v, p, o)     v_u8_st_tnsr_high(n, t, v, 0, p, o)
#define st_tnsr_high_i_bv_b(n, t, v, p, o)       v_i1_st_tnsr_high(n, t, v, 0, p, o)

#define f32_st_tnsr_high_i_v(n, t, v)            f32_st_tnsr_high_i_v_b(n, t, v, 1, 0)
#define bf16_st_tnsr_high_i_v(n, t, v)           bf16_st_tnsr_high_i_v_b(n, t, v, 1, 0)
#define i32_st_tnsr_high_i_v(n, t, v)            i32_st_tnsr_high_i_v_b(n, t, v, 1, 0)
#define u32_st_tnsr_high_i_v(n, t, v)            u32_st_tnsr_high_i_v_b(n, t, v, 1, 0)
#define i16_st_tnsr_high_i_v(n, t, v)            i16_st_tnsr_high_i_v_b(n, t, v, 1, 0)
#define u16_st_tnsr_high_i_v(n, t, v)            u16_st_tnsr_high_i_v_b(n, t, v, 1, 0)
#define i8_st_tnsr_high_i_v(n, t, v)             i8_st_tnsr_high_i_v_b(n, t, v, 1, 0)
#define u8_st_tnsr_high_i_v(n, t, v)             u8_st_tnsr_high_i_v_b(n, t, v, 1, 0)
#define st_tnsr_high_i_bv(n, t, v)               st_tnsr_high_i_bv_b(n, t, v, 1, 0)

#define f32_st_tnsr_high_pack_i_v_b(n, t, v, p, o)     v_f32_st_tnsr_high(n, t, v, SW_PACK, p, o)
#define bf16_st_tnsr_high_pack_i_v_b(n, t, v, p, o)    v_bf16_st_tnsr_high(n, t, v, SW_PACK, p, o)
#define i32_st_tnsr_high_pack_i_v_b(n, t, v, p, o)     v_i32_st_tnsr_high(n, t, v, SW_PACK, p, o)
#define u32_st_tnsr_high_pack_i_v_b(n, t, v, p, o)     v_u32_st_tnsr_high(n, t, v, SW_PACK, p, o)
#define i16_st_tnsr_high_pack_i_v_b(n, t, v, p, o)     v_i16_st_tnsr_high(n, t, v, SW_PACK, p, o)
#define u16_st_tnsr_high_pack_i_v_b(n, t, v, p, o)     v_u16_st_tnsr_high(n, t, v, SW_PACK, p, o)
#define i8_st_tnsr_high_pack_i_v_b(n, t, v, p, o)      v_i8_st_tnsr_high(n, t, v, SW_PACK, p, o)
#define u8_st_tnsr_high_pack_i_v_b(n, t, v, p, o)      v_u8_st_tnsr_high(n, t, v, SW_PACK, p, o)

#define f32_st_tnsr_high_pack_i_v(n, t, v)             f32_st_tnsr_high_pack_i_v_b(n, t, v, 1, 0)
#define bf16_st_tnsr_high_pack_i_v(n, t, v)            bf16_st_tnsr_high_pack_i_v_b(n, t, v, 1, 0)
#define f16_st_tnsr_high_pack_i_v(n, t, v)             f16_st_tnsr_high_pack_i_v_b(n, t, v, 1, 0)
#define i32_st_tnsr_high_pack_i_v(n, t, v)             i32_st_tnsr_high_pack_i_v_b(n, t, v, 1, 0)
#define u32_st_tnsr_high_pack_i_v(n, t, v)             u32_st_tnsr_high_pack_i_v_b(n, t, v, 1, 0)
#define i16_st_tnsr_high_pack_i_v(n, t, v)             i16_st_tnsr_high_pack_i_v_b(n, t, v, 1, 0)
#define u16_st_tnsr_high_pack_i_v(n, t, v)             u16_st_tnsr_high_pack_i_v_b(n, t, v, 1, 0)
#define i8_st_tnsr_high_pack_i_v(n, t, v)              i8_st_tnsr_high_pack_i_v_b(n, t, v, 1, 0)
#define u8_st_tnsr_high_pack_i_v(n, t, v)              u8_st_tnsr_high_pack_i_v_b(n, t, v, 1, 0)

#define f32_st_tnsr_high_reg_i_s_v_b         f32_st_tnsr_high_i_v_b
#define bf16_st_tnsr_high_reg_i_s_v_b        bf16_st_tnsr_high_i_v_b
#define f16_st_tnsr_high_reg_i_s_v_b         f16_st_tnsr_high_i_v_b
#define i32_st_tnsr_high_reg_i_s_v_b         i32_st_tnsr_high_i_v_b
#define u32_st_tnsr_high_reg_i_s_v_b         u32_st_tnsr_high_i_v_b
#define i16_st_tnsr_high_reg_i_s_v_b         i16_st_tnsr_high_i_v_b
#define u16_st_tnsr_high_reg_i_s_v_b         u16_st_tnsr_high_i_v_b
#define i8_st_tnsr_high_reg_i_s_v_b          i8_st_tnsr_high_i_v_b
#define u8_st_tnsr_high_reg_i_s_v_b          u8_st_tnsr_high_i_v_b
#define st_tnsr_high_reg_i_s_bv_b            st_tnsr_high_i_bv_b

#define f32_st_tnsr_high_reg_i_s_v           f32_st_tnsr_high_i_v
#define bf16_st_tnsr_high_reg_i_s_v          bf16_st_tnsr_high_i_v
#define f16_st_tnsr_high_reg_i_s_v           f16_st_tnsr_high_i_v
#define i32_st_tnsr_high_reg_i_s_v           i32_st_tnsr_high_i_v
#define u32_st_tnsr_high_reg_i_s_v           u32_st_tnsr_high_i_v
#define i16_st_tnsr_high_reg_i_s_v           i16_st_tnsr_high_i_v
#define u16_st_tnsr_high_reg_i_s_v           u16_st_tnsr_high_i_v
#define i8_st_tnsr_high_reg_i_s_v            i8_st_tnsr_high_i_v
#define u8_st_tnsr_high_reg_i_s_v            u8_st_tnsr_high_i_v
#define st_tnsr_high_reg_i_s_bv              st_tnsr_high_i_bv

#define f32_st_tnsr_high_reg_pack_i_s_v_b    f32_st_tnsr_high_pack_i_v_b
#define bf16_st_tnsr_high_reg_pack_i_s_v_b   bf16_st_tnsr_high_pack_i_v_b
#define f16_st_tnsr_high_reg_pack_i_s_v_b    f16_st_tnsr_high_pack_i_v_b
#define i32_st_tnsr_high_reg_pack_i_s_v_b    i32_st_tnsr_high_pack_i_v_b
#define u32_st_tnsr_high_reg_pack_i_s_v_b    u32_st_tnsr_high_pack_i_v_b
#define i16_st_tnsr_high_reg_pack_i_s_v_b    i16_st_tnsr_high_pack_i_v_b
#define u16_st_tnsr_high_reg_pack_i_s_v_b    u16_st_tnsr_high_pack_i_v_b
#define i8_st_tnsr_high_reg_pack_i_s_v_b     i8_st_tnsr_high_pack_i_v_b
#define u8_st_tnsr_high_reg_pack_i_s_v_b     u8_st_tnsr_high_pack_i_v_b

#define f32_st_tnsr_high_reg_pack_i_s_v      f32_st_tnsr_high_pack_i_v
#define bf16_st_tnsr_high_reg_pack_i_s_v     bf16_st_tnsr_high_pack_i_v
#define f16_st_tnsr_high_reg_pack_i_s_v      f16_st_tnsr_high_pack_i_v
#define i32_st_tnsr_high_reg_pack_i_s_v      i32_st_tnsr_high_pack_i_v
#define u32_st_tnsr_high_reg_pack_i_s_v      u32_st_tnsr_high_pack_i_v
#define i16_st_tnsr_high_reg_pack_i_s_v      i16_st_tnsr_high_pack_i_v
#define u16_st_tnsr_high_reg_pack_i_s_v      u16_st_tnsr_high_pack_i_v
#define i8_st_tnsr_high_reg_pack_i_s_v       i8_st_tnsr_high_pack_i_v
#define u8_st_tnsr_high_reg_pack_i_s_v       u8_st_tnsr_high_pack_i_v

#define f32_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_f32_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define bf16_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)  v_bf16_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define i32_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_i32_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define u32_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_u32_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define i16_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_i16_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define u16_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_u16_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define i8_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)    v_i8_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)
#define u8_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, p, o)    v_u8_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), 0, p, o)

#define f32_st_tnsr_high_rmw_i_v(n, t, v, dt, op, rmw, dl)   f32_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define bf16_st_tnsr_high_rmw_i_v(n, t, v, dt, op, rmw, dl)  bf16_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define f16_st_tnsr_high_rmw_i_v(n, t, v, dt, op, rmw, dl)   f16_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i32_st_tnsr_high_rmw_i_v(n, t, v, dt, op, rmw, dl)   i32_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u32_st_tnsr_high_rmw_i_v(n, t, v, dt, op, rmw, dl)   u32_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i16_st_tnsr_high_rmw_i_v(n, t, v, dt, op, rmw, dl)   i16_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u16_st_tnsr_high_rmw_i_v(n, t, v, dt, op, rmw, dl)   u16_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i8_st_tnsr_high_rmw_i_v(n, t, v, dt, op, rmw, dl)    i8_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u8_st_tnsr_high_rmw_i_v(n, t, v, dt, op, rmw, dl)    u8_st_tnsr_high_rmw_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)

#define f32_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_f32_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define bf16_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)  v_bf16_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define i32_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_i32_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define u32_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_u32_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define i16_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_i16_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define u16_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)   v_u16_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define i8_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)    v_i8_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)
#define u8_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, p, o)    v_u8_st_tnsr_high_rmw(n, t, v, MkRMW(dt, op, rmw, dl), SW_PACK, p, o)

#define f32_st_tnsr_high_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    f32_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define bf16_st_tnsr_high_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)   bf16_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i32_st_tnsr_high_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    i32_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u32_st_tnsr_high_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    u32_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i16_st_tnsr_high_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    i16_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u16_st_tnsr_high_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)    u16_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define i8_st_tnsr_high_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)     i8_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)
#define u8_st_tnsr_high_rmw_pack_i_v(n, t, v, dt, op, rmw, dl)     u8_st_tnsr_high_rmw_pack_i_v_b(n, t, v, dt, op, rmw, dl, 1, 0)

#define f32_st_tnsr_high_reg_rmw_i_s_v_b       f32_st_tnsr_high_rmw_i_v_b
#define bf16_st_tnsr_high_reg_rmw_i_s_v_b      bf16_st_tnsr_high_rmw_i_v_b
#define f16_st_tnsr_high_reg_rmw_i_s_v_b       f16_st_tnsr_high_rmw_i_v_b
#define i32_st_tnsr_high_reg_rmw_i_s_v_b       i32_st_tnsr_high_rmw_i_v_b
#define u32_st_tnsr_high_reg_rmw_i_s_v_b       u32_st_tnsr_high_rmw_i_v_b
#define i16_st_tnsr_high_reg_rmw_i_s_v_b       i16_st_tnsr_high_rmw_i_v_b
#define u16_st_tnsr_high_reg_rmw_i_s_v_b       u16_st_tnsr_high_rmw_i_v_b
#define i8_st_tnsr_high_reg_rmw_i_s_v_b        i8_st_tnsr_high_rmw_i_v_b
#define u8_st_tnsr_high_reg_rmw_i_s_v_b        u8_st_tnsr_high_rmw_i_v_b

#define f32_st_tnsr_high_reg_rmw_i_s_v         f32_st_tnsr_high_rmw_i_v
#define bf16_st_tnsr_high_reg_rmw_i_s_v        bf16_st_tnsr_high_rmw_i_v
#define f16_st_tnsr_high_reg_rmw_i_s_v         f16_st_tnsr_high_rmw_i_v
#define i32_st_tnsr_high_reg_rmw_i_s_v         i32_st_tnsr_high_rmw_i_v
#define u32_st_tnsr_high_reg_rmw_i_s_v         u32_st_tnsr_high_rmw_i_v
#define i16_st_tnsr_high_reg_rmw_i_s_v         i16_st_tnsr_high_rmw_i_v
#define u16_st_tnsr_high_reg_rmw_i_s_v         u16_st_tnsr_high_rmw_i_v
#define i8_st_tnsr_high_reg_rmw_i_s_v          i8_st_tnsr_high_rmw_i_v
#define u8_st_tnsr_high_reg_rmw_i_s_v          u8_st_tnsr_high_rmw_i_v

#define f32_st_tnsr_high_reg_rmw_pack_i_s_v_b    f32_st_tnsr_high_rmw_pack_i_v_b
#define bf16_st_tnsr_high_reg_rmw_pack_i_s_v_b   bf16_st_tnsr_high_rmw_pack_i_v_b
#define f16_st_tnsr_high_reg_rmw_pack_i_s_v_b    f16_st_tnsr_high_rmw_pack_i_v_b
#define i32_st_tnsr_high_reg_rmw_pack_i_s_v_b    i32_st_tnsr_high_rmw_pack_i_v_b
#define u32_st_tnsr_high_reg_rmw_pack_i_s_v_b    u32_st_tnsr_high_rmw_pack_i_v_b
#define i16_st_tnsr_high_reg_rmw_pack_i_s_v_b    i16_st_tnsr_high_rmw_pack_i_v_b
#define u16_st_tnsr_high_reg_rmw_pack_i_s_v_b    u16_st_tnsr_high_rmw_pack_i_v_b
#define i8_st_tnsr_high_reg_rmw_pack_i_s_v_b     i8_st_tnsr_high_rmw_pack_i_v_b
#define u8_st_tnsr_high_reg_rmw_pack_i_s_v_b     u8_st_tnsr_high_rmw_pack_i_v_b

#define f32_st_tnsr_high_reg_rmw_pack_i_s_v      f32_st_tnsr_high_rmw_pack_i_v
#define bf16_st_tnsr_high_reg_rmw_pack_i_s_v     bf16_st_tnsr_high_rmw_pack_i_v
#define f16_st_tnsr_high_reg_rmw_pack_i_s_v      f16_st_tnsr_high_rmw_pack_i_v
#define i32_st_tnsr_high_reg_rmw_pack_i_s_v      i32_st_tnsr_high_rmw_pack_i_v
#define u32_st_tnsr_high_reg_rmw_pack_i_s_v      u32_st_tnsr_high_rmw_pack_i_v
#define i16_st_tnsr_high_reg_rmw_pack_i_s_v      i16_st_tnsr_high_rmw_pack_i_v
#define u16_st_tnsr_high_reg_rmw_pack_i_s_v      u16_st_tnsr_high_rmw_pack_i_v
#define i8_st_tnsr_high_reg_rmw_pack_i_s_v       i8_st_tnsr_high_rmw_pack_i_v
#define u8_st_tnsr_high_reg_rmw_pack_i_s_v       u8_st_tnsr_high_rmw_pack_i_v


// CONVERT
// for FP32 to BF16/FP16, when using tpc-c, there is a switch of ALL_LANES,
// which translates the instruction into 2 assembly instructions
// SINGLE_LANE with LANE_SEL=0 and SINGLE_LANE with LANE_SEL=
#if defined(__gaudi__)
#define v_convert_f32_to_bf16_all_vb(src, switches, income, predicate,         \
                                     polarity)                                 \
  v_bf16_mov_vb(v_convert_f32_to_bf16_all_b(src, switches,income,1,0),0,       \
                income,predicate, polarity                                     \
              )
#endif



#define s_convert_i8_to_f32_s_b(src, income, pred, pol)         s_convert_i8_to_f32(src, SW_CSR, income, pred, pol)
#define s_convert_i8_to_bf16_s_b(src, income, pred, pol)        s_convert_i8_to_bf16(src, SW_CSR, income, pred, pol)
#define s_convert_i8_to_i32_s_b(src, income, pred, pol)         s_convert_i8_to_i32(src, SW_CSR, income, pred, pol)
#define s_convert_i8_to_u32_s_b(src, income, pred, pol)         s_convert_i8_to_u32(src, SW_CSR, income, pred, pol)
#define s_convert_i8_to_i16_s_b(src, income, pred, pol)         s_convert_i8_to_i16(src, SW_CSR, income, pred, pol)
#define s_convert_i8_to_u16_s_b(src, income, pred, pol)         s_convert_i8_to_u16(src, SW_CSR, income, pred, pol)
#define s_convert_i8_to_u8_s_b(src, income, pred, pol)          s_convert_i8_to_u8(src, SW_CSR, income, pred, pol)
#define s_convert_i8_to_f32_s(src)                              s_convert_i8_to_f32_s_b(src, 0, 1, 0)
#define s_convert_i8_to_bf16_s(src)                             s_convert_i8_to_bf16_s_b(src, 0, 1, 0)
#define s_convert_i8_to_i32_s(src)                              s_convert_i8_to_i32_s_b(src, 0, 1, 0)
#define s_convert_i8_to_u32_s(src)                              s_convert_i8_to_u32_s_b(src, 0, 1, 0)
#define s_convert_i8_to_i16_s(src)                              s_convert_i8_to_i16_s_b(src, 0, 1, 0)
#define s_convert_i8_to_u16_s(src)                              s_convert_i8_to_u16_s_b(src, 0, 1, 0)
#define s_convert_i8_to_u8_s(src)                               s_convert_i8_to_u8_s_b(src, 0, 1, 0)

#define s_convert_u8_to_f32_s_b(src, income, pred, pol)         s_convert_u8_to_f32(src, SW_CSR, income, pred, pol)
#define s_convert_u8_to_bf16_s_b(src, income, pred, pol)        s_convert_u8_to_bf16(src, SW_CSR, income, pred, pol)
#define s_convert_u8_to_i32_s_b(src, income, pred, pol)         s_convert_u8_to_i32(src, SW_CSR, income, pred, pol)
#define s_convert_u8_to_u32_s_b(src, income, pred, pol)         s_convert_u8_to_u32(src, SW_CSR, income, pred, pol)
#define s_convert_u8_to_i16_s_b(src, income, pred, pol)         s_convert_u8_to_i16(src, SW_CSR, income, pred, pol)
#define s_convert_u8_to_u16_s_b(src, income, pred, pol)         s_convert_u8_to_u16(src, SW_CSR, income, pred, pol)
#define s_convert_u8_to_i8_s_b(src, income, pred, pol)          s_convert_u8_to_i8(src, SW_CSR, income, pred, pol)
#define s_convert_u8_to_f32_s(src)                              s_convert_u8_to_f32_s_b(src, 0, 1, 0)
#define s_convert_u8_to_bf16_s(src)                             s_convert_u8_to_bf16_s_b(src, 0, 1, 0)
#define s_convert_u8_to_i32_s(src)                              s_convert_u8_to_i32_s_b(src, 0, 1, 0)
#define s_convert_u8_to_u32_s(src)                              s_convert_u8_to_u32_s_b(src, 0, 1, 0)
#define s_convert_u8_to_i16_s(src)                              s_convert_u8_to_i16_s_b(src, 0, 1, 0)
#define s_convert_u8_to_u16_s(src)                              s_convert_u8_to_u16_s_b(src, 0, 1, 0)
#define s_convert_u8_to_i8_s(src)                               s_convert_u8_to_i8_s_b(src, 0, 1, 0)

#define s_convert_i16_to_f32_s_b(src, income, pred, pol)        s_convert_i16_to_f32(src, SW_CSR, income, pred, pol)
#define s_convert_i16_to_bf16_s_b(src, income, rm, pred, pol)   s_convert_i16_to_bf16(src, (rm) << 16, income, pred, pol)
#define s_convert_i16_to_i32_s_b(src, income, pred, pol)        s_convert_i16_to_i32(src, SW_CSR, income, pred, pol)
#define s_convert_i16_to_u32_s_b(src, income, pred, pol)        s_convert_i16_to_u32(src, SW_CSR, income, pred, pol)
#define s_convert_i16_to_u16_s_b(src, income, pred, pol)        s_convert_i16_to_u16(src, SW_CSR, income, pred, pol)
#define s_convert_i16_to_i8_s_b(src, income, pred, pol)         s_convert_i16_to_i8(src, SW_CSR, income, pred, pol)
#define s_convert_i16_to_u8_s_b(src, income, pred, pol)         s_convert_i16_to_u8(src, SW_CSR, income, pred, pol)
#define s_convert_i16_to_f32_s(src)                             s_convert_i16_to_f32_s_b(src, 0, 1, 0)
#define s_convert_i16_to_bf16_s(src, rm)                        s_convert_i16_to_bf16_s_b(src, 0, rm, 1, 0)
#define s_convert_i16_to_i32_s(src)                             s_convert_i16_to_i32_s_b(src, 0, 1, 0)
#define s_convert_i16_to_u32_s(src)                             s_convert_i16_to_u32_s_b(src, 0, 1, 0)
#define s_convert_i16_to_u16_s(src)                             s_convert_i16_to_u16_s_b(src, 0, 1, 0)
#define s_convert_i16_to_u8_s(src)                              s_convert_i16_to_u8_s_b(src, 0, 1, 0)
#define s_convert_i16_to_i8_s(src)                              s_convert_i16_to_i8_s_b(src, 0, 1, 0)

#define s_convert_u16_to_bf16_s_b(src, income, rm, pred, pol)   s_convert_u16_to_bf16(src, (rm) << 16, income, pred, pol)
#define s_convert_u16_to_bf16_s(src, rm)                        s_convert_u16_to_bf16_s_b(src, 0, rm, 1, 0)

#define s_convert_i32_to_f32_s_b(src, income, rm, pred, pol)    s_convert_i32_to_f32(src, (rm) << 16, income, pred, pol)
#define s_convert_i32_to_bf16_s_b(src, income, rm, pred, pol)   s_convert_i32_to_bf16(src, (rm) << 16, income, pred, pol)
#define s_convert_i32_to_u32_s_b(src, income, rm, pred, pol)    s_convert_i32_to_u32(src, (rm) << 16, income, pred, pol)
#define s_convert_i32_to_f32_s(src, rm)                         s_convert_i32_to_f32_s_b(src, 0, rm, 1, 0)
#define s_convert_i32_to_u32_s(src, rm)                         s_convert_i32_to_u32_s_b(src, 0, rm, 1, 0)
#define s_convert_i32_to_bf16_s(src, rm)                        s_convert_i32_to_bf16_s_b(src, 0, rm, 1, 0)

#define s_convert_bf16_to_f32_s_b(src, income, pred, pol)       s_convert_bf16_to_f32(src, SW_CSR, income, pred, pol)
#define s_convert_bf16_to_i16_s_b(src, income, rm, pred, pol)   s_convert_bf16_to_i16(src, (rm) << 16, income, pred, pol)
#define s_convert_bf16_to_i8_s_b(src, income, rm, pred, pol)    s_convert_bf16_to_i8(src, (rm) << 16, income, pred, pol)
#define s_convert_bf16_to_f32_s(src)                            s_convert_bf16_to_f32_s_b(src, 0, 1, 0)
#define s_convert_bf16_to_i16_s(src, rm)                        s_convert_bf16_to_i16_s_b(src, 0, rm, 1, 0)
#define s_convert_bf16_to_i8_s(src, rm)                         s_convert_bf16_to_i8_s_b(src, 0, rm, 1, 0)


#define s_convert_f32_to_bf16_s_b(src, income, rm, pred, pol)   s_convert_f32_to_bf16(src, (rm) << 16, income, pred, pol)
#define s_convert_f32_to_i32_s_b(src, income, rm, pred, pol)    s_convert_f32_to_i32(src, (rm) << 16, income, pred, pol)
#define s_convert_f32_to_i16_s_b(src, income, rm, pred, pol)    s_convert_f32_to_i16(src, (rm) << 16, income, pred, pol)
#define s_convert_f32_to_i8_s_b(src, income, rm, pred, pol)     s_convert_f32_to_i8(src, (rm) << 16, income, pred, pol)
#define s_convert_f32_to_bf16_s(src, rm)                        s_convert_f32_to_bf16_s_b(src, 0, rm, 1, 0)
#define s_convert_f32_to_i32_s(src, rm)                         s_convert_f32_to_i32_s_b(src, 0, rm, 1, 0)
#define s_convert_f32_to_i16_s(src, rm)                         s_convert_f32_to_i16_s_b(src, 0, rm, 1, 0)
#define s_convert_f32_to_i8_s(src, rm)                          s_convert_f32_to_i8_s_b(src, 0, rm, 1, 0)

#define v_convert_i8_to_f32_v_vb(a, i, p, o)  v_convert_i8_to_f32_vb(a, 0, i, to_bool64(p), o)
#define v_convert_i8_to_f32_v_b(a, i, p, o)   v_convert_i8_to_f32_b(a, 0, i, p, o)
#define v_convert_i8_to_f32_v(a)              v_convert_i8_to_f32_v_b(a, 0, 1, 0)
#define v_convert_i8_to_i32_v_vb(a, i, p, o)  v_convert_i8_to_i32_vb(a, 0, i, to_bool64(p), o)
#define v_convert_i8_to_i32_v_b(a, i, p, o)   v_convert_i8_to_i32_b(a, 0, i, p, o)
#define v_convert_i8_to_i32_v(a)              v_convert_i8_to_i32_v_b(a, 0, 1, 0)
#define v_convert_i8_to_u32_v_vb(a, i, p, o)  v_convert_i8_to_u32_vb(a, 0, i, to_bool64(p), o)
#define v_convert_i8_to_u32_v_b(a, i, p, o)   v_convert_i8_to_u32_b(a, 0, i, p, o)
#define v_convert_i8_to_u32_v(a)              v_convert_i8_to_u32_v_b(a, 0, 1, 0)
#define v_convert_i8_to_i16_v_vb(a, i, p, o)  v_convert_i8_to_i16_vb(a, 0, i, to_bool128(p), o)
#define v_convert_i8_to_i16_v_b(a, i, p, o)   v_convert_i8_to_i16_b(a, 0, i, p, o)
#define v_convert_i8_to_i16_v(a)              v_convert_i8_to_i16_v_b(a, 0, 1, 0)
#define v_convert_i8_to_u16_v_vb(a, i, p, o)  v_convert_i8_to_u16_vb(a, 0, i, to_bool128(p), o)
#define v_convert_i8_to_u16_v_b(a, i, p, o)   v_convert_i8_to_u16_b(a, 0, i, p, o)
#define v_convert_i8_to_u16_v(a)              v_convert_i8_to_u16_v_b(a, 0, 1, 0)
#define v_convert_i8_to_u8_v_vb(a, i, p, o)   v_convert_i8_to_u8_vb(a, 0, i, p, o)
#define v_convert_i8_to_u8_v_b(a, i, p, o)    v_convert_i8_to_u8_b(a, 0, i, p, o)
#define v_convert_i8_to_u8_v(a)               v_convert_i8_to_u8_v_b(a, 0, 1, 0)

#define v_convert_i16_to_f32_v_vb(a, i, p, o)  v_convert_i16_to_f32_vb(a, 0, i, to_bool64(p), o)
#define v_convert_i16_to_f32_v_b(a, i, p, o)   v_convert_i16_to_f32_b(a, 0, i, p, o)
#define v_convert_i16_to_f32_v(a)              v_convert_i16_to_f32_v_b(a, 0, 1, 0)
#define v_convert_i16_to_i32_v_vb(a, i, p, o)  v_convert_i16_to_i32_vb(a, 0, i, to_bool64(p), o)
#define v_convert_i16_to_i32_v_b(a, i, p, o)   v_convert_i16_to_i32_b(a, 0, i, p, o)
#define v_convert_i16_to_i32_v(a)              v_convert_i16_to_i32_v_b(a, 0, 1, 0)
#define v_convert_i16_to_u32_v_vb(a, i, p, o)  v_convert_i16_to_u32_vb(a, 0, i, to_bool64(p), o)
#define v_convert_i16_to_u32_v_b(a, i, p, o)   v_convert_i16_to_u32_b(a, 0, i, p, o)
#define v_convert_i16_to_u32_v(a)              v_convert_i16_to_u32_v_b(a, 0, 1, 0)
#define v_convert_i16_to_u16_v_vb(a, i, p, o)  v_convert_i16_to_u16_vb(a, 0, i, to_bool128(p), o)
#define v_convert_i16_to_u16_v_b(a, i, p, o)   v_convert_i16_to_u16_b(a, 0, i, p, o)
#define v_convert_i16_to_u16_v(a)              v_convert_i16_to_u16_v_b(a, 0, 1, 0)

#define v_convert_i32_to_f32_v_vb(a, i, rm, p, o)  v_convert_i32_to_f32_vb(a, (rm <<16) , i, to_bool64(p), o)
#define v_convert_i32_to_f32_v_b(a, i, rm, p, o)   v_convert_i32_to_f32_b(a, (rm <<16), i, p, o)
#define v_convert_i32_to_f32_v(a, rm)              v_convert_i32_to_f32_v_b(a, 0, rm, 1, 0)
#define v_convert_i32_to_u32_v_vb(a, i, rm, p, o)  v_convert_i32_to_u32_vb(a, (rm <<16), i, to_bool64(p), o)
#define v_convert_i32_to_u32_v_b(a, i, rm, p, o)   v_convert_i32_to_u32_b(a, (rm <<16), i, p, o)
#define v_convert_i32_to_u32_v(a, rm)              v_convert_i32_to_u32_v_b(a, 0, rm, 1, 0)

#define v_convert_f32_to_i32_v_vb(a, i, rm, p, o)     v_convert_f32_to_i32_vb(a, (rm <<16), i, to_bool64(p), o)
#define v_convert_f32_to_i32_v_b(a, i, rm, p, o)      v_convert_f32_to_i32_b(a, (rm <<16), i, p, o)
#define v_convert_f32_to_i32_v(a, rm)                 v_convert_f32_to_i32_v_b(a, 0, rm, 1, 0)
#define v_convert_f32_to_i16_v_vb(a, i, rm, l, p, o)  v_convert_f32_to_i16_vb(a, l, (rm <<16), i, to_bool128(p), o)
#define v_convert_f32_to_i16_v_b(a, i, rm, l, p, o)   v_convert_f32_to_i16_b(a, l, (rm <<16), i, p, o)
#define v_convert_f32_to_i16_v(a, i, rm, l)           v_convert_f32_to_i16_v_b(a, i, rm, l, 1, 0)
#define v_convert_f32_to_i8_v_vb(a, i, rm, l, p, o)   v_convert_f32_to_i8_vb(a, l, (rm <<16), i, p, o)
#define v_convert_f32_to_i8_v_b(a, i, rm, l, p, o)    v_convert_f32_to_i8_b(a, l, (rm <<16), i, p, o)
#define v_convert_f32_to_i8_v(a, i, rm, l )           v_convert_f32_to_i8_v_b(a, i, rm, l, 1, 0)

#define v_convert_f32_to_bf16_single_lane_v_vb(a, i, rm, p, o)  v_convert_f32_to_bf16_single_vb(a, (rm <<16), i, to_bool128(p), o)
#define v_convert_f32_to_bf16_single_lane_v_b(a, i, rm, p, o)   v_convert_f32_to_bf16_single_b(a, (rm <<16), i, p, o)
#define v_convert_f32_to_bf16_single_lane_v(a, rm)              v_convert_f32_to_bf16_single_lane_v_b(a, 0, rm, 1, 0)

#define v_convert_f32_to_half_single_lane_v_vb(a, i, rm, p, o)  v_convert_f32_to_half_single_vb(a, (rm <<16), i, to_bool128(p), o)
#define v_convert_f32_to_half_single_lane_v_b(a, i, rm, p, o)   v_convert_f32_to_half_single_b(a, (rm <<16), i, p, o)
#define v_convert_f32_to_half_single_lane_v(a, rm)              v_convert_f32_to_half_single_lane_v_b(a, 0, rm, 1, 0)




#define v_convert_f32_to_bf16_av_vb(a, i, rm, p, o)    v_convert_f32_to_bf16_all_vb(a, (rm <<16), i, to_bool128(p), o)
#define v_convert_f32_to_bf16_av_b(a, i, rm, p, o)     v_convert_f32_to_bf16_all_b(a, (rm <<16), i, p, o)
#define v_convert_f32_to_bf16_av(a, rm)                v_convert_f32_to_bf16_av_b(a, 0, rm, 1, 0)
#define av_convert_bf16_to_f32_v_vb(a, i, p, o)        v_convert_bf16_to_f32_all_vb(a, 0, i, to_bool128(p), o)
#define av_convert_bf16_to_f32_v_b(a, i, p, o)         v_convert_bf16_to_f32_all_b(a, 0, i, p, o)
#define av_convert_bf16_to_f32_v(a)                    av_convert_bf16_to_f32_v_b(a, (float128){0},  1, 0)
#define v_convert_bf16_to_i16_v_vb(a, i, rm, p, o)     v_convert_bf16_to_i16_vb(a, (rm <<16), i, to_bool128(p), o)
#define v_convert_bf16_to_i16_v_b(a, i, rm, p, o)      v_convert_bf16_to_i16_b(a, (rm <<16), i, p, o)
#define v_convert_bf16_to_i16_v(a, rm)                 v_convert_bf16_to_i16_v_b(a, 0, rm, 1, 0)
#define v_convert_i32_to_bf16_v_vb(a, i, rm, l, p, o)  v_convert_i32_to_bf16_vb(a, l, (rm <<16), i, to_bool64(p), o)
#define v_convert_i32_to_bf16_v_b(a, i, rm, l, p, o)   v_convert_i32_to_bf16_b(a, l, (rm <<16), i, p, o)
#define v_convert_i32_to_bf16_v(a, i, rm, l)           v_convert_i32_to_bf16_v_b(a, i, rm, l, 1, 0)
#define v_convert_i16_to_bf16_v_vb(a, i, rm, p, o)     v_convert_i16_to_bf16_vb(a, (rm <<16), i, to_bool128(p), o)
#define v_convert_i16_to_bf16_v_b(a, i, rm, p, o)      v_convert_i16_to_bf16_b(a, (rm <<16), i, p, o)
#define v_convert_i16_to_bf16_v(a, rm)                 v_convert_i16_to_bf16_v_b(a, 0, rm, 1, 0)
#define v_convert_u16_to_bf16_v_vb(a, i, rm, p, o)     v_convert_u16_to_bf16_vb(a, (rm <<16), i, to_bool128(p), o)
#define v_convert_u16_to_bf16_v_b(a, i, rm, p, o)      v_convert_u16_to_bf16_b(a, (rm <<16), i, p, o)
#define v_convert_u16_to_bf16_v(a, rm)                 v_convert_u16_to_bf16_v_b(a, 0, rm, 1, 0)


#define v_convert_i16_to_u8_v_vb(a, i, l, p, o)        v_convert_i16_to_u8_vb(a, l, 0, i, to_bool128(p), o)
#define v_convert_i16_to_u8_v_b(a, i, l, p, o)         v_convert_i16_to_u8_b(a, l, 0, i, p, o)
#define v_convert_i16_to_u8_v(a, i, l)                 v_convert_i16_to_u8_v_b(a, i, l, 1, 0)


// MAC

#define s_f32_mac_s_s_b(x0, x1, acc, sw, pred, pol)       s_f32_mac(x0, x1, acc, (sw) << 1, pred, pol)
#define s_bf16_mac_s_s_b(x0, x1, acc, sw, pred, pol)      s_bf16_mac(x0, x1, acc, (sw) << 1, pred, pol)
#define s_i16_mac_s_s_b(x0, x1, acc, sw, pred, pol)       s_i16_mac(x0, x1, acc, sw, pred, pol)
#define s_u16_mac_s_s_b(x0, x1, acc, sw, pred, pol)       s_u16_mac(x0, x1, acc, sw, pred, pol)
#define s_i8_mac_s_s_b(x0, x1, acc, sw, pred, pol)        s_i8_mac(x0, x1, acc, sw, pred, pol)
#define s_u8_mac_s_s_b(x0, x1, acc, sw, pred, pol)        s_u8_mac(x0, x1, acc, sw, pred, pol)

#define s_f32_mac_s_s(x0, x1, acc, sw)                s_f32_mac_s_s_b(x0, x1, acc, sw, 1, 0)
#define s_i8_mac_s_s(x0, x1, acc, sw)                 s_i8_mac_s_s_b(x0, x1, acc, sw, 1, 0)
#define s_u8_mac_s_s(x0, x1, acc, sw)                 s_u8_mac_s_s_b(x0, x1, acc, sw, 1, 0)
#define s_i16_mac_s_s(x0, x1, acc, sw)                s_i16_mac_s_s_b(x0, x1, acc, sw, 1, 0)
#define s_u16_mac_s_s(x0, x1, acc, sw)                s_u16_mac_s_s_b(x0, x1, acc, sw, 1, 0)
#define s_bf16_mac_s_s(x0, x1, acc, sw)               s_bf16_mac_s_s_b(x0, x1, acc, sw, 1, 0)

#define s_bf16_mac_acc_s_s_b(x0, x1, acc, sw, pred, pol)  s_bf16_mac_acc32(x0, x1, acc, (sw) << 1, pred, pol)
#define s_i8_mac_acc_s_s_b(x0, x1, acc, sw, p, pol)       s_i8_mac_acc16(x0, x1, acc, sw, p, pol)
#define s_u8_mac_acc_s_s_b(x0, x1, acc, sw, p, pol)       s_u8_mac_acc16(x0, x1, acc, sw, p, pol)

#define s_bf16_mac_acc_s_s(x0, x1, acc, sw)               s_bf16_mac_acc_s_s_b(x0, x1, acc, sw, 1, 0)
#define s_i8_mac_acc_s_s(x0, x1, acc, sw)                 s_i8_mac_acc_s_s_b(x0, x1, acc, sw, 1, 0)
#define s_u8_mac_acc_s_s(x0, x1, acc, sw)                 s_u8_mac_acc_s_s_b(x0, x1, acc, sw, 1, 0)

#define v_f32_mac_v_v_vb(a, b, acc, neg, p, pol)      v_f32_mac_vb(a, b, acc, (neg) << 1, to_bool64(p), pol)
#define v_bf16_mac_v_v_vb(a, b, acc, neg, p, pol)     v_bf16_mac_vb(a, b, acc, (neg) << 1, to_bool128(p), pol)
#define av_i16_mac_v_v_vb(a, b, acc, sat, p, pol)     v_i16_mac_vb(a, b, acc, sat, to_bool128(p), pol)
#define av_u16_mac_v_v_vb(a, b, acc, sat, p, pol)     v_u16_mac_vb(a, b, acc, sat, to_bool128(p), pol)
#define av_i8_mac_v_v_vb(a, b, acc, sat, p, pol)      v_i8_mac_vb(a, b, acc, sat, p, pol)
#define av_u8_mac_v_v_vb(a, b, acc, sat, p, pol)      v_u8_mac_vb(a, b, acc, sat, p, pol)

#define v_f32_mac_v_v_b(a, b, acc, neg, p, pol)       v_f32_mac_b(a, b, acc, (neg) << 1, p, pol)
#define v_bf16_mac_v_v_b(a, b, acc, neg, p, pol)      v_bf16_mac_b(a, b, acc, (neg) << 1, p, pol)
#define av_i16_mac_v_v_b(a, b, acc, sat, p, pol)      v_i16_mac_b(a, b, acc, sat, p, pol)
#define av_u16_mac_v_v_b(a, b, acc, sat, p, pol)      v_u16_mac_b(a, b, acc, sat, p, pol)
#define av_i8_mac_v_v_b(a, b, acc, sat, p, pol)       v_i8_mac_b(a, b, acc, sat, p, pol)
#define av_u8_mac_v_v_b(a, b, acc, sat, p, pol)       v_u8_mac_b(a, b, acc, sat, p, pol)

#define v_f32_mac_v_s_vb(a, b, acc, neg, p, pol)      v_f32_mac_v_v_vb(a, b, acc, neg, p, pol)
#define v_bf16_mac_v_s_vb(a, b, acc, neg, p, pol)     v_bf16_mac_v_v_vb(a, b, acc, neg, p, pol)
#define av_i16_mac_v_s_vb(a, b, acc, sat, p, pol)     av_i16_mac_v_v_vb(a, b, acc, sat, p, pol)
#define av_u16_mac_v_s_vb(a, b, acc, sat, p, pol)     av_u16_mac_v_v_vb(a, b, acc, sat, p, pol)
#define av_i8_mac_v_s_vb(a, b, acc, sat, p, pol)      av_i8_mac_v_v_vb(a, b, acc, sat, p, pol)
#define av_u8_mac_v_s_vb(a, b, acc, sat, p, pol)      av_u8_mac_v_v_vb(a, b, acc, sat, p, pol)

#define v_f32_mac_v_s_b(a, b, acc, neg, p, pol)       v_f32_mac_v_v_b(a, b, acc, neg, p, pol)
#define v_bf16_mac_v_s_b(a, b, acc, neg, p, pol)      v_bf16_mac_v_v_b(a, b, acc, neg, p, pol)
#define av_i16_mac_v_s_b(a, b, acc, sat, p, pol)      av_i16_mac_v_v_b(a, b, acc, sat, p, pol)
#define av_u16_mac_v_s_b(a, b, acc, sat, p, pol)      av_u16_mac_v_v_b(a, b, acc, sat, p, pol)
#define av_i8_mac_v_s_b(a, b, acc, sat, p, pol)       av_i8_mac_v_v_b(a, b, acc, sat, p, pol)
#define av_u8_mac_v_s_b(a, b, acc, sat, p, pol)       av_u8_mac_v_v_b(a, b, acc, sat, p, pol)

#define v_f32_mac_v_v(a, b, acc, neg)                 v_f32_mac_v_v_b(a, b, acc, neg, 1, 0)
#define v_bf16_mac_v_v(a, b, acc, neg)                v_bf16_mac_v_v_b(a, b, acc, neg, 1, 0)
#define av_i16_mac_v_v(a, b, acc, sat)                av_i16_mac_v_v_b(a, b, acc, sat, 1, 0)
#define av_u16_mac_v_v(a, b, acc, sat)                av_u16_mac_v_v_b(a, b, acc, sat, 1, 0)
#define av_i8_mac_v_v(a, b, acc, sat)                 av_i8_mac_v_v_b(a, b, acc, sat, 1, 0)
#define av_u8_mac_v_v(a, b, acc, sat)                 av_u8_mac_v_v_b(a, b, acc, sat, 1, 0)

#define v_f32_mac_v_s(a, b, acc, neg)                 v_f32_mac_v_v(a, b, acc, neg)
#define v_bf16_mac_v_s(a, b, acc, neg)                v_bf16_mac_v_v(a, b, acc, neg)
#define av_i16_mac_v_s(a, b, acc, sat)                av_i16_mac_v_v(a, b, acc, sat)
#define av_u16_mac_v_s(a, b, acc, sat)                av_u16_mac_v_v(a, b, acc, sat)
#define av_i8_mac_v_s(a, b, acc, sat)                 av_i8_mac_v_v(a, b, acc, sat)
#define av_u8_mac_v_s(a, b, acc, sat)                 av_u8_mac_v_v(a, b, acc, sat)

#define av_bf16_mac_acc_v_v_vb(a, b, acc, neg, p, pol) v_bf16_mac_acc32_vb(a, b, acc, (neg) << 1, to_bool128(p), pol)
#define av_bf16_mac_acc_v_v_b(a, b, acc, neg, p, pol)  v_bf16_mac_acc32_b(a, b, acc, (neg) << 1, p, pol)
#define av_bf16_mac_acc_v_s_vb(a, b, acc, neg, p, pol) av_bf16_mac_acc_v_v_vb(a, b, acc, neg, p, pol)
#define av_bf16_mac_acc_v_s_b(a, b, acc, neg, p, pol)  av_bf16_mac_acc_v_v_b(a, b, acc, neg, p, pol)
#define av_bf16_mac_acc_v_v(a, b, acc, neg)            av_bf16_mac_acc_v_v_b(a, b, acc, neg, 1, 0)
#define av_bf16_mac_acc_v_s(a, b, acc, neg)            av_bf16_mac_acc_v_v(a, b, acc, neg)


// MUL
// variant for f8/h8 w/o acc is absent in hardware see 2.3.2
#define s_f32_mul_s_s_b(a, b, i, p, o)              s_f32_mul(a, b, 0, i, p, o)
#define s_bf16_mul_s_s_b(a, b, i, p, o)             s_bf16_mul(a, b, 0, i, p, o)
#define s_i32_mul_s_s_b(a, b, i, s, p, o)           s_i32_mul(a, b, s << 2, i, p, o)
#define s_u32_mul_s_s_b(a, b, i, s, p, o)           s_u32_mul(a, b, s << 2, i, p, o)
#define s_i16_mul_s_s_b(a, b, i, p, o)              s_i16_mul(a, b, 0, i, p, o)
#define s_u16_mul_s_s_b(a, b, i, p, o)              s_u16_mul(a, b, 0, i, p, o)
#define s_i8_mul_s_s_b(a, b, i, p, o)               s_i8_mul(a, b, 0, i, p, o)
#define s_u8_mul_s_s_b(a, b, i, p, o)               s_u8_mul(a, b, 0, i, p, o)

#define s_f32_mul_s_s(a, b)                         s_f32_mul_s_s_b(a, b, 0, 1, 0)
#define s_bf16_mul_s_s(a, b)                        s_bf16_mul_s_s_b(a, b, 0, 1, 0)
#define s_i32_mul_s_s(a, b, s)                      s_i32_mul_s_s_b(a, b, 0, s, 1, 0)
#define s_u32_mul_s_s(a, b, s)                      s_u32_mul_s_s_b(a, b, 0, s, 1, 0)
#define s_i16_mul_s_s(a, b)                         s_i16_mul_s_s_b(a, b, 0, 1, 0)
#define s_u16_mul_s_s(a, b)                         s_u16_mul_s_s_b(a, b, 0, 1, 0)
#define s_i8_mul_s_s(a, b)                          s_i8_mul_s_s_b(a, b, 0, 1, 0)
#define s_u8_mul_s_s(a, b)                          s_u8_mul_s_s_b(a, b, 0, 1, 0)

#define s_bf16_mul_acc_s_s_b(a, b, i, p, o)         s_bf16_mul_acc32(a, b, 0, i, p, o)
#define s_bf16_mul_acc_s_s(a, b)                    s_bf16_mul_acc_s_s_b(a, b, 0, 1, 0)

#define i_i32_mul_i_i_b(a, b, i, m, p, o)           i_i32_mul(a, b, m, 0, i, p, o)
#define i_i32_mul_s_i_b(a, b, i, m, p, o)           i_i32_mul_i_i_b(a, b, i, m, p, o)
#define i_i32_mul_i_i(a, b, i, m)                   i_i32_mul_i_i_b(a, b, i, m, 1, 0)
#define i_i32_mul_s_i(a, b, i, m)                   i_i32_mul_i_i(a, b, i, m)

#define v_f32_mul_v_v_vb(a, b, i, p, o)             v_f32_mul_vb(a, b, 0, i, to_bool64(p), o)
#define v_bf16_mul_v_v_vb(a, b, i, p, o)            v_bf16_mul_vb(a, b, 0, i, to_bool128(p), o)

#define av_i32_mul_v_v_vb(a, b, i, p, o)            v_i32_mul_vb(a, b, 0, i, to_bool64(p), o)
#define av_u32_mul_v_v_vb(a, b, i, p, o)            v_u32_mul_vb(a, b, 0, i, to_bool64(p), o)
#define av_i16_mul_v_v_vb(a, b, i, p, o)            v_i16_mul_vb(a, b, 0, i, to_bool128(p), o)
#define av_u16_mul_v_v_vb(a, b, i, p, o)            v_u16_mul_vb(a, b, 0, i, to_bool128(p), o)
#define v_i8_mul_v_v_vb(a, b, i, p, o)              v_i8_mul_vb(a, b, 0, i, p, o)
#define v_u8_mul_v_v_vb(a, b, i, p, o)              v_u8_mul_vb(a, b, 0, i, p, o)

#define v_f32_mul_v_v_b(a, b, i, p, o)              v_f32_mul_b(a, b, 0, i, p, o)
#define v_bf16_mul_v_v_b(a, b, i, p, o)             v_bf16_mul_b(a, b, 0, i, p, o)

#define av_i32_mul_v_v_b(a, b, i, p, o)             v_i32_mul_b(a, b, 0, i, p, o)
#define av_u32_mul_v_v_b(a, b, i, p, o)             v_u32_mul_b(a, b, 0, i, p, o)
#define av_i16_mul_v_v_b(a, b, i, p, o)             v_i16_mul_b(a, b, 0, i, p, o)
#define av_u16_mul_v_v_b(a, b, i, p, o)             v_u16_mul_b(a, b, 0, i, p, o)
#define v_i8_mul_v_v_b(a, b, i, p, o)               v_i8_mul_b(a, b, 0, i, p, o)
#define v_u8_mul_v_v_b(a, b, i, p, o)               v_u8_mul_b(a, b, 0, i, p, o)

#define v_f32_mul_v_s_vb(a, b, i, p, o)             v_f32_mul_v_v_vb(a, b, i, p, o)
#define v_bf16_mul_v_s_vb(a, b, i, p, o)            v_bf16_mul_v_v_vb(a, b, i, p, o)
#define av_i32_mul_v_s_vb(a, b, i, p, o)            av_i32_mul_v_v_vb(a, b, i, p, o)
#define av_u32_mul_v_s_vb(a, b, i, p, o)            av_u32_mul_v_v_vb(a, b, i, p, o)
#define av_i16_mul_v_s_vb(a, b, i, p, o)            av_i16_mul_v_v_vb(a, b, i, p, o)
#define av_u16_mul_v_s_vb(a, b, i, p, o)            av_u16_mul_v_v_vb(a, b, i, p, o)
#define v_i8_mul_v_s_vb(a, b, i, p, o)              v_i8_mul_v_v_vb(a, b, i, p, o)
#define v_u8_mul_v_s_vb(a, b, i, p, o)              v_u8_mul_v_v_vb(a, b, i, p, o)

#define v_f32_mul_v_s_b(a, b, i, p, o)              v_f32_mul_v_v_b(a, b, i, p, o)
#define v_bf16_mul_v_s_b(a, b, i, p, o)             v_bf16_mul_v_v_b(a, b, i, p, o)
#define av_i32_mul_v_s_b(a, b, i, p, o)             av_i32_mul_v_v_b(a, b, i, p, o)
#define av_u32_mul_v_s_b(a, b, i, p, o)             av_u32_mul_v_v_b(a, b, i, p, o)
#define av_i16_mul_v_s_b(a, b, i, p, o)             av_i16_mul_v_v_b(a, b, i, p, o)
#define av_u16_mul_v_s_b(a, b, i, p, o)             av_u16_mul_v_v_b(a, b, i, p, o)
#define v_i8_mul_v_s_b(a, b, i, p, o)               v_i8_mul_v_v_b(a, b, i, p, o)
#define v_u8_mul_v_s_b(a, b, i, p, o)               v_u8_mul_v_v_b(a, b, i, p, o)

#define v_f32_mul_v_v(a, b)                         v_f32_mul_v_v_b(a, b, 0, 1, 0)
#define v_bf16_mul_v_v(a, b)                        v_bf16_mul_v_v_b(a, b, 0, 1, 0)
#define av_i32_mul_v_v(a, b)                        av_i32_mul_v_v_b(a, b, (int128){0}, 1, 0)
#define av_u32_mul_v_v(a, b)                        av_u32_mul_v_v_b(a, b, (uint128){0}, 1, 0)
#define av_i16_mul_v_v(a, b)                        av_i16_mul_v_v_b(a, b, (int128){0}, 1, 0)
#define av_u16_mul_v_v(a, b)                        av_u16_mul_v_v_b(a, b, (uint128){0}, 1, 0)
#define v_i8_mul_v_v(a, b)                          v_i8_mul_v_v_b(a, b, (int256){ 0 }, 1, 0)
#define v_u8_mul_v_v(a, b)                          v_u8_mul_v_v_b(a, b, (uint256){ 0 }, 1, 0)

#define v_f32_mul_v_s(a, b)                         v_f32_mul_v_v(a, b)
#define v_bf16_mul_v_s(a, b)                        v_bf16_mul_v_v(a, b)
#define av_i32_mul_v_s(a, b)                        av_i32_mul_v_v(a, b)
#define av_u32_mul_v_s(a, b)                        av_u32_mul_v_v(a, b)
#define av_i16_mul_v_s(a, b)                        av_i16_mul_v_v(a, b)
#define av_u16_mul_v_s(a, b)                        av_u16_mul_v_v(a, b)
#define v_i8_mul_v_s(a, b)                          v_i8_mul_v_v(a, b)
#define v_u8_mul_v_s(a, b)                          v_u8_mul_v_v(a, b)

#define av_bf16_mul_acc_v_v_vb(a, b, i, p, o)       v_bf16_mul_acc32_vb(a, b, 0, i, to_bool128(p), o)
#define av_bf16_mul_acc_v_v_b(a, b, i, p, o)        v_bf16_mul_acc32_b(a, b, 0, i, p, o)
#define av_bf16_mul_acc_v_s_vb(a, b, i, p, o)       av_bf16_mul_acc_v_v_vb(a, b, i, p, o)
#define av_bf16_mul_acc_v_s_b(a, b, i, p, o)        av_bf16_mul_acc_v_v_b(a, b, i, p, o)
#define av_bf16_mul_acc_v_v(a, b)                   av_bf16_mul_acc_v_v_b(a, b, (float128){0}, 1, 0)
#define av_bf16_mul_acc_v_s(a, b)                   av_bf16_mul_acc_v_v(a, b)


#define v_i32_mul_double_and_round_v_v_vb(a, b, i, p, o)  v_i32_mul_round_vb(a, b, 0, i, to_bool64(p), o)
#define v_i32_mul_double_and_round_v_v_b(a, b, i, p, o)   v_i32_mul_round_b(a, b, 0, i, p, o)
#define v_i32_mul_double_and_round_v_s_vb(a, b, i, p, o)  v_i32_mul_double_and_round_v_v_vb(a, b, i, p, o)
#define v_i32_mul_double_and_round_v_s_b(a, b, i, p, o)   v_i32_mul_double_and_round_v_v_b(a, b, i, p, o)
#define v_i32_mul_double_and_round_v_v(a, b)              v_i32_mul_double_and_round_v_v_b(a, b, 0, 1, 0)
#define v_i32_mul_double_and_round_v_s(a, b)              v_i32_mul_double_and_round_v_v(a, b)

#define v_u32_mul_double_and_round_v_v_vb(a, b, i, p, o)  v_u32_mul_round_vb(a, b, 0, i, to_bool64(p), o)
#define v_u32_mul_double_and_round_v_v_b(a, b, i, p, o)   v_u32_mul_round_b(a, b, 0, i, p, o)
#define v_u32_mul_double_and_round_v_s_vb(a, b, i, p, o)  v_u32_mul_double_and_round_v_v_vb(a, b, i, p, o)
#define v_u32_mul_double_and_round_v_s_b(a, b, i, p, o)   v_u32_mul_double_and_round_v_v_b(a, b, i, p, o)
#define v_u32_mul_double_and_round_v_v(a, b)              v_u32_mul_double_and_round_v_v_b(a, b, 0, 1, 0)
#define v_u32_mul_double_and_round_v_s(a, b)              v_u32_mul_double_and_round_v_v(a, b)

// ADD

#define s_f32_add_s_s_b(a, b, i, p, o)              s_f32_add(a, b, 0, i, p, o)
#define s_bf16_add_s_s_b(a, b, i, p, o)             s_bf16_add(a, b, 0, i, p, o)
#define s_i32_add_s_s_b(a, b, i, s, p, o)           s_i32_add(a, b, s, i, p, o)
#define s_u32_add_s_s_b(a, b, i, s, p, o)           s_u32_add(a, b, s, i, p, o)
#define s_i16_add_s_s_b(a, b, i, s, p, o)           s_i16_add(a, b, s, i, p, o)
#define s_u16_add_s_s_b(a, b, i, s, p, o)           s_u16_add(a, b, s, i, p, o)
#define s_i8_add_s_s_b(a, b, i, s, p, o)            s_i8_add(a, b, s, i, p, o)
#define s_u8_add_s_s_b(a, b, i, s, p, o)            s_u8_add(a, b, s, i, p, o)

#define s_f32_add_s_s                               s_f32_add
#define s_bf16_add_s_s                              s_bf16_add
#define s_i32_add_s_s                               s_i32_add
#define s_u32_add_s_s                               s_u32_add
#define s_i16_add_s_s                               s_i16_add
#define s_u16_add_s_s                               s_u16_add
#define s_i8_add_s_s                                s_i8_add
#define s_u8_add_s_s                                s_u8_add

#define i_i32_add_i_i_b(a, b, i, m, p, o)           i_i32_add(a, b, m, 0, i, p, o)
#define i_i32_add_s_i_b(a, b, i, m, p, o)           i_i32_add_i_i_b(a, b, i, m, p, o)
#define i_i32_add_i_i(a, b, i, m)                   i_i32_add_i_i_b(a, b, i, m, 1, 0)
#define i_i32_add_s_i(a, b, i, m)                   i_i32_add_i_i(a, b, i, m)

#define v_f32_add_v_v_vb(a, b, i, p, o)             v_f32_add_vb(a, b, 0, i, to_bool64(p), o)
#define v_bf16_add_v_v_vb(a, b, i, p, o)            v_bf16_add_vb(a, b, 0, i, to_bool128(p), o)
#define v_i32_add_v_v_vb(a, b, i, s, p, o)          v_i32_add_vb(a, b, s, i, to_bool64(p), o)
#define v_u32_add_v_v_vb(a, b, i, s, p, o)          v_u32_add_vb(a, b, s, i, to_bool64(p), o)
#define v_i16_add_v_v_vb(a, b, i, s, p, o)          v_i16_add_vb(a, b, s, i, to_bool128(p), o)
#define v_u16_add_v_v_vb(a, b, i, s, p, o)          v_u16_add_vb(a, b, s, i, to_bool128(p), o)
#define v_i8_add_v_v_vb(a, b, i, s, p, o)           v_i8_add_vb(a, b, s, i, p, o)
#define v_u8_add_v_v_vb(a, b, i, s, p, o)           v_u8_add_vb(a, b, s, i, p, o)

#define v_f32_add_v_v_b(a, b, i, p, o)              v_f32_add_b(a, b, 0, i, p, o)
#define v_bf16_add_v_v_b(a, b, i, p, o)             v_bf16_add_b(a, b, 0, i, p, o)
#define v_i32_add_v_v_b(a, b, i, s, p, o)           v_i32_add_b(a, b, s, i, p, o)
#define v_u32_add_v_v_b(a, b, i, s, p, o)           v_u32_add_b(a, b, s, i, p, o)
#define v_i16_add_v_v_b(a, b, i, s, p, o)           v_i16_add_b(a, b, s, i, p, o)
#define v_u16_add_v_v_b(a, b, i, s, p, o)           v_u16_add_b(a, b, s, i, p, o)
#define v_i8_add_v_v_b(a, b, i, s, p, o)            v_i8_add_b(a, b, s, i, p, o)
#define v_u8_add_v_v_b(a, b, i, s, p, o)            v_u8_add_b(a, b, s, i, p, o)

#define v_f32_add_v_s_vb                            v_f32_add_v_v_vb
#define v_bf16_add_v_s_vb                           v_bf16_add_v_v_vb
#define v_i32_add_v_s_vb                            v_i32_add_v_v_vb
#define v_u32_add_v_s_vb                            v_u32_add_v_v_vb
#define v_i16_add_v_s_vb                            v_i16_add_v_v_vb
#define v_u16_add_v_s_vb                            v_u16_add_v_v_vb
#define v_i8_add_v_s_vb                             v_i8_add_v_v_vb
#define v_u8_add_v_s_vb                             v_u8_add_v_v_vb

#define v_f32_add_v_s_b                             v_f32_add_v_v_b
#define v_bf16_add_v_s_b                            v_bf16_add_v_v_b
#define v_i32_add_v_s_b                             v_i32_add_v_v_b
#define v_u32_add_v_s_b                             v_u32_add_v_v_b
#define v_i16_add_v_s_b                             v_i16_add_v_v_b
#define v_u16_add_v_s_b                             v_u16_add_v_v_b
#define v_i8_add_v_s_b                              v_i8_add_v_v_b
#define v_u8_add_v_s_b                              v_u8_add_v_v_b

#define v_f32_add_v_v                               v_f32_add_b
#define v_bf16_add_v_v                              v_bf16_add_b
#define v_i32_add_v_v                               v_i32_add_b
#define v_u32_add_v_v                               v_u32_add_b
#define v_i16_add_v_v                               v_i16_add_b
#define v_u16_add_v_v                               v_u16_add_b
#define v_i8_add_v_v                                v_i8_add_b
#define v_u8_add_v_v                                v_u8_add_b

#define v_f32_add_v_s                               v_f32_add_v_v
#define v_bf16_add_v_s                              v_bf16_add_v_v
#define v_i32_add_v_s                               v_i32_add_v_v
#define v_u32_add_v_s                               v_u32_add_v_v
#define v_i16_add_v_s                               v_i16_add_v_v
#define v_u16_add_v_s                               v_u16_add_v_v
#define v_i8_add_v_s                                v_i8_add_v_v
#define v_u8_add_v_s                                v_u8_add_v_v

// SEL_EQ

// f32 - f32
#define v_f32_f32_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_eq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_f32_sel_eq_v_s_v_v_b                          v_f32_f32_sel_eq_v_v_v_v_b
#define v_f32_f32_sel_eq_v_v_v_s_b                          v_f32_f32_sel_eq_v_v_v_v_b
#define v_f32_f32_sel_eq_v_v_v_v(a, b, c, d)                v_f32_f32_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_f32_sel_eq_v_s_v_v                            v_f32_f32_sel_eq_v_v_v_v
#define v_f32_f32_sel_eq_v_v_v_s                            v_f32_f32_sel_eq_v_v_v_v

#define v_f32_f32_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_eq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_f32_sel_eq_v_s_v_v_vb                         v_f32_f32_sel_eq_v_v_v_v_vb
#define v_f32_f32_sel_eq_v_v_v_s_vb                         v_f32_f32_sel_eq_v_v_v_v_vb

// f32 - i32
#define v_i32_f32_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_eq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_f32_sel_eq_v_s_v_v_b                          v_i32_f32_sel_eq_v_v_v_v_b
#define v_i32_f32_sel_eq_v_v_v_s_b                          v_i32_f32_sel_eq_v_v_v_v_b
#define v_i32_f32_sel_eq_v_v_v_v(a, b, c, d)                v_i32_f32_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_f32_sel_eq_v_s_v_v                            v_i32_f32_sel_eq_v_v_v_v
#define v_i32_f32_sel_eq_v_v_v_s                            v_i32_f32_sel_eq_v_v_v_v

#define v_i32_f32_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_eq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_f32_sel_eq_v_s_v_v_vb                         v_i32_f32_sel_eq_v_v_v_v_vb
#define v_i32_f32_sel_eq_v_v_v_s_vb                         v_i32_f32_sel_eq_v_v_v_v_vb

// f32 - u32
#define v_u32_f32_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_eq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_f32_sel_eq_v_s_v_v_b                          v_u32_f32_sel_eq_v_v_v_v_b
#define v_u32_f32_sel_eq_v_v_v_s_b                          v_u32_f32_sel_eq_v_v_v_v_b
#define v_u32_f32_sel_eq_v_v_v_v(a, b, c, d)                v_u32_f32_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_f32_sel_eq_v_s_v_v                            v_u32_f32_sel_eq_v_v_v_v
#define v_u32_f32_sel_eq_v_v_v_s                            v_u32_f32_sel_eq_v_v_v_v

#define v_u32_f32_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_eq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_f32_sel_eq_v_s_v_v_vb                         v_u32_f32_sel_eq_v_v_v_v_vb
#define v_u32_f32_sel_eq_v_v_v_s_vb                         v_u32_f32_sel_eq_v_v_v_v_vb

// bf16 - bf16
#define v_bf16_bf16_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)   v_bf16_sel_eq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_bf16_sel_eq_v_s_v_v_b                        v_bf16_bf16_sel_eq_v_v_v_v_b
#define v_bf16_bf16_sel_eq_v_v_v_s_b                        v_bf16_bf16_sel_eq_v_v_v_v_b
#define v_bf16_bf16_sel_eq_v_v_v_v(a, b, c, d)              v_bf16_bf16_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_bf16_sel_eq_v_s_v_v                          v_bf16_bf16_sel_eq_v_v_v_v
#define v_bf16_bf16_sel_eq_v_v_v_s                          v_bf16_bf16_sel_eq_v_v_v_v

#define v_bf16_bf16_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)  v_bf16_sel_eq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_bf16_sel_eq_v_s_v_v_vb                       v_bf16_bf16_sel_eq_v_v_v_v_vb
#define v_bf16_bf16_sel_eq_v_v_v_s_vb                       v_bf16_bf16_sel_eq_v_v_v_v_vb

// bf16 - i16
#define v_i16_bf16_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel_eq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_bf16_sel_eq_v_s_v_v_b                         v_i16_bf16_sel_eq_v_v_v_v_b
#define v_i16_bf16_sel_eq_v_v_v_s_b                         v_i16_bf16_sel_eq_v_v_v_v_b
#define v_i16_bf16_sel_eq_v_v_v_v(a, b, c, d)               v_i16_bf16_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_bf16_sel_eq_v_s_v_v                           v_i16_bf16_sel_eq_v_v_v_v
#define v_i16_bf16_sel_eq_v_v_v_s                           v_i16_bf16_sel_eq_v_v_v_v

#define v_i16_bf16_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel_eq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_bf16_sel_eq_v_s_v_v_vb                        v_i16_bf16_sel_eq_v_v_v_v_vb
#define v_i16_bf16_sel_eq_v_v_v_s_vb                        v_i16_bf16_sel_eq_v_v_v_v_vb

// bf16 - u16
#define v_u16_bf16_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel_eq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_bf16_sel_eq_v_s_v_v_b                         v_u16_bf16_sel_eq_v_v_v_v_b
#define v_u16_bf16_sel_eq_v_v_v_s_b                         v_u16_bf16_sel_eq_v_v_v_v_b
#define v_u16_bf16_sel_eq_v_v_v_v(a, b, c, d)               v_u16_bf16_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_bf16_sel_eq_v_s_v_v                           v_u16_bf16_sel_eq_v_v_v_v
#define v_u16_bf16_sel_eq_v_v_v_s                           v_u16_bf16_sel_eq_v_v_v_v

#define v_u16_bf16_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel_eq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_bf16_sel_eq_v_s_v_v_vb                        v_u16_bf16_sel_eq_v_v_v_v_vb
#define v_u16_bf16_sel_eq_v_v_v_s_vb                        v_u16_bf16_sel_eq_v_v_v_v_vb

// i32 - f32
#define v_f32_i32_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_eq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_i32_sel_eq_v_s_v_v_b                          v_f32_i32_sel_eq_v_v_v_v_b
#define v_f32_i32_sel_eq_v_v_v_s_b                          v_f32_i32_sel_eq_v_v_v_v_b
#define v_f32_i32_sel_eq_v_v_v_v(a, b, c, d)                v_f32_i32_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_i32_sel_eq_v_s_v_v                            v_f32_i32_sel_eq_v_v_v_v
#define v_f32_i32_sel_eq_v_v_v_s                            v_f32_i32_sel_eq_v_v_v_v

#define v_f32_i32_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_eq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_i32_sel_eq_v_s_v_v_vb                         v_f32_i32_sel_eq_v_v_v_v_vb
#define v_f32_i32_sel_eq_v_v_v_s_vb                         v_f32_i32_sel_eq_v_v_v_v_vb

// i32 - i32
#define v_i32_i32_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_eq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_i32_sel_eq_v_s_v_v_b                          v_i32_i32_sel_eq_v_v_v_v_b
#define v_i32_i32_sel_eq_v_v_v_s_b                          v_i32_i32_sel_eq_v_v_v_v_b
#define v_i32_i32_sel_eq_v_v_v_v(a, b, c, d)                v_i32_i32_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_i32_sel_eq_v_s_v_v                            v_i32_i32_sel_eq_v_v_v_v
#define v_i32_i32_sel_eq_v_v_v_s                            v_i32_i32_sel_eq_v_v_v_v

#define v_i32_i32_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_eq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_i32_sel_eq_v_s_v_v_vb                         v_i32_i32_sel_eq_v_v_v_v_vb
#define v_i32_i32_sel_eq_v_v_v_s_vb                         v_i32_i32_sel_eq_v_v_v_v_vb

// i32 - u32
#define v_u32_i32_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_eq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_i32_sel_eq_v_s_v_v_b                          v_u32_i32_sel_eq_v_v_v_v_b
#define v_u32_i32_sel_eq_v_v_v_s_b                          v_u32_i32_sel_eq_v_v_v_v_b
#define v_u32_i32_sel_eq_v_v_v_v(a, b, c, d)                v_u32_i32_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_i32_sel_eq_v_s_v_v                            v_u32_i32_sel_eq_v_v_v_v
#define v_u32_i32_sel_eq_v_v_v_s                            v_u32_i32_sel_eq_v_v_v_v

#define v_u32_i32_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_eq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_i32_sel_eq_v_s_v_v_vb                         v_u32_i32_sel_eq_v_v_v_v_vb
#define v_u32_i32_sel_eq_v_v_v_s_vb                         v_u32_i32_sel_eq_v_v_v_v_vb

// u32 - f32
#define v_f32_u32_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_eq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_u32_sel_eq_v_s_v_v_b                          v_f32_u32_sel_eq_v_v_v_v_b
#define v_f32_u32_sel_eq_v_v_v_s_b                          v_f32_u32_sel_eq_v_v_v_v_b
#define v_f32_u32_sel_eq_v_v_v_v(a, b, c, d)                v_f32_u32_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_u32_sel_eq_v_s_v_v                            v_f32_u32_sel_eq_v_v_v_v
#define v_f32_u32_sel_eq_v_v_v_s                            v_f32_u32_sel_eq_v_v_v_v

#define v_f32_u32_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_eq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_u32_sel_eq_v_s_v_v_vb                         v_f32_u32_sel_eq_v_v_v_v_vb
#define v_f32_u32_sel_eq_v_v_v_s_vb                         v_f32_u32_sel_eq_v_v_v_v_vb

// u32 - i32
#define v_i32_u32_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_eq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_u32_sel_eq_v_s_v_v_b                          v_i32_u32_sel_eq_v_v_v_v_b
#define v_i32_u32_sel_eq_v_v_v_s_b                          v_i32_u32_sel_eq_v_v_v_v_b
#define v_i32_u32_sel_eq_v_v_v_v(a, b, c, d)                v_i32_u32_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_u32_sel_eq_v_s_v_v                            v_i32_u32_sel_eq_v_v_v_v
#define v_i32_u32_sel_eq_v_v_v_s                            v_i32_u32_sel_eq_v_v_v_v

#define v_i32_u32_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_eq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_u32_sel_eq_v_s_v_v_vb                         v_i32_u32_sel_eq_v_v_v_v_vb
#define v_i32_u32_sel_eq_v_v_v_s_vb                         v_i32_u32_sel_eq_v_v_v_v_vb

// u32 - u32
#define v_u32_u32_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_eq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_u32_sel_eq_v_s_v_v_b                          v_u32_u32_sel_eq_v_v_v_v_b
#define v_u32_u32_sel_eq_v_v_v_s_b                          v_u32_u32_sel_eq_v_v_v_v_b
#define v_u32_u32_sel_eq_v_v_v_v(a, b, c, d)                v_u32_u32_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_u32_sel_eq_v_s_v_v                            v_u32_u32_sel_eq_v_v_v_v
#define v_u32_u32_sel_eq_v_v_v_s                            v_u32_u32_sel_eq_v_v_v_v

#define v_u32_u32_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_eq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_u32_sel_eq_v_s_v_v_vb                         v_u32_u32_sel_eq_v_v_v_v_vb
#define v_u32_u32_sel_eq_v_v_v_s_vb                         v_u32_u32_sel_eq_v_v_v_v_vb

// i16 - bf16
#define v_bf16_i16_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel_eq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_i16_sel_eq_v_s_v_v_b                         v_bf16_i16_sel_eq_v_v_v_v_b
#define v_bf16_i16_sel_eq_v_v_v_s_b                         v_bf16_i16_sel_eq_v_v_v_v_b
#define v_bf16_i16_sel_eq_v_v_v_v(a, b, c, d)               v_bf16_i16_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_i16_sel_eq_v_s_v_v                           v_bf16_i16_sel_eq_v_v_v_v
#define v_bf16_i16_sel_eq_v_v_v_s                           v_bf16_i16_sel_eq_v_v_v_v

#define v_bf16_i16_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel_eq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_i16_sel_eq_v_s_v_v_vb                        v_bf16_i16_sel_eq_v_v_v_v_vb
#define v_bf16_i16_sel_eq_v_v_v_s_vb                        v_bf16_i16_sel_eq_v_v_v_v_vb

// i16 - i16
#define v_i16_i16_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i16_sel_eq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_i16_sel_eq_v_s_v_v_b                          v_i16_i16_sel_eq_v_v_v_v_b
#define v_i16_i16_sel_eq_v_v_v_s_b                          v_i16_i16_sel_eq_v_v_v_v_b
#define v_i16_i16_sel_eq_v_v_v_v(a, b, c, d)                v_i16_i16_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_i16_sel_eq_v_s_v_v                            v_i16_i16_sel_eq_v_v_v_v
#define v_i16_i16_sel_eq_v_v_v_s                            v_i16_i16_sel_eq_v_v_v_v

#define v_i16_i16_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i16_sel_eq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_i16_sel_eq_v_s_v_v_vb                         v_i16_i16_sel_eq_v_v_v_v_vb
#define v_i16_i16_sel_eq_v_v_v_s_vb                         v_i16_i16_sel_eq_v_v_v_v_vb

// i16 - u16
#define v_u16_i16_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i16_sel_eq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_i16_sel_eq_v_s_v_v_b                          v_u16_i16_sel_eq_v_v_v_v_b
#define v_u16_i16_sel_eq_v_v_v_s_b                          v_u16_i16_sel_eq_v_v_v_v_b
#define v_u16_i16_sel_eq_v_v_v_v(a, b, c, d)                v_u16_i16_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_i16_sel_eq_v_s_v_v                            v_u16_i16_sel_eq_v_v_v_v
#define v_u16_i16_sel_eq_v_v_v_s                            v_u16_i16_sel_eq_v_v_v_v

#define v_u16_i16_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i16_sel_eq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_i16_sel_eq_v_s_v_v_vb                         v_u16_i16_sel_eq_v_v_v_v_vb
#define v_u16_i16_sel_eq_v_v_v_s_vb                         v_u16_i16_sel_eq_v_v_v_v_vb

// u16 - bf16
#define v_bf16_u16_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel_eq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_u16_sel_eq_v_s_v_v_b                         v_bf16_u16_sel_eq_v_v_v_v_b
#define v_bf16_u16_sel_eq_v_v_v_s_b                         v_bf16_u16_sel_eq_v_v_v_v_b
#define v_bf16_u16_sel_eq_v_v_v_v(a, b, c, d)               v_bf16_u16_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_u16_sel_eq_v_s_v_v                           v_bf16_u16_sel_eq_v_v_v_v
#define v_bf16_u16_sel_eq_v_v_v_s                           v_bf16_u16_sel_eq_v_v_v_v

#define v_bf16_u16_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel_eq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_u16_sel_eq_v_s_v_v_vb                        v_bf16_u16_sel_eq_v_v_v_v_vb
#define v_bf16_u16_sel_eq_v_v_v_s_vb                        v_bf16_u16_sel_eq_v_v_v_v_vb

// u16 - i16
#define v_i16_u16_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u16_sel_eq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_u16_sel_eq_v_s_v_v_b                          v_i16_u16_sel_eq_v_v_v_v_b
#define v_i16_u16_sel_eq_v_v_v_s_b                          v_i16_u16_sel_eq_v_v_v_v_b
#define v_i16_u16_sel_eq_v_v_v_v(a, b, c, d)                v_i16_u16_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_u16_sel_eq_v_s_v_v                            v_i16_u16_sel_eq_v_v_v_v
#define v_i16_u16_sel_eq_v_v_v_s                            v_i16_u16_sel_eq_v_v_v_v

#define v_i16_u16_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u16_sel_eq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_u16_sel_eq_v_s_v_v_vb                         v_i16_u16_sel_eq_v_v_v_v_vb
#define v_i16_u16_sel_eq_v_v_v_s_vb                         v_i16_u16_sel_eq_v_v_v_v_vb

// u16 - u16
#define v_u16_u16_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u16_sel_eq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_u16_sel_eq_v_s_v_v_b                          v_u16_u16_sel_eq_v_v_v_v_b
#define v_u16_u16_sel_eq_v_v_v_s_b                          v_u16_u16_sel_eq_v_v_v_v_b
#define v_u16_u16_sel_eq_v_v_v_v(a, b, c, d)                v_u16_u16_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_u16_sel_eq_v_s_v_v                            v_u16_u16_sel_eq_v_v_v_v
#define v_u16_u16_sel_eq_v_v_v_s                            v_u16_u16_sel_eq_v_v_v_v

#define v_u16_u16_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u16_sel_eq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_u16_sel_eq_v_s_v_v_vb                         v_u16_u16_sel_eq_v_v_v_v_vb
#define v_u16_u16_sel_eq_v_v_v_s_vb                         v_u16_u16_sel_eq_v_v_v_v_vb

// i8 - u8
#define v_i8_i8_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)       v_i8_sel_eq_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel_eq_v_s_v_v_b                            v_i8_i8_sel_eq_v_v_v_v_b
#define v_i8_i8_sel_eq_v_v_v_s_b                            v_i8_i8_sel_eq_v_v_v_v_b
#define v_i8_i8_sel_eq_v_v_v_v(a, b, c, d)                  v_i8_i8_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i8_i8_sel_eq_v_s_v_v                              v_i8_i8_sel_eq_v_v_v_v
#define v_i8_i8_sel_eq_v_v_v_s                              v_i8_i8_sel_eq_v_v_v_v

#define v_i8_i8_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_i8_sel_eq_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel_eq_v_s_v_v_vb                           v_i8_i8_sel_eq_v_v_v_v_vb
#define v_i8_i8_sel_eq_v_v_v_s_vb                           v_i8_i8_sel_eq_v_v_v_v_vb

// i8 - u8
#define v_u8_i8_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)       v_i8_sel_eq_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel_eq_v_s_v_v_b                            v_u8_i8_sel_eq_v_v_v_v_b
#define v_u8_i8_sel_eq_v_v_v_s_b                            v_u8_i8_sel_eq_v_v_v_v_b
#define v_u8_i8_sel_eq_v_v_v_v(a, b, c, d)                  v_u8_i8_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u8_i8_sel_eq_v_s_v_v                              v_u8_i8_sel_eq_v_v_v_v
#define v_u8_i8_sel_eq_v_v_v_s                              v_u8_i8_sel_eq_v_v_v_v

#define v_u8_i8_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_i8_sel_eq_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel_eq_v_s_v_v_vb                           v_u8_i8_sel_eq_v_v_v_v_vb
#define v_u8_i8_sel_eq_v_v_v_s_vb                           v_u8_i8_sel_eq_v_v_v_v_vb

// u8 - i8
#define v_i8_u8_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)       v_u8_sel_eq_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel_eq_v_s_v_v_b                            v_i8_u8_sel_eq_v_v_v_v_b
#define v_i8_u8_sel_eq_v_v_v_s_b                            v_i8_u8_sel_eq_v_v_v_v_b
#define v_i8_u8_sel_eq_v_v_v_v(a, b, c, d)                  v_i8_u8_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i8_u8_sel_eq_v_s_v_v                              v_i8_u8_sel_eq_v_v_v_v
#define v_i8_u8_sel_eq_v_v_v_s                              v_i8_u8_sel_eq_v_v_v_v

#define v_i8_u8_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_u8_sel_eq_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel_eq_v_s_v_v_vb                           v_i8_u8_sel_eq_v_v_v_v_vb
#define v_i8_u8_sel_eq_v_v_v_s_vb                           v_i8_u8_sel_eq_v_v_v_v_vb

// u8 - u8
#define v_u8_u8_sel_eq_v_v_v_v_b(a, b, c, d, i, p, o)       v_u8_sel_eq_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel_eq_v_s_v_v_b                            v_u8_u8_sel_eq_v_v_v_v_b
#define v_u8_u8_sel_eq_v_v_v_s_b                            v_u8_u8_sel_eq_v_v_v_v_b
#define v_u8_u8_sel_eq_v_v_v_v(a, b, c, d)                  v_u8_u8_sel_eq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u8_u8_sel_eq_v_s_v_v                              v_u8_u8_sel_eq_v_v_v_v
#define v_u8_u8_sel_eq_v_v_v_s                              v_u8_u8_sel_eq_v_v_v_v

#define v_u8_u8_sel_eq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_u8_sel_eq_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel_eq_v_s_v_v_vb                           v_u8_u8_sel_eq_v_v_v_v_vb
#define v_u8_u8_sel_eq_v_v_v_s_vb                           v_u8_u8_sel_eq_v_v_v_v_vb


// SEL_EQ with MASK_EQ_ZERO

// f32 - f32
#define v_f32_f32_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_eq_f32_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_f32_f32_sel_eq_zero_v_s_v_v_b                          v_f32_f32_sel_eq_zero_v_v_v_v_b
#define v_f32_f32_sel_eq_zero_v_v_v_s_b                          v_f32_f32_sel_eq_zero_v_v_v_v_b
#define v_f32_f32_sel_eq_zero_v_v_v_v(a, b, c, d)                v_f32_f32_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_f32_sel_eq_zero_v_s_v_v                            v_f32_f32_sel_eq_zero_v_v_v_v
#define v_f32_f32_sel_eq_zero_v_v_v_s                            v_f32_f32_sel_eq_zero_v_v_v_v

#define v_f32_f32_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_eq_f32_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool64(p), o)
#define v_f32_f32_sel_eq_zero_v_s_v_v_vb                         v_f32_f32_sel_eq_zero_v_v_v_v_vb
#define v_f32_f32_sel_eq_zero_v_v_v_s_vb                         v_f32_f32_sel_eq_zero_v_v_v_v_vb

// f32 - i32
#define v_i32_f32_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_eq_i32_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_i32_f32_sel_eq_zero_v_s_v_v_b                          v_i32_f32_sel_eq_zero_v_v_v_v_b
#define v_i32_f32_sel_eq_zero_v_v_v_s_b                          v_i32_f32_sel_eq_zero_v_v_v_v_b
#define v_i32_f32_sel_eq_zero_v_v_v_v(a, b, c, d)                v_i32_f32_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_f32_sel_eq_zero_v_s_v_v                            v_i32_f32_sel_eq_zero_v_v_v_v
#define v_i32_f32_sel_eq_zero_v_v_v_s                            v_i32_f32_sel_eq_zero_v_v_v_v

#define v_i32_f32_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_eq_i32_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool64(p), o)
#define v_i32_f32_sel_eq_zero_v_s_v_v_vb                         v_i32_f32_sel_eq_zero_v_v_v_v_vb
#define v_i32_f32_sel_eq_zero_v_v_v_s_vb                         v_i32_f32_sel_eq_zero_v_v_v_v_vb

// f32 - u32
#define v_u32_f32_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_eq_u32_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_u32_f32_sel_eq_zero_v_s_v_v_b                          v_u32_f32_sel_eq_zero_v_v_v_v_b
#define v_u32_f32_sel_eq_zero_v_v_v_s_b                          v_u32_f32_sel_eq_zero_v_v_v_v_b
#define v_u32_f32_sel_eq_zero_v_v_v_v(a, b, c, d)                v_u32_f32_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_f32_sel_eq_zero_v_s_v_v                            v_u32_f32_sel_eq_zero_v_v_v_v
#define v_u32_f32_sel_eq_zero_v_v_v_s                            v_u32_f32_sel_eq_zero_v_v_v_v

#define v_u32_f32_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_eq_u32_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool64(p), o)
#define v_u32_f32_sel_eq_zero_v_s_v_v_vb                         v_u32_f32_sel_eq_zero_v_v_v_v_vb
#define v_u32_f32_sel_eq_zero_v_v_v_s_vb                         v_u32_f32_sel_eq_zero_v_v_v_v_vb

// bf16 - bf16
#define v_bf16_bf16_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)   v_bf16_sel_eq_bf16_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_bf16_bf16_sel_eq_zero_v_s_v_v_b                        v_bf16_bf16_sel_eq_zero_v_v_v_v_b
#define v_bf16_bf16_sel_eq_zero_v_v_v_s_b                        v_bf16_bf16_sel_eq_zero_v_v_v_v_b
#define v_bf16_bf16_sel_eq_zero_v_v_v_v(a, b, c, d)              v_bf16_bf16_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_bf16_sel_eq_zero_v_s_v_v                          v_bf16_bf16_sel_eq_zero_v_v_v_v
#define v_bf16_bf16_sel_eq_zero_v_v_v_s                          v_bf16_bf16_sel_eq_zero_v_v_v_v

#define v_bf16_bf16_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)  v_bf16_sel_eq_bf16_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool128(p), o)
#define v_bf16_bf16_sel_eq_zero_v_s_v_v_vb                       v_bf16_bf16_sel_eq_zero_v_v_v_v_vb
#define v_bf16_bf16_sel_eq_zero_v_v_v_s_vb                       v_bf16_bf16_sel_eq_zero_v_v_v_v_vb

// bf16 - i16
#define v_i16_bf16_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel_eq_i16_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_i16_bf16_sel_eq_zero_v_s_v_v_b                         v_i16_bf16_sel_eq_zero_v_v_v_v_b
#define v_i16_bf16_sel_eq_zero_v_v_v_s_b                         v_i16_bf16_sel_eq_zero_v_v_v_v_b
#define v_i16_bf16_sel_eq_zero_v_v_v_v(a, b, c, d)               v_i16_bf16_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_bf16_sel_eq_zero_v_s_v_v                           v_i16_bf16_sel_eq_zero_v_v_v_v
#define v_i16_bf16_sel_eq_zero_v_v_v_s                           v_i16_bf16_sel_eq_zero_v_v_v_v

#define v_i16_bf16_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel_eq_i16_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool128(p), o)
#define v_i16_bf16_sel_eq_zero_v_s_v_v_vb                        v_i16_bf16_sel_eq_zero_v_v_v_v_vb
#define v_i16_bf16_sel_eq_zero_v_v_v_s_vb                        v_i16_bf16_sel_eq_zero_v_v_v_v_vb

// bf16 - u16
#define v_u16_bf16_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel_eq_u16_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_u16_bf16_sel_eq_zero_v_s_v_v_b                         v_u16_bf16_sel_eq_zero_v_v_v_v_b
#define v_u16_bf16_sel_eq_zero_v_v_v_s_b                         v_u16_bf16_sel_eq_zero_v_v_v_v_b
#define v_u16_bf16_sel_eq_zero_v_v_v_v(a, b, c, d)               v_u16_bf16_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_bf16_sel_eq_zero_v_s_v_v                           v_u16_bf16_sel_eq_zero_v_v_v_v
#define v_u16_bf16_sel_eq_zero_v_v_v_s                           v_u16_bf16_sel_eq_zero_v_v_v_v

#define v_u16_bf16_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel_eq_u16_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool128(p), o)
#define v_u16_bf16_sel_eq_zero_v_s_v_v_vb                        v_u16_bf16_sel_eq_zero_v_v_v_v_vb
#define v_u16_bf16_sel_eq_zero_v_v_v_s_vb                        v_u16_bf16_sel_eq_zero_v_v_v_v_vb

// i32 - f32
#define v_f32_i32_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_eq_f32_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_f32_i32_sel_eq_zero_v_s_v_v_b                          v_f32_i32_sel_eq_zero_v_v_v_v_b
#define v_f32_i32_sel_eq_zero_v_v_v_s_b                          v_f32_i32_sel_eq_zero_v_v_v_v_b
#define v_f32_i32_sel_eq_zero_v_v_v_v(a, b, c, d)                v_f32_i32_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_i32_sel_eq_zero_v_s_v_v                            v_f32_i32_sel_eq_zero_v_v_v_v
#define v_f32_i32_sel_eq_zero_v_v_v_s                            v_f32_i32_sel_eq_zero_v_v_v_v

#define v_f32_i32_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_eq_f32_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool64(p), o)
#define v_f32_i32_sel_eq_zero_v_s_v_v_vb                         v_f32_i32_sel_eq_zero_v_v_v_v_vb
#define v_f32_i32_sel_eq_zero_v_v_v_s_vb                         v_f32_i32_sel_eq_zero_v_v_v_v_vb

// i32 - i32
#define v_i32_i32_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_eq_i32_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_i32_i32_sel_eq_zero_v_s_v_v_b                          v_i32_i32_sel_eq_zero_v_v_v_v_b
#define v_i32_i32_sel_eq_zero_v_v_v_s_b                          v_i32_i32_sel_eq_zero_v_v_v_v_b
#define v_i32_i32_sel_eq_zero_v_v_v_v(a, b, c, d)                v_i32_i32_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_i32_sel_eq_zero_v_s_v_v                            v_i32_i32_sel_eq_zero_v_v_v_v
#define v_i32_i32_sel_eq_zero_v_v_v_s                            v_i32_i32_sel_eq_zero_v_v_v_v

#define v_i32_i32_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_eq_i32_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool64(p), o)
#define v_i32_i32_sel_eq_zero_v_s_v_v_vb                         v_i32_i32_sel_eq_zero_v_v_v_v_vb
#define v_i32_i32_sel_eq_zero_v_v_v_s_vb                         v_i32_i32_sel_eq_zero_v_v_v_v_vb

// i32 - u32
#define v_u32_i32_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_eq_u32_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_u32_i32_sel_eq_zero_v_s_v_v_b                          v_u32_i32_sel_eq_zero_v_v_v_v_b
#define v_u32_i32_sel_eq_zero_v_v_v_s_b                          v_u32_i32_sel_eq_zero_v_v_v_v_b
#define v_u32_i32_sel_eq_zero_v_v_v_v(a, b, c, d)                v_u32_i32_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_i32_sel_eq_zero_v_s_v_v                            v_u32_i32_sel_eq_zero_v_v_v_v
#define v_u32_i32_sel_eq_zero_v_v_v_s                            v_u32_i32_sel_eq_zero_v_v_v_v

#define v_u32_i32_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_eq_u32_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool64(p), o)
#define v_u32_i32_sel_eq_zero_v_s_v_v_vb                         v_u32_i32_sel_eq_zero_v_v_v_v_vb
#define v_u32_i32_sel_eq_zero_v_v_v_s_vb                         v_u32_i32_sel_eq_zero_v_v_v_v_vb

// u32 - f32
#define v_f32_u32_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_eq_f32_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_f32_u32_sel_eq_zero_v_s_v_v_b                          v_f32_u32_sel_eq_zero_v_v_v_v_b
#define v_f32_u32_sel_eq_zero_v_v_v_s_b                          v_f32_u32_sel_eq_zero_v_v_v_v_b
#define v_f32_u32_sel_eq_zero_v_v_v_v(a, b, c, d)                v_f32_u32_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_u32_sel_eq_zero_v_s_v_v                            v_f32_u32_sel_eq_zero_v_v_v_v
#define v_f32_u32_sel_eq_zero_v_v_v_s                            v_f32_u32_sel_eq_zero_v_v_v_v

#define v_f32_u32_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_eq_f32_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool64(p), o)
#define v_f32_u32_sel_eq_zero_v_s_v_v_vb                         v_f32_u32_sel_eq_zero_v_v_v_v_vb
#define v_f32_u32_sel_eq_zero_v_v_v_s_vb                         v_f32_u32_sel_eq_zero_v_v_v_v_vb

// u32 - i32
#define v_i32_u32_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_eq_i32_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_i32_u32_sel_eq_zero_v_s_v_v_b                          v_i32_u32_sel_eq_zero_v_v_v_v_b
#define v_i32_u32_sel_eq_zero_v_v_v_s_b                          v_i32_u32_sel_eq_zero_v_v_v_v_b
#define v_i32_u32_sel_eq_zero_v_v_v_v(a, b, c, d)                v_i32_u32_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_u32_sel_eq_zero_v_s_v_v                            v_i32_u32_sel_eq_zero_v_v_v_v
#define v_i32_u32_sel_eq_zero_v_v_v_s                            v_i32_u32_sel_eq_zero_v_v_v_v

#define v_i32_u32_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_eq_i32_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool64(p), o)
#define v_i32_u32_sel_eq_zero_v_s_v_v_vb                         v_i32_u32_sel_eq_zero_v_v_v_v_vb
#define v_i32_u32_sel_eq_zero_v_v_v_s_vb                         v_i32_u32_sel_eq_zero_v_v_v_v_vb

// u32 - u32
#define v_u32_u32_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_eq_u32_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_u32_u32_sel_eq_zero_v_s_v_v_b                          v_u32_u32_sel_eq_zero_v_v_v_v_b
#define v_u32_u32_sel_eq_zero_v_v_v_s_b                          v_u32_u32_sel_eq_zero_v_v_v_v_b
#define v_u32_u32_sel_eq_zero_v_v_v_v(a, b, c, d)                v_u32_u32_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_u32_sel_eq_zero_v_s_v_v                            v_u32_u32_sel_eq_zero_v_v_v_v
#define v_u32_u32_sel_eq_zero_v_v_v_s                            v_u32_u32_sel_eq_zero_v_v_v_v

#define v_u32_u32_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_eq_u32_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool64(p), o)
#define v_u32_u32_sel_eq_zero_v_s_v_v_vb                         v_u32_u32_sel_eq_zero_v_v_v_v_vb
#define v_u32_u32_sel_eq_zero_v_v_v_s_vb                         v_u32_u32_sel_eq_zero_v_v_v_v_vb

// i16 - bf16
#define v_bf16_i16_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel_eq_bf16_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_bf16_i16_sel_eq_zero_v_s_v_v_b                         v_bf16_i16_sel_eq_zero_v_v_v_v_b
#define v_bf16_i16_sel_eq_zero_v_v_v_s_b                         v_bf16_i16_sel_eq_zero_v_v_v_v_b
#define v_bf16_i16_sel_eq_zero_v_v_v_v(a, b, c, d)               v_bf16_i16_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_i16_sel_eq_zero_v_s_v_v                           v_bf16_i16_sel_eq_zero_v_v_v_v
#define v_bf16_i16_sel_eq_zero_v_v_v_s                           v_bf16_i16_sel_eq_zero_v_v_v_v

#define v_bf16_i16_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel_eq_bf16_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool128(p), o)
#define v_bf16_i16_sel_eq_zero_v_s_v_v_vb                        v_bf16_i16_sel_eq_zero_v_v_v_v_vb
#define v_bf16_i16_sel_eq_zero_v_v_v_s_vb                        v_bf16_i16_sel_eq_zero_v_v_v_v_vb

// i16 - i16
#define v_i16_i16_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)     v_i16_sel_eq_i16_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_i16_i16_sel_eq_zero_v_s_v_v_b                          v_i16_i16_sel_eq_zero_v_v_v_v_b
#define v_i16_i16_sel_eq_zero_v_v_v_s_b                          v_i16_i16_sel_eq_zero_v_v_v_v_b
#define v_i16_i16_sel_eq_zero_v_v_v_v(a, b, c, d)                v_i16_i16_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_i16_sel_eq_zero_v_s_v_v                            v_i16_i16_sel_eq_zero_v_v_v_v
#define v_i16_i16_sel_eq_zero_v_v_v_s                            v_i16_i16_sel_eq_zero_v_v_v_v

#define v_i16_i16_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i16_sel_eq_i16_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool128(p), o)
#define v_i16_i16_sel_eq_zero_v_s_v_v_vb                         v_i16_i16_sel_eq_zero_v_v_v_v_vb
#define v_i16_i16_sel_eq_zero_v_v_v_s_vb                         v_i16_i16_sel_eq_zero_v_v_v_v_vb

// i16 - u16
#define v_u16_i16_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)     v_i16_sel_eq_u16_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_u16_i16_sel_eq_zero_v_s_v_v_b                          v_u16_i16_sel_eq_zero_v_v_v_v_b
#define v_u16_i16_sel_eq_zero_v_v_v_s_b                          v_u16_i16_sel_eq_zero_v_v_v_v_b
#define v_u16_i16_sel_eq_zero_v_v_v_v(a, b, c, d)                v_u16_i16_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_i16_sel_eq_zero_v_s_v_v                            v_u16_i16_sel_eq_zero_v_v_v_v
#define v_u16_i16_sel_eq_zero_v_v_v_s                            v_u16_i16_sel_eq_zero_v_v_v_v

#define v_u16_i16_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i16_sel_eq_u16_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool128(p), o)
#define v_u16_i16_sel_eq_zero_v_s_v_v_vb                         v_u16_i16_sel_eq_zero_v_v_v_v_vb
#define v_u16_i16_sel_eq_zero_v_v_v_s_vb                         v_u16_i16_sel_eq_zero_v_v_v_v_vb

// u16 - bf16
#define v_bf16_u16_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel_eq_bf16_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_bf16_u16_sel_eq_zero_v_s_v_v_b                         v_bf16_u16_sel_eq_zero_v_v_v_v_b
#define v_bf16_u16_sel_eq_zero_v_v_v_s_b                         v_bf16_u16_sel_eq_zero_v_v_v_v_b
#define v_bf16_u16_sel_eq_zero_v_v_v_v(a, b, c, d)               v_bf16_u16_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_u16_sel_eq_zero_v_s_v_v                           v_bf16_u16_sel_eq_zero_v_v_v_v
#define v_bf16_u16_sel_eq_zero_v_v_v_s                           v_bf16_u16_sel_eq_zero_v_v_v_v

#define v_bf16_u16_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel_eq_bf16_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool128(p), o)
#define v_bf16_u16_sel_eq_zero_v_s_v_v_vb                        v_bf16_u16_sel_eq_zero_v_v_v_v_vb
#define v_bf16_u16_sel_eq_zero_v_v_v_s_vb                        v_bf16_u16_sel_eq_zero_v_v_v_v_vb

// u16 - i16
#define v_i16_u16_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)     v_u16_sel_eq_i16_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_i16_u16_sel_eq_zero_v_s_v_v_b                          v_i16_u16_sel_eq_zero_v_v_v_v_b
#define v_i16_u16_sel_eq_zero_v_v_v_s_b                          v_i16_u16_sel_eq_zero_v_v_v_v_b
#define v_i16_u16_sel_eq_zero_v_v_v_v(a, b, c, d)                v_i16_u16_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_u16_sel_eq_zero_v_s_v_v                            v_i16_u16_sel_eq_zero_v_v_v_v
#define v_i16_u16_sel_eq_zero_v_v_v_s                            v_i16_u16_sel_eq_zero_v_v_v_v

#define v_i16_u16_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u16_sel_eq_i16_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool128(p), o)
#define v_i16_u16_sel_eq_zero_v_s_v_v_vb                         v_i16_u16_sel_eq_zero_v_v_v_v_vb
#define v_i16_u16_sel_eq_zero_v_v_v_s_vb                         v_i16_u16_sel_eq_zero_v_v_v_v_vb

// u16 - u16
#define v_u16_u16_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)     v_u16_sel_eq_u16_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_u16_u16_sel_eq_zero_v_s_v_v_b                          v_u16_u16_sel_eq_zero_v_v_v_v_b
#define v_u16_u16_sel_eq_zero_v_v_v_s_b                          v_u16_u16_sel_eq_zero_v_v_v_v_b
#define v_u16_u16_sel_eq_zero_v_v_v_v(a, b, c, d)                v_u16_u16_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_u16_sel_eq_zero_v_s_v_v                            v_u16_u16_sel_eq_zero_v_v_v_v
#define v_u16_u16_sel_eq_zero_v_v_v_s                            v_u16_u16_sel_eq_zero_v_v_v_v

#define v_u16_u16_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u16_sel_eq_u16_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, to_bool128(p), o)
#define v_u16_u16_sel_eq_zero_v_s_v_v_vb                         v_u16_u16_sel_eq_zero_v_v_v_v_vb
#define v_u16_u16_sel_eq_zero_v_v_v_s_vb                         v_u16_u16_sel_eq_zero_v_v_v_v_vb

// i8 - u8
#define v_i8_i8_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)       v_i8_sel_eq_i8_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_i8_i8_sel_eq_zero_v_s_v_v_b                            v_i8_i8_sel_eq_zero_v_v_v_v_b
#define v_i8_i8_sel_eq_zero_v_v_v_s_b                            v_i8_i8_sel_eq_zero_v_v_v_v_b
#define v_i8_i8_sel_eq_zero_v_v_v_v(a, b, c, d)                  v_i8_i8_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i8_i8_sel_eq_zero_v_s_v_v                              v_i8_i8_sel_eq_zero_v_v_v_v
#define v_i8_i8_sel_eq_zero_v_v_v_s                              v_i8_i8_sel_eq_zero_v_v_v_v

#define v_i8_i8_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)      v_i8_sel_eq_i8_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_i8_i8_sel_eq_zero_v_s_v_v_vb                           v_i8_i8_sel_eq_zero_v_v_v_v_vb
#define v_i8_i8_sel_eq_zero_v_v_v_s_vb                           v_i8_i8_sel_eq_zero_v_v_v_v_vb

// i8 - u8
#define v_u8_i8_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)       v_i8_sel_eq_u8_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_u8_i8_sel_eq_zero_v_s_v_v_b                            v_u8_i8_sel_eq_zero_v_v_v_v_b
#define v_u8_i8_sel_eq_zero_v_v_v_s_b                            v_u8_i8_sel_eq_zero_v_v_v_v_b
#define v_u8_i8_sel_eq_zero_v_v_v_v(a, b, c, d)                  v_u8_i8_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u8_i8_sel_eq_zero_v_s_v_v                              v_u8_i8_sel_eq_zero_v_v_v_v
#define v_u8_i8_sel_eq_zero_v_v_v_s                              v_u8_i8_sel_eq_zero_v_v_v_v

#define v_u8_i8_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)      v_i8_sel_eq_u8_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_u8_i8_sel_eq_zero_v_s_v_v_vb                           v_u8_i8_sel_eq_zero_v_v_v_v_vb
#define v_u8_i8_sel_eq_zero_v_v_v_s_vb                           v_u8_i8_sel_eq_zero_v_v_v_v_vb

// u8 - i8
#define v_i8_u8_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)       v_u8_sel_eq_i8_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_i8_u8_sel_eq_zero_v_s_v_v_b                            v_i8_u8_sel_eq_zero_v_v_v_v_b
#define v_i8_u8_sel_eq_zero_v_v_v_s_b                            v_i8_u8_sel_eq_zero_v_v_v_v_b
#define v_i8_u8_sel_eq_zero_v_v_v_v(a, b, c, d)                  v_i8_u8_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i8_u8_sel_eq_zero_v_s_v_v                              v_i8_u8_sel_eq_zero_v_v_v_v
#define v_i8_u8_sel_eq_zero_v_v_v_s                              v_i8_u8_sel_eq_zero_v_v_v_v

#define v_i8_u8_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)      v_u8_sel_eq_i8_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_i8_u8_sel_eq_zero_v_s_v_v_vb                           v_i8_u8_sel_eq_zero_v_v_v_v_vb
#define v_i8_u8_sel_eq_zero_v_v_v_s_vb                           v_i8_u8_sel_eq_zero_v_v_v_v_vb

// u8 - u8
#define v_u8_u8_sel_eq_zero_v_v_v_v_b(a, b, c, d, i, p, o)       v_u8_sel_eq_u8_b(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_u8_u8_sel_eq_zero_v_s_v_v_b                            v_u8_u8_sel_eq_zero_v_v_v_v_b
#define v_u8_u8_sel_eq_zero_v_v_v_s_b                            v_u8_u8_sel_eq_zero_v_v_v_v_b
#define v_u8_u8_sel_eq_zero_v_v_v_v(a, b, c, d)                  v_u8_u8_sel_eq_zero_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u8_u8_sel_eq_zero_v_s_v_v                              v_u8_u8_sel_eq_zero_v_v_v_v
#define v_u8_u8_sel_eq_zero_v_v_v_s                              v_u8_u8_sel_eq_zero_v_v_v_v

#define v_u8_u8_sel_eq_zero_v_v_v_v_vb(a, b, c, d, i, p, o)      v_u8_sel_eq_u8_vb(a, b, c, d, SW_MASK_EQ_ZERO, i, p, o)
#define v_u8_u8_sel_eq_zero_v_s_v_v_vb                           v_u8_u8_sel_eq_zero_v_v_v_v_vb
#define v_u8_u8_sel_eq_zero_v_v_v_s_vb                           v_u8_u8_sel_eq_zero_v_v_v_v_vb


// SEL_NEQ

// f32 - f32
#define v_f32_f32_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_neq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_f32_sel_neq_v_s_v_v_b                          v_f32_f32_sel_neq_v_v_v_v_b
#define v_f32_f32_sel_neq_v_v_v_s_b                          v_f32_f32_sel_neq_v_v_v_v_b
#define v_f32_f32_sel_neq_v_v_v_v(a, b, c, d)                v_f32_f32_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_f32_sel_neq_v_s_v_v                            v_f32_f32_sel_neq_v_v_v_v
#define v_f32_f32_sel_neq_v_v_v_s                            v_f32_f32_sel_neq_v_v_v_v

#define v_f32_f32_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_neq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_f32_sel_neq_v_s_v_v_vb                         v_f32_f32_sel_neq_v_v_v_v_vb
#define v_f32_f32_sel_neq_v_v_v_s_vb                         v_f32_f32_sel_neq_v_v_v_v_vb

// f32 - i32
#define v_i32_f32_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_neq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_f32_sel_neq_v_s_v_v_b                          v_i32_f32_sel_neq_v_v_v_v_b
#define v_i32_f32_sel_neq_v_v_v_s_b                          v_i32_f32_sel_neq_v_v_v_v_b
#define v_i32_f32_sel_neq_v_v_v_v(a, b, c, d)                v_i32_f32_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_f32_sel_neq_v_s_v_v                            v_i32_f32_sel_neq_v_v_v_v
#define v_i32_f32_sel_neq_v_v_v_s                            v_i32_f32_sel_neq_v_v_v_v

#define v_i32_f32_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_neq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_f32_sel_neq_v_s_v_v_vb                         v_i32_f32_sel_neq_v_v_v_v_vb
#define v_i32_f32_sel_neq_v_v_v_s_vb                         v_i32_f32_sel_neq_v_v_v_v_vb

// f32 - u32
#define v_u32_f32_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_neq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_f32_sel_neq_v_s_v_v_b                          v_u32_f32_sel_neq_v_v_v_v_b
#define v_u32_f32_sel_neq_v_v_v_s_b                          v_u32_f32_sel_neq_v_v_v_v_b
#define v_u32_f32_sel_neq_v_v_v_v(a, b, c, d)                v_u32_f32_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_f32_sel_neq_v_s_v_v                            v_u32_f32_sel_neq_v_v_v_v
#define v_u32_f32_sel_neq_v_v_v_s                            v_u32_f32_sel_neq_v_v_v_v

#define v_u32_f32_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_neq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_f32_sel_neq_v_s_v_v_vb                         v_u32_f32_sel_neq_v_v_v_v_vb
#define v_u32_f32_sel_neq_v_v_v_s_vb                         v_u32_f32_sel_neq_v_v_v_v_vb

// bf16 - bf16
#define v_bf16_bf16_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)   v_bf16_sel_neq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_bf16_sel_neq_v_s_v_v_b                        v_bf16_bf16_sel_neq_v_v_v_v_b
#define v_bf16_bf16_sel_neq_v_v_v_s_b                        v_bf16_bf16_sel_neq_v_v_v_v_b
#define v_bf16_bf16_sel_neq_v_v_v_v(a, b, c, d)              v_bf16_bf16_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_bf16_sel_neq_v_s_v_v                          v_bf16_bf16_sel_neq_v_v_v_v
#define v_bf16_bf16_sel_neq_v_v_v_s                          v_bf16_bf16_sel_neq_v_v_v_v

#define v_bf16_bf16_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)  v_bf16_sel_neq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_bf16_sel_neq_v_s_v_v_vb                       v_bf16_bf16_sel_neq_v_v_v_v_vb
#define v_bf16_bf16_sel_neq_v_v_v_s_vb                       v_bf16_bf16_sel_neq_v_v_v_v_vb

// bf16 - i16
#define v_i16_bf16_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel_neq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_bf16_sel_neq_v_s_v_v_b                         v_i16_bf16_sel_neq_v_v_v_v_b
#define v_i16_bf16_sel_neq_v_v_v_s_b                         v_i16_bf16_sel_neq_v_v_v_v_b
#define v_i16_bf16_sel_neq_v_v_v_v(a, b, c, d)               v_i16_bf16_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_bf16_sel_neq_v_s_v_v                           v_i16_bf16_sel_neq_v_v_v_v
#define v_i16_bf16_sel_neq_v_v_v_s                           v_i16_bf16_sel_neq_v_v_v_v

#define v_i16_bf16_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel_neq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_bf16_sel_neq_v_s_v_v_vb                        v_i16_bf16_sel_neq_v_v_v_v_vb
#define v_i16_bf16_sel_neq_v_v_v_s_vb                        v_i16_bf16_sel_neq_v_v_v_v_vb

// bf16 - u16
#define v_u16_bf16_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel_neq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_bf16_sel_neq_v_s_v_v_b                         v_u16_bf16_sel_neq_v_v_v_v_b
#define v_u16_bf16_sel_neq_v_v_v_s_b                         v_u16_bf16_sel_neq_v_v_v_v_b
#define v_u16_bf16_sel_neq_v_v_v_v(a, b, c, d)               v_u16_bf16_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_bf16_sel_neq_v_s_v_v                           v_u16_bf16_sel_neq_v_v_v_v
#define v_u16_bf16_sel_neq_v_v_v_s                           v_u16_bf16_sel_neq_v_v_v_v

#define v_u16_bf16_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel_neq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_bf16_sel_neq_v_s_v_v_vb                        v_u16_bf16_sel_neq_v_v_v_v_vb
#define v_u16_bf16_sel_neq_v_v_v_s_vb                        v_u16_bf16_sel_neq_v_v_v_v_vb

// i32 - f32
#define v_f32_i32_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_neq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_i32_sel_neq_v_s_v_v_b                          v_f32_i32_sel_neq_v_v_v_v_b
#define v_f32_i32_sel_neq_v_v_v_s_b                          v_f32_i32_sel_neq_v_v_v_v_b
#define v_f32_i32_sel_neq_v_v_v_v(a, b, c, d)                v_f32_i32_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_i32_sel_neq_v_s_v_v                            v_f32_i32_sel_neq_v_v_v_v
#define v_f32_i32_sel_neq_v_v_v_s                            v_f32_i32_sel_neq_v_v_v_v

#define v_f32_i32_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_neq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_i32_sel_neq_v_s_v_v_vb                         v_f32_i32_sel_neq_v_v_v_v_vb
#define v_f32_i32_sel_neq_v_v_v_s_vb                         v_f32_i32_sel_neq_v_v_v_v_vb

// i32 - i32
#define v_i32_i32_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_neq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_i32_sel_neq_v_s_v_v_b                          v_i32_i32_sel_neq_v_v_v_v_b
#define v_i32_i32_sel_neq_v_v_v_s_b                          v_i32_i32_sel_neq_v_v_v_v_b
#define v_i32_i32_sel_neq_v_v_v_v(a, b, c, d)                v_i32_i32_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_i32_sel_neq_v_s_v_v                            v_i32_i32_sel_neq_v_v_v_v
#define v_i32_i32_sel_neq_v_v_v_s                            v_i32_i32_sel_neq_v_v_v_v

#define v_i32_i32_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_neq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_i32_sel_neq_v_s_v_v_vb                         v_i32_i32_sel_neq_v_v_v_v_vb
#define v_i32_i32_sel_neq_v_v_v_s_vb                         v_i32_i32_sel_neq_v_v_v_v_vb

// i32 - u32
#define v_u32_i32_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_neq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_i32_sel_neq_v_s_v_v_b                          v_u32_i32_sel_neq_v_v_v_v_b
#define v_u32_i32_sel_neq_v_v_v_s_b                          v_u32_i32_sel_neq_v_v_v_v_b
#define v_u32_i32_sel_neq_v_v_v_v(a, b, c, d)                v_u32_i32_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_i32_sel_neq_v_s_v_v                            v_u32_i32_sel_neq_v_v_v_v
#define v_u32_i32_sel_neq_v_v_v_s                            v_u32_i32_sel_neq_v_v_v_v

#define v_u32_i32_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_neq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_i32_sel_neq_v_s_v_v_vb                         v_u32_i32_sel_neq_v_v_v_v_vb
#define v_u32_i32_sel_neq_v_v_v_s_vb                         v_u32_i32_sel_neq_v_v_v_v_vb

// u32 - f32
#define v_f32_u32_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_neq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_u32_sel_neq_v_s_v_v_b                          v_f32_u32_sel_neq_v_v_v_v_b
#define v_f32_u32_sel_neq_v_v_v_s_b                          v_f32_u32_sel_neq_v_v_v_v_b
#define v_f32_u32_sel_neq_v_v_v_v(a, b, c, d)                v_f32_u32_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_u32_sel_neq_v_s_v_v                            v_f32_u32_sel_neq_v_v_v_v
#define v_f32_u32_sel_neq_v_v_v_s                            v_f32_u32_sel_neq_v_v_v_v

#define v_f32_u32_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_neq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_u32_sel_neq_v_s_v_v_vb                         v_f32_u32_sel_neq_v_v_v_v_vb
#define v_f32_u32_sel_neq_v_v_v_s_vb                         v_f32_u32_sel_neq_v_v_v_v_vb

// u32 - i32
#define v_i32_u32_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_neq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_u32_sel_neq_v_s_v_v_b                          v_i32_u32_sel_neq_v_v_v_v_b
#define v_i32_u32_sel_neq_v_v_v_s_b                          v_i32_u32_sel_neq_v_v_v_v_b
#define v_i32_u32_sel_neq_v_v_v_v(a, b, c, d)                v_i32_u32_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_u32_sel_neq_v_s_v_v                            v_i32_u32_sel_neq_v_v_v_v
#define v_i32_u32_sel_neq_v_v_v_s                            v_i32_u32_sel_neq_v_v_v_v

#define v_i32_u32_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_neq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_u32_sel_neq_v_s_v_v_vb                         v_i32_u32_sel_neq_v_v_v_v_vb
#define v_i32_u32_sel_neq_v_v_v_s_vb                         v_i32_u32_sel_neq_v_v_v_v_vb

// u32 - u32
#define v_u32_u32_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_neq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_u32_sel_neq_v_s_v_v_b                          v_u32_u32_sel_neq_v_v_v_v_b
#define v_u32_u32_sel_neq_v_v_v_s_b                          v_u32_u32_sel_neq_v_v_v_v_b
#define v_u32_u32_sel_neq_v_v_v_v(a, b, c, d)                v_u32_u32_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_u32_sel_neq_v_s_v_v                            v_u32_u32_sel_neq_v_v_v_v
#define v_u32_u32_sel_neq_v_v_v_s                            v_u32_u32_sel_neq_v_v_v_v

#define v_u32_u32_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_neq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_u32_sel_neq_v_s_v_v_vb                         v_u32_u32_sel_neq_v_v_v_v_vb
#define v_u32_u32_sel_neq_v_v_v_s_vb                         v_u32_u32_sel_neq_v_v_v_v_vb

// i16 - bf16
#define v_bf16_i16_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel_neq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_i16_sel_neq_v_s_v_v_b                         v_bf16_i16_sel_neq_v_v_v_v_b
#define v_bf16_i16_sel_neq_v_v_v_s_b                         v_bf16_i16_sel_neq_v_v_v_v_b
#define v_bf16_i16_sel_neq_v_v_v_v(a, b, c, d)               v_bf16_i16_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_i16_sel_neq_v_s_v_v                           v_bf16_i16_sel_neq_v_v_v_v
#define v_bf16_i16_sel_neq_v_v_v_s                           v_bf16_i16_sel_neq_v_v_v_v

#define v_bf16_i16_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel_neq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_i16_sel_neq_v_s_v_v_vb                        v_bf16_i16_sel_neq_v_v_v_v_vb
#define v_bf16_i16_sel_neq_v_v_v_s_vb                        v_bf16_i16_sel_neq_v_v_v_v_vb

// i16 - i16
#define v_i16_i16_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i16_sel_neq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_i16_sel_neq_v_s_v_v_b                          v_i16_i16_sel_neq_v_v_v_v_b
#define v_i16_i16_sel_neq_v_v_v_s_b                          v_i16_i16_sel_neq_v_v_v_v_b
#define v_i16_i16_sel_neq_v_v_v_v(a, b, c, d)                v_i16_i16_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_i16_sel_neq_v_s_v_v                            v_i16_i16_sel_neq_v_v_v_v
#define v_i16_i16_sel_neq_v_v_v_s                            v_i16_i16_sel_neq_v_v_v_v

#define v_i16_i16_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i16_sel_neq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_i16_sel_neq_v_s_v_v_vb                         v_i16_i16_sel_neq_v_v_v_v_vb
#define v_i16_i16_sel_neq_v_v_v_s_vb                         v_i16_i16_sel_neq_v_v_v_v_vb

// i16 - u16
#define v_u16_i16_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i16_sel_neq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_i16_sel_neq_v_s_v_v_b                          v_u16_i16_sel_neq_v_v_v_v_b
#define v_u16_i16_sel_neq_v_v_v_s_b                          v_u16_i16_sel_neq_v_v_v_v_b
#define v_u16_i16_sel_neq_v_v_v_v(a, b, c, d)                v_u16_i16_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_i16_sel_neq_v_s_v_v                            v_u16_i16_sel_neq_v_v_v_v
#define v_u16_i16_sel_neq_v_v_v_s                            v_u16_i16_sel_neq_v_v_v_v

#define v_u16_i16_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i16_sel_neq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_i16_sel_neq_v_s_v_v_vb                         v_u16_i16_sel_neq_v_v_v_v_vb
#define v_u16_i16_sel_neq_v_v_v_s_vb                         v_u16_i16_sel_neq_v_v_v_v_vb

// u16 - bf16
#define v_bf16_u16_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel_neq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_u16_sel_neq_v_s_v_v_b                         v_bf16_u16_sel_neq_v_v_v_v_b
#define v_bf16_u16_sel_neq_v_v_v_s_b                         v_bf16_u16_sel_neq_v_v_v_v_b
#define v_bf16_u16_sel_neq_v_v_v_v(a, b, c, d)               v_bf16_u16_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_u16_sel_neq_v_s_v_v                           v_bf16_u16_sel_neq_v_v_v_v
#define v_bf16_u16_sel_neq_v_v_v_s                           v_bf16_u16_sel_neq_v_v_v_v

#define v_bf16_u16_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel_neq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_u16_sel_neq_v_s_v_v_vb                        v_bf16_u16_sel_neq_v_v_v_v_vb
#define v_bf16_u16_sel_neq_v_v_v_s_vb                        v_bf16_u16_sel_neq_v_v_v_v_vb

// u16 - i16
#define v_i16_u16_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u16_sel_neq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_u16_sel_neq_v_s_v_v_b                          v_i16_u16_sel_neq_v_v_v_v_b
#define v_i16_u16_sel_neq_v_v_v_s_b                          v_i16_u16_sel_neq_v_v_v_v_b
#define v_i16_u16_sel_neq_v_v_v_v(a, b, c, d)                v_i16_u16_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_u16_sel_neq_v_s_v_v                            v_i16_u16_sel_neq_v_v_v_v
#define v_i16_u16_sel_neq_v_v_v_s                            v_i16_u16_sel_neq_v_v_v_v

#define v_i16_u16_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u16_sel_neq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_u16_sel_neq_v_s_v_v_vb                         v_i16_u16_sel_neq_v_v_v_v_vb
#define v_i16_u16_sel_neq_v_v_v_s_vb                         v_i16_u16_sel_neq_v_v_v_v_vb

// u16 - u16
#define v_u16_u16_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u16_sel_neq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_u16_sel_neq_v_s_v_v_b                          v_u16_u16_sel_neq_v_v_v_v_b
#define v_u16_u16_sel_neq_v_v_v_s_b                          v_u16_u16_sel_neq_v_v_v_v_b
#define v_u16_u16_sel_neq_v_v_v_v(a, b, c, d)                v_u16_u16_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_u16_sel_neq_v_s_v_v                            v_u16_u16_sel_neq_v_v_v_v
#define v_u16_u16_sel_neq_v_v_v_s                            v_u16_u16_sel_neq_v_v_v_v

#define v_u16_u16_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u16_sel_neq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_u16_sel_neq_v_s_v_v_vb                         v_u16_u16_sel_neq_v_v_v_v_vb
#define v_u16_u16_sel_neq_v_v_v_s_vb                         v_u16_u16_sel_neq_v_v_v_v_vb

// i8 - u8
#define v_i8_i8_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)       v_i8_sel_neq_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel_neq_v_s_v_v_b                            v_i8_i8_sel_neq_v_v_v_v_b
#define v_i8_i8_sel_neq_v_v_v_s_b                            v_i8_i8_sel_neq_v_v_v_v_b
#define v_i8_i8_sel_neq_v_v_v_v(a, b, c, d)                  v_i8_i8_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i8_i8_sel_neq_v_s_v_v                              v_i8_i8_sel_neq_v_v_v_v
#define v_i8_i8_sel_neq_v_v_v_s                              v_i8_i8_sel_neq_v_v_v_v

#define v_i8_i8_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_i8_sel_neq_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel_neq_v_s_v_v_vb                           v_i8_i8_sel_neq_v_v_v_v_vb
#define v_i8_i8_sel_neq_v_v_v_s_vb                           v_i8_i8_sel_neq_v_v_v_v_vb

// i8 - u8
#define v_u8_i8_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)       v_i8_sel_neq_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel_neq_v_s_v_v_b                            v_u8_i8_sel_neq_v_v_v_v_b
#define v_u8_i8_sel_neq_v_v_v_s_b                            v_u8_i8_sel_neq_v_v_v_v_b
#define v_u8_i8_sel_neq_v_v_v_v(a, b, c, d)                  v_u8_i8_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u8_i8_sel_neq_v_s_v_v                              v_u8_i8_sel_neq_v_v_v_v
#define v_u8_i8_sel_neq_v_v_v_s                              v_u8_i8_sel_neq_v_v_v_v

#define v_u8_i8_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_i8_sel_neq_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel_neq_v_s_v_v_vb                           v_u8_i8_sel_neq_v_v_v_v_vb
#define v_u8_i8_sel_neq_v_v_v_s_vb                           v_u8_i8_sel_neq_v_v_v_v_vb

// u8 - i8
#define v_i8_u8_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)       v_u8_sel_neq_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel_neq_v_s_v_v_b                            v_i8_u8_sel_neq_v_v_v_v_b
#define v_i8_u8_sel_neq_v_v_v_s_b                            v_i8_u8_sel_neq_v_v_v_v_b
#define v_i8_u8_sel_neq_v_v_v_v(a, b, c, d)                  v_i8_u8_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i8_u8_sel_neq_v_s_v_v                              v_i8_u8_sel_neq_v_v_v_v
#define v_i8_u8_sel_neq_v_v_v_s                              v_i8_u8_sel_neq_v_v_v_v

#define v_i8_u8_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_u8_sel_neq_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel_neq_v_s_v_v_vb                           v_i8_u8_sel_neq_v_v_v_v_vb
#define v_i8_u8_sel_neq_v_v_v_s_vb                           v_i8_u8_sel_neq_v_v_v_v_vb

// u8 - u8
#define v_u8_u8_sel_neq_v_v_v_v_b(a, b, c, d, i, p, o)       v_u8_sel_neq_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel_neq_v_s_v_v_b                            v_u8_u8_sel_neq_v_v_v_v_b
#define v_u8_u8_sel_neq_v_v_v_s_b                            v_u8_u8_sel_neq_v_v_v_v_b
#define v_u8_u8_sel_neq_v_v_v_v(a, b, c, d)                  v_u8_u8_sel_neq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u8_u8_sel_neq_v_s_v_v                              v_u8_u8_sel_neq_v_v_v_v
#define v_u8_u8_sel_neq_v_v_v_s                              v_u8_u8_sel_neq_v_v_v_v

#define v_u8_u8_sel_neq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_u8_sel_neq_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel_neq_v_s_v_v_vb                           v_u8_u8_sel_neq_v_v_v_v_vb
#define v_u8_u8_sel_neq_v_v_v_s_vb                           v_u8_u8_sel_neq_v_v_v_v_vb

// SEL_LESS

// f32 - f32
#define v_f32_f32_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_less_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_f32_sel_less_v_s_v_v_b                          v_f32_f32_sel_less_v_v_v_v_b
#define v_f32_f32_sel_less_v_v_v_s_b                          v_f32_f32_sel_less_v_v_v_v_b
#define v_f32_f32_sel_less_v_v_v_v(a, b, c, d)                v_f32_f32_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_f32_sel_less_v_s_v_v                            v_f32_f32_sel_less_v_v_v_v
#define v_f32_f32_sel_less_v_v_v_s                            v_f32_f32_sel_less_v_v_v_v

#define v_f32_f32_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_less_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_f32_sel_less_v_s_v_v_vb                         v_f32_f32_sel_less_v_v_v_v_vb
#define v_f32_f32_sel_less_v_v_v_s_vb                         v_f32_f32_sel_less_v_v_v_v_vb

// f32 - i32
#define v_i32_f32_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_less_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_f32_sel_less_v_s_v_v_b                          v_i32_f32_sel_less_v_v_v_v_b
#define v_i32_f32_sel_less_v_v_v_s_b                          v_i32_f32_sel_less_v_v_v_v_b
#define v_i32_f32_sel_less_v_v_v_v(a, b, c, d)                v_i32_f32_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_f32_sel_less_v_s_v_v                            v_i32_f32_sel_less_v_v_v_v
#define v_i32_f32_sel_less_v_v_v_s                            v_i32_f32_sel_less_v_v_v_v

#define v_i32_f32_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_less_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_f32_sel_less_v_s_v_v_vb                         v_i32_f32_sel_less_v_v_v_v_vb
#define v_i32_f32_sel_less_v_v_v_s_vb                         v_i32_f32_sel_less_v_v_v_v_vb

// f32 - u32
#define v_u32_f32_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_less_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_f32_sel_less_v_s_v_v_b                          v_u32_f32_sel_less_v_v_v_v_b
#define v_u32_f32_sel_less_v_v_v_s_b                          v_u32_f32_sel_less_v_v_v_v_b
#define v_u32_f32_sel_less_v_v_v_v(a, b, c, d)                v_u32_f32_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_f32_sel_less_v_s_v_v                            v_u32_f32_sel_less_v_v_v_v
#define v_u32_f32_sel_less_v_v_v_s                            v_u32_f32_sel_less_v_v_v_v

#define v_u32_f32_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_less_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_f32_sel_less_v_s_v_v_vb                         v_u32_f32_sel_less_v_v_v_v_vb
#define v_u32_f32_sel_less_v_v_v_s_vb                         v_u32_f32_sel_less_v_v_v_v_vb

// bf16 - bf16
#define v_bf16_bf16_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)   v_bf16_sel_less_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_bf16_sel_less_v_s_v_v_b                        v_bf16_bf16_sel_less_v_v_v_v_b
#define v_bf16_bf16_sel_less_v_v_v_s_b                        v_bf16_bf16_sel_less_v_v_v_v_b
#define v_bf16_bf16_sel_less_v_v_v_v(a, b, c, d)              v_bf16_bf16_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_bf16_sel_less_v_s_v_v                          v_bf16_bf16_sel_less_v_v_v_v
#define v_bf16_bf16_sel_less_v_v_v_s                          v_bf16_bf16_sel_less_v_v_v_v

#define v_bf16_bf16_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)  v_bf16_sel_less_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_bf16_sel_less_v_s_v_v_vb                       v_bf16_bf16_sel_less_v_v_v_v_vb
#define v_bf16_bf16_sel_less_v_v_v_s_vb                       v_bf16_bf16_sel_less_v_v_v_v_vb

// bf16 - i16
#define v_i16_bf16_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel_less_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_bf16_sel_less_v_s_v_v_b                         v_i16_bf16_sel_less_v_v_v_v_b
#define v_i16_bf16_sel_less_v_v_v_s_b                         v_i16_bf16_sel_less_v_v_v_v_b
#define v_i16_bf16_sel_less_v_v_v_v(a, b, c, d)               v_i16_bf16_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_bf16_sel_less_v_s_v_v                           v_i16_bf16_sel_less_v_v_v_v
#define v_i16_bf16_sel_less_v_v_v_s                           v_i16_bf16_sel_less_v_v_v_v

#define v_i16_bf16_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel_less_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_bf16_sel_less_v_s_v_v_vb                        v_i16_bf16_sel_less_v_v_v_v_vb
#define v_i16_bf16_sel_less_v_v_v_s_vb                        v_i16_bf16_sel_less_v_v_v_v_vb

// bf16 - u16
#define v_u16_bf16_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel_less_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_bf16_sel_less_v_s_v_v_b                         v_u16_bf16_sel_less_v_v_v_v_b
#define v_u16_bf16_sel_less_v_v_v_s_b                         v_u16_bf16_sel_less_v_v_v_v_b
#define v_u16_bf16_sel_less_v_v_v_v(a, b, c, d)               v_u16_bf16_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_bf16_sel_less_v_s_v_v                           v_u16_bf16_sel_less_v_v_v_v
#define v_u16_bf16_sel_less_v_v_v_s                           v_u16_bf16_sel_less_v_v_v_v

#define v_u16_bf16_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel_less_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_bf16_sel_less_v_s_v_v_vb                        v_u16_bf16_sel_less_v_v_v_v_vb
#define v_u16_bf16_sel_less_v_v_v_s_vb                        v_u16_bf16_sel_less_v_v_v_v_vb

// i32 - f32
#define v_f32_i32_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_less_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_i32_sel_less_v_s_v_v_b                          v_f32_i32_sel_less_v_v_v_v_b
#define v_f32_i32_sel_less_v_v_v_s_b                          v_f32_i32_sel_less_v_v_v_v_b
#define v_f32_i32_sel_less_v_v_v_v(a, b, c, d)                v_f32_i32_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_i32_sel_less_v_s_v_v                            v_f32_i32_sel_less_v_v_v_v
#define v_f32_i32_sel_less_v_v_v_s                            v_f32_i32_sel_less_v_v_v_v

#define v_f32_i32_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_less_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_i32_sel_less_v_s_v_v_vb                         v_f32_i32_sel_less_v_v_v_v_vb
#define v_f32_i32_sel_less_v_v_v_s_vb                         v_f32_i32_sel_less_v_v_v_v_vb

// i32 - i32
#define v_i32_i32_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_less_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_i32_sel_less_v_s_v_v_b                          v_i32_i32_sel_less_v_v_v_v_b
#define v_i32_i32_sel_less_v_v_v_s_b                          v_i32_i32_sel_less_v_v_v_v_b
#define v_i32_i32_sel_less_v_v_v_v(a, b, c, d)                v_i32_i32_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_i32_sel_less_v_s_v_v                            v_i32_i32_sel_less_v_v_v_v
#define v_i32_i32_sel_less_v_v_v_s                            v_i32_i32_sel_less_v_v_v_v

#define v_i32_i32_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_less_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_i32_sel_less_v_s_v_v_vb                         v_i32_i32_sel_less_v_v_v_v_vb
#define v_i32_i32_sel_less_v_v_v_s_vb                         v_i32_i32_sel_less_v_v_v_v_vb

// i32 - u32
#define v_u32_i32_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_less_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_i32_sel_less_v_s_v_v_b                          v_u32_i32_sel_less_v_v_v_v_b
#define v_u32_i32_sel_less_v_v_v_s_b                          v_u32_i32_sel_less_v_v_v_v_b
#define v_u32_i32_sel_less_v_v_v_v(a, b, c, d)                v_u32_i32_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_i32_sel_less_v_s_v_v                            v_u32_i32_sel_less_v_v_v_v
#define v_u32_i32_sel_less_v_v_v_s                            v_u32_i32_sel_less_v_v_v_v

#define v_u32_i32_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_less_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_i32_sel_less_v_s_v_v_vb                         v_u32_i32_sel_less_v_v_v_v_vb
#define v_u32_i32_sel_less_v_v_v_s_vb                         v_u32_i32_sel_less_v_v_v_v_vb

// u32 - f32
#define v_f32_u32_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_less_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_u32_sel_less_v_s_v_v_b                          v_f32_u32_sel_less_v_v_v_v_b
#define v_f32_u32_sel_less_v_v_v_s_b                          v_f32_u32_sel_less_v_v_v_v_b
#define v_f32_u32_sel_less_v_v_v_v(a, b, c, d)                v_f32_u32_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_u32_sel_less_v_s_v_v                            v_f32_u32_sel_less_v_v_v_v
#define v_f32_u32_sel_less_v_v_v_s                            v_f32_u32_sel_less_v_v_v_v

#define v_f32_u32_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_less_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_u32_sel_less_v_s_v_v_vb                         v_f32_u32_sel_less_v_v_v_v_vb
#define v_f32_u32_sel_less_v_v_v_s_vb                         v_f32_u32_sel_less_v_v_v_v_vb

// u32 - i32
#define v_i32_u32_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_less_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_u32_sel_less_v_s_v_v_b                          v_i32_u32_sel_less_v_v_v_v_b
#define v_i32_u32_sel_less_v_v_v_s_b                          v_i32_u32_sel_less_v_v_v_v_b
#define v_i32_u32_sel_less_v_v_v_v(a, b, c, d)                v_i32_u32_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_u32_sel_less_v_s_v_v                            v_i32_u32_sel_less_v_v_v_v
#define v_i32_u32_sel_less_v_v_v_s                            v_i32_u32_sel_less_v_v_v_v

#define v_i32_u32_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_less_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_u32_sel_less_v_s_v_v_vb                         v_i32_u32_sel_less_v_v_v_v_vb
#define v_i32_u32_sel_less_v_v_v_s_vb                         v_i32_u32_sel_less_v_v_v_v_vb

// u32 - u32
#define v_u32_u32_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_less_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_u32_sel_less_v_s_v_v_b                          v_u32_u32_sel_less_v_v_v_v_b
#define v_u32_u32_sel_less_v_v_v_s_b                          v_u32_u32_sel_less_v_v_v_v_b
#define v_u32_u32_sel_less_v_v_v_v(a, b, c, d)                v_u32_u32_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_u32_sel_less_v_s_v_v                            v_u32_u32_sel_less_v_v_v_v
#define v_u32_u32_sel_less_v_v_v_s                            v_u32_u32_sel_less_v_v_v_v

#define v_u32_u32_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_less_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_u32_sel_less_v_s_v_v_vb                         v_u32_u32_sel_less_v_v_v_v_vb
#define v_u32_u32_sel_less_v_v_v_s_vb                         v_u32_u32_sel_less_v_v_v_v_vb

// i16 - bf16
#define v_bf16_i16_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel_less_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_i16_sel_less_v_s_v_v_b                         v_bf16_i16_sel_less_v_v_v_v_b
#define v_bf16_i16_sel_less_v_v_v_s_b                         v_bf16_i16_sel_less_v_v_v_v_b
#define v_bf16_i16_sel_less_v_v_v_v(a, b, c, d)               v_bf16_i16_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_i16_sel_less_v_s_v_v                           v_bf16_i16_sel_less_v_v_v_v
#define v_bf16_i16_sel_less_v_v_v_s                           v_bf16_i16_sel_less_v_v_v_v

#define v_bf16_i16_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel_less_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_i16_sel_less_v_s_v_v_vb                        v_bf16_i16_sel_less_v_v_v_v_vb
#define v_bf16_i16_sel_less_v_v_v_s_vb                        v_bf16_i16_sel_less_v_v_v_v_vb

// i16 - i16
#define v_i16_i16_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)     v_i16_sel_less_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_i16_sel_less_v_s_v_v_b                          v_i16_i16_sel_less_v_v_v_v_b
#define v_i16_i16_sel_less_v_v_v_s_b                          v_i16_i16_sel_less_v_v_v_v_b
#define v_i16_i16_sel_less_v_v_v_v(a, b, c, d)                v_i16_i16_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_i16_sel_less_v_s_v_v                            v_i16_i16_sel_less_v_v_v_v
#define v_i16_i16_sel_less_v_v_v_s                            v_i16_i16_sel_less_v_v_v_v

#define v_i16_i16_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i16_sel_less_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_i16_sel_less_v_s_v_v_vb                         v_i16_i16_sel_less_v_v_v_v_vb
#define v_i16_i16_sel_less_v_v_v_s_vb                         v_i16_i16_sel_less_v_v_v_v_vb

// i16 - u16
#define v_u16_i16_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)     v_i16_sel_less_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_i16_sel_less_v_s_v_v_b                          v_u16_i16_sel_less_v_v_v_v_b
#define v_u16_i16_sel_less_v_v_v_s_b                          v_u16_i16_sel_less_v_v_v_v_b
#define v_u16_i16_sel_less_v_v_v_v(a, b, c, d)                v_u16_i16_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_i16_sel_less_v_s_v_v                            v_u16_i16_sel_less_v_v_v_v
#define v_u16_i16_sel_less_v_v_v_s                            v_u16_i16_sel_less_v_v_v_v

#define v_u16_i16_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i16_sel_less_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_i16_sel_less_v_s_v_v_vb                         v_u16_i16_sel_less_v_v_v_v_vb
#define v_u16_i16_sel_less_v_v_v_s_vb                         v_u16_i16_sel_less_v_v_v_v_vb

// u16 - bf16
#define v_bf16_u16_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel_less_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_u16_sel_less_v_s_v_v_b                         v_bf16_u16_sel_less_v_v_v_v_b
#define v_bf16_u16_sel_less_v_v_v_s_b                         v_bf16_u16_sel_less_v_v_v_v_b
#define v_bf16_u16_sel_less_v_v_v_v(a, b, c, d)               v_bf16_u16_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_u16_sel_less_v_s_v_v                           v_bf16_u16_sel_less_v_v_v_v
#define v_bf16_u16_sel_less_v_v_v_s                           v_bf16_u16_sel_less_v_v_v_v

#define v_bf16_u16_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel_less_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_u16_sel_less_v_s_v_v_vb                        v_bf16_u16_sel_less_v_v_v_v_vb
#define v_bf16_u16_sel_less_v_v_v_s_vb                        v_bf16_u16_sel_less_v_v_v_v_vb

// u16 - i16
#define v_i16_u16_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)     v_u16_sel_less_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_u16_sel_less_v_s_v_v_b                          v_i16_u16_sel_less_v_v_v_v_b
#define v_i16_u16_sel_less_v_v_v_s_b                          v_i16_u16_sel_less_v_v_v_v_b
#define v_i16_u16_sel_less_v_v_v_v(a, b, c, d)                v_i16_u16_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_u16_sel_less_v_s_v_v                            v_i16_u16_sel_less_v_v_v_v
#define v_i16_u16_sel_less_v_v_v_s                            v_i16_u16_sel_less_v_v_v_v

#define v_i16_u16_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u16_sel_less_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_u16_sel_less_v_s_v_v_vb                         v_i16_u16_sel_less_v_v_v_v_vb
#define v_i16_u16_sel_less_v_v_v_s_vb                         v_i16_u16_sel_less_v_v_v_v_vb

// u16 - u16
#define v_u16_u16_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)     v_u16_sel_less_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_u16_sel_less_v_s_v_v_b                          v_u16_u16_sel_less_v_v_v_v_b
#define v_u16_u16_sel_less_v_v_v_s_b                          v_u16_u16_sel_less_v_v_v_v_b
#define v_u16_u16_sel_less_v_v_v_v(a, b, c, d)                v_u16_u16_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_u16_sel_less_v_s_v_v                            v_u16_u16_sel_less_v_v_v_v
#define v_u16_u16_sel_less_v_v_v_s                            v_u16_u16_sel_less_v_v_v_v

#define v_u16_u16_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u16_sel_less_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_u16_sel_less_v_s_v_v_vb                         v_u16_u16_sel_less_v_v_v_v_vb
#define v_u16_u16_sel_less_v_v_v_s_vb                         v_u16_u16_sel_less_v_v_v_v_vb

// i8 - u8
#define v_i8_i8_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)       v_i8_sel_less_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel_less_v_s_v_v_b                            v_i8_i8_sel_less_v_v_v_v_b
#define v_i8_i8_sel_less_v_v_v_s_b                            v_i8_i8_sel_less_v_v_v_v_b
#define v_i8_i8_sel_less_v_v_v_v(a, b, c, d)                  v_i8_i8_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i8_i8_sel_less_v_s_v_v                              v_i8_i8_sel_less_v_v_v_v
#define v_i8_i8_sel_less_v_v_v_s                              v_i8_i8_sel_less_v_v_v_v

#define v_i8_i8_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)      v_i8_sel_less_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel_less_v_s_v_v_vb                           v_i8_i8_sel_less_v_v_v_v_vb
#define v_i8_i8_sel_less_v_v_v_s_vb                           v_i8_i8_sel_less_v_v_v_v_vb

// i8 - u8
#define v_u8_i8_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)       v_i8_sel_less_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel_less_v_s_v_v_b                            v_u8_i8_sel_less_v_v_v_v_b
#define v_u8_i8_sel_less_v_v_v_s_b                            v_u8_i8_sel_less_v_v_v_v_b
#define v_u8_i8_sel_less_v_v_v_v(a, b, c, d)                  v_u8_i8_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u8_i8_sel_less_v_s_v_v                              v_u8_i8_sel_less_v_v_v_v
#define v_u8_i8_sel_less_v_v_v_s                              v_u8_i8_sel_less_v_v_v_v

#define v_u8_i8_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)      v_i8_sel_less_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel_less_v_s_v_v_vb                           v_u8_i8_sel_less_v_v_v_v_vb
#define v_u8_i8_sel_less_v_v_v_s_vb                           v_u8_i8_sel_less_v_v_v_v_vb

// u8 - i8
#define v_i8_u8_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)       v_u8_sel_less_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel_less_v_s_v_v_b                            v_i8_u8_sel_less_v_v_v_v_b
#define v_i8_u8_sel_less_v_v_v_s_b                            v_i8_u8_sel_less_v_v_v_v_b
#define v_i8_u8_sel_less_v_v_v_v(a, b, c, d)                  v_i8_u8_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i8_u8_sel_less_v_s_v_v                              v_i8_u8_sel_less_v_v_v_v
#define v_i8_u8_sel_less_v_v_v_s                              v_i8_u8_sel_less_v_v_v_v

#define v_i8_u8_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)      v_u8_sel_less_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel_less_v_s_v_v_vb                           v_i8_u8_sel_less_v_v_v_v_vb
#define v_i8_u8_sel_less_v_v_v_s_vb                           v_i8_u8_sel_less_v_v_v_v_vb

// u8 - u8
#define v_u8_u8_sel_less_v_v_v_v_b(a, b, c, d, i, p, o)       v_u8_sel_less_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel_less_v_s_v_v_b                            v_u8_u8_sel_less_v_v_v_v_b
#define v_u8_u8_sel_less_v_v_v_s_b                            v_u8_u8_sel_less_v_v_v_v_b
#define v_u8_u8_sel_less_v_v_v_v(a, b, c, d)                  v_u8_u8_sel_less_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u8_u8_sel_less_v_s_v_v                              v_u8_u8_sel_less_v_v_v_v
#define v_u8_u8_sel_less_v_v_v_s                              v_u8_u8_sel_less_v_v_v_v

#define v_u8_u8_sel_less_v_v_v_v_vb(a, b, c, d, i, p, o)      v_u8_sel_less_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel_less_v_s_v_v_vb                           v_u8_u8_sel_less_v_v_v_v_vb
#define v_u8_u8_sel_less_v_v_v_s_vb                           v_u8_u8_sel_less_v_v_v_v_vb


// SEL_LEQ

// f32 - f32
#define v_f32_f32_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_leq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_f32_sel_leq_v_s_v_v_b                          v_f32_f32_sel_leq_v_v_v_v_b
#define v_f32_f32_sel_leq_v_v_v_s_b                          v_f32_f32_sel_leq_v_v_v_v_b
#define v_f32_f32_sel_leq_v_v_v_v(a, b, c, d)                v_f32_f32_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_f32_sel_leq_v_s_v_v                            v_f32_f32_sel_leq_v_v_v_v
#define v_f32_f32_sel_leq_v_v_v_s                            v_f32_f32_sel_leq_v_v_v_v

#define v_f32_f32_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_leq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_f32_sel_leq_v_s_v_v_vb                         v_f32_f32_sel_leq_v_v_v_v_vb
#define v_f32_f32_sel_leq_v_v_v_s_vb                         v_f32_f32_sel_leq_v_v_v_v_vb

// f32 - i32
#define v_i32_f32_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_leq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_f32_sel_leq_v_s_v_v_b                          v_i32_f32_sel_leq_v_v_v_v_b
#define v_i32_f32_sel_leq_v_v_v_s_b                          v_i32_f32_sel_leq_v_v_v_v_b
#define v_i32_f32_sel_leq_v_v_v_v(a, b, c, d)                v_i32_f32_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_f32_sel_leq_v_s_v_v                            v_i32_f32_sel_leq_v_v_v_v
#define v_i32_f32_sel_leq_v_v_v_s                            v_i32_f32_sel_leq_v_v_v_v

#define v_i32_f32_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_leq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_f32_sel_leq_v_s_v_v_vb                         v_i32_f32_sel_leq_v_v_v_v_vb
#define v_i32_f32_sel_leq_v_v_v_s_vb                         v_i32_f32_sel_leq_v_v_v_v_vb

// f32 - u32
#define v_u32_f32_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_leq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_f32_sel_leq_v_s_v_v_b                          v_u32_f32_sel_leq_v_v_v_v_b
#define v_u32_f32_sel_leq_v_v_v_s_b                          v_u32_f32_sel_leq_v_v_v_v_b
#define v_u32_f32_sel_leq_v_v_v_v(a, b, c, d)                v_u32_f32_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_f32_sel_leq_v_s_v_v                            v_u32_f32_sel_leq_v_v_v_v
#define v_u32_f32_sel_leq_v_v_v_s                            v_u32_f32_sel_leq_v_v_v_v

#define v_u32_f32_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_leq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_f32_sel_leq_v_s_v_v_vb                         v_u32_f32_sel_leq_v_v_v_v_vb
#define v_u32_f32_sel_leq_v_v_v_s_vb                         v_u32_f32_sel_leq_v_v_v_v_vb

// bf16 - bf16
#define v_bf16_bf16_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)   v_bf16_sel_leq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_bf16_sel_leq_v_s_v_v_b                        v_bf16_bf16_sel_leq_v_v_v_v_b
#define v_bf16_bf16_sel_leq_v_v_v_s_b                        v_bf16_bf16_sel_leq_v_v_v_v_b
#define v_bf16_bf16_sel_leq_v_v_v_v(a, b, c, d)              v_bf16_bf16_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_bf16_sel_leq_v_s_v_v                          v_bf16_bf16_sel_leq_v_v_v_v
#define v_bf16_bf16_sel_leq_v_v_v_s                          v_bf16_bf16_sel_leq_v_v_v_v

#define v_bf16_bf16_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)  v_bf16_sel_leq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_bf16_sel_leq_v_s_v_v_vb                       v_bf16_bf16_sel_leq_v_v_v_v_vb
#define v_bf16_bf16_sel_leq_v_v_v_s_vb                       v_bf16_bf16_sel_leq_v_v_v_v_vb

// bf16 - i16
#define v_i16_bf16_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel_leq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_bf16_sel_leq_v_s_v_v_b                         v_i16_bf16_sel_leq_v_v_v_v_b
#define v_i16_bf16_sel_leq_v_v_v_s_b                         v_i16_bf16_sel_leq_v_v_v_v_b
#define v_i16_bf16_sel_leq_v_v_v_v(a, b, c, d)               v_i16_bf16_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_bf16_sel_leq_v_s_v_v                           v_i16_bf16_sel_leq_v_v_v_v
#define v_i16_bf16_sel_leq_v_v_v_s                           v_i16_bf16_sel_leq_v_v_v_v

#define v_i16_bf16_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel_leq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_bf16_sel_leq_v_s_v_v_vb                        v_i16_bf16_sel_leq_v_v_v_v_vb
#define v_i16_bf16_sel_leq_v_v_v_s_vb                        v_i16_bf16_sel_leq_v_v_v_v_vb

// bf16 - u16
#define v_u16_bf16_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel_leq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_bf16_sel_leq_v_s_v_v_b                         v_u16_bf16_sel_leq_v_v_v_v_b
#define v_u16_bf16_sel_leq_v_v_v_s_b                         v_u16_bf16_sel_leq_v_v_v_v_b
#define v_u16_bf16_sel_leq_v_v_v_v(a, b, c, d)               v_u16_bf16_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_bf16_sel_leq_v_s_v_v                           v_u16_bf16_sel_leq_v_v_v_v
#define v_u16_bf16_sel_leq_v_v_v_s                           v_u16_bf16_sel_leq_v_v_v_v

#define v_u16_bf16_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel_leq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_bf16_sel_leq_v_s_v_v_vb                        v_u16_bf16_sel_leq_v_v_v_v_vb
#define v_u16_bf16_sel_leq_v_v_v_s_vb                        v_u16_bf16_sel_leq_v_v_v_v_vb

// i32 - f32
#define v_f32_i32_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_leq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_i32_sel_leq_v_s_v_v_b                          v_f32_i32_sel_leq_v_v_v_v_b
#define v_f32_i32_sel_leq_v_v_v_s_b                          v_f32_i32_sel_leq_v_v_v_v_b
#define v_f32_i32_sel_leq_v_v_v_v(a, b, c, d)                v_f32_i32_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_i32_sel_leq_v_s_v_v                            v_f32_i32_sel_leq_v_v_v_v
#define v_f32_i32_sel_leq_v_v_v_s                            v_f32_i32_sel_leq_v_v_v_v

#define v_f32_i32_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_leq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_i32_sel_leq_v_s_v_v_vb                         v_f32_i32_sel_leq_v_v_v_v_vb
#define v_f32_i32_sel_leq_v_v_v_s_vb                         v_f32_i32_sel_leq_v_v_v_v_vb

// i32 - i32
#define v_i32_i32_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_leq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_i32_sel_leq_v_s_v_v_b                          v_i32_i32_sel_leq_v_v_v_v_b
#define v_i32_i32_sel_leq_v_v_v_s_b                          v_i32_i32_sel_leq_v_v_v_v_b
#define v_i32_i32_sel_leq_v_v_v_v(a, b, c, d)                v_i32_i32_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_i32_sel_leq_v_s_v_v                            v_i32_i32_sel_leq_v_v_v_v
#define v_i32_i32_sel_leq_v_v_v_s                            v_i32_i32_sel_leq_v_v_v_v

#define v_i32_i32_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_leq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_i32_sel_leq_v_s_v_v_vb                         v_i32_i32_sel_leq_v_v_v_v_vb
#define v_i32_i32_sel_leq_v_v_v_s_vb                         v_i32_i32_sel_leq_v_v_v_v_vb

// i32 - u32
#define v_u32_i32_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_leq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_i32_sel_leq_v_s_v_v_b                          v_u32_i32_sel_leq_v_v_v_v_b
#define v_u32_i32_sel_leq_v_v_v_s_b                          v_u32_i32_sel_leq_v_v_v_v_b
#define v_u32_i32_sel_leq_v_v_v_v(a, b, c, d)                v_u32_i32_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_i32_sel_leq_v_s_v_v                            v_u32_i32_sel_leq_v_v_v_v
#define v_u32_i32_sel_leq_v_v_v_s                            v_u32_i32_sel_leq_v_v_v_v

#define v_u32_i32_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_leq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_i32_sel_leq_v_s_v_v_vb                         v_u32_i32_sel_leq_v_v_v_v_vb
#define v_u32_i32_sel_leq_v_v_v_s_vb                         v_u32_i32_sel_leq_v_v_v_v_vb

// u32 - f32
#define v_f32_u32_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_leq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_u32_sel_leq_v_s_v_v_b                          v_f32_u32_sel_leq_v_v_v_v_b
#define v_f32_u32_sel_leq_v_v_v_s_b                          v_f32_u32_sel_leq_v_v_v_v_b
#define v_f32_u32_sel_leq_v_v_v_v(a, b, c, d)                v_f32_u32_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_u32_sel_leq_v_s_v_v                            v_f32_u32_sel_leq_v_v_v_v
#define v_f32_u32_sel_leq_v_v_v_s                            v_f32_u32_sel_leq_v_v_v_v

#define v_f32_u32_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_leq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_u32_sel_leq_v_s_v_v_vb                         v_f32_u32_sel_leq_v_v_v_v_vb
#define v_f32_u32_sel_leq_v_v_v_s_vb                         v_f32_u32_sel_leq_v_v_v_v_vb

// u32 - i32
#define v_i32_u32_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_leq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_u32_sel_leq_v_s_v_v_b                          v_i32_u32_sel_leq_v_v_v_v_b
#define v_i32_u32_sel_leq_v_v_v_s_b                          v_i32_u32_sel_leq_v_v_v_v_b
#define v_i32_u32_sel_leq_v_v_v_v(a, b, c, d)                v_i32_u32_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_u32_sel_leq_v_s_v_v                            v_i32_u32_sel_leq_v_v_v_v
#define v_i32_u32_sel_leq_v_v_v_s                            v_i32_u32_sel_leq_v_v_v_v

#define v_i32_u32_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_leq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_u32_sel_leq_v_s_v_v_vb                         v_i32_u32_sel_leq_v_v_v_v_vb
#define v_i32_u32_sel_leq_v_v_v_s_vb                         v_i32_u32_sel_leq_v_v_v_v_vb

// u32 - u32
#define v_u32_u32_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_leq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_u32_sel_leq_v_s_v_v_b                          v_u32_u32_sel_leq_v_v_v_v_b
#define v_u32_u32_sel_leq_v_v_v_s_b                          v_u32_u32_sel_leq_v_v_v_v_b
#define v_u32_u32_sel_leq_v_v_v_v(a, b, c, d)                v_u32_u32_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_u32_sel_leq_v_s_v_v                            v_u32_u32_sel_leq_v_v_v_v
#define v_u32_u32_sel_leq_v_v_v_s                            v_u32_u32_sel_leq_v_v_v_v

#define v_u32_u32_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_leq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_u32_sel_leq_v_s_v_v_vb                         v_u32_u32_sel_leq_v_v_v_v_vb
#define v_u32_u32_sel_leq_v_v_v_s_vb                         v_u32_u32_sel_leq_v_v_v_v_vb

// i16 - bf16
#define v_bf16_i16_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel_leq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_i16_sel_leq_v_s_v_v_b                         v_bf16_i16_sel_leq_v_v_v_v_b
#define v_bf16_i16_sel_leq_v_v_v_s_b                         v_bf16_i16_sel_leq_v_v_v_v_b
#define v_bf16_i16_sel_leq_v_v_v_v(a, b, c, d)               v_bf16_i16_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_i16_sel_leq_v_s_v_v                           v_bf16_i16_sel_leq_v_v_v_v
#define v_bf16_i16_sel_leq_v_v_v_s                           v_bf16_i16_sel_leq_v_v_v_v

#define v_bf16_i16_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel_leq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_i16_sel_leq_v_s_v_v_vb                        v_bf16_i16_sel_leq_v_v_v_v_vb
#define v_bf16_i16_sel_leq_v_v_v_s_vb                        v_bf16_i16_sel_leq_v_v_v_v_vb

// i16 - i16
#define v_i16_i16_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i16_sel_leq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_i16_sel_leq_v_s_v_v_b                          v_i16_i16_sel_leq_v_v_v_v_b
#define v_i16_i16_sel_leq_v_v_v_s_b                          v_i16_i16_sel_leq_v_v_v_v_b
#define v_i16_i16_sel_leq_v_v_v_v(a, b, c, d)                v_i16_i16_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_i16_sel_leq_v_s_v_v                            v_i16_i16_sel_leq_v_v_v_v
#define v_i16_i16_sel_leq_v_v_v_s                            v_i16_i16_sel_leq_v_v_v_v

#define v_i16_i16_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i16_sel_leq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_i16_sel_leq_v_s_v_v_vb                         v_i16_i16_sel_leq_v_v_v_v_vb
#define v_i16_i16_sel_leq_v_v_v_s_vb                         v_i16_i16_sel_leq_v_v_v_v_vb

// i16 - u16
#define v_u16_i16_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i16_sel_leq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_i16_sel_leq_v_s_v_v_b                          v_u16_i16_sel_leq_v_v_v_v_b
#define v_u16_i16_sel_leq_v_v_v_s_b                          v_u16_i16_sel_leq_v_v_v_v_b
#define v_u16_i16_sel_leq_v_v_v_v(a, b, c, d)                v_u16_i16_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_i16_sel_leq_v_s_v_v                            v_u16_i16_sel_leq_v_v_v_v
#define v_u16_i16_sel_leq_v_v_v_s                            v_u16_i16_sel_leq_v_v_v_v

#define v_u16_i16_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i16_sel_leq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_i16_sel_leq_v_s_v_v_vb                         v_u16_i16_sel_leq_v_v_v_v_vb
#define v_u16_i16_sel_leq_v_v_v_s_vb                         v_u16_i16_sel_leq_v_v_v_v_vb

// u16 - bf16
#define v_bf16_u16_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel_leq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_u16_sel_leq_v_s_v_v_b                         v_bf16_u16_sel_leq_v_v_v_v_b
#define v_bf16_u16_sel_leq_v_v_v_s_b                         v_bf16_u16_sel_leq_v_v_v_v_b
#define v_bf16_u16_sel_leq_v_v_v_v(a, b, c, d)               v_bf16_u16_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_u16_sel_leq_v_s_v_v                           v_bf16_u16_sel_leq_v_v_v_v
#define v_bf16_u16_sel_leq_v_v_v_s                           v_bf16_u16_sel_leq_v_v_v_v

#define v_bf16_u16_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel_leq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_u16_sel_leq_v_s_v_v_vb                        v_bf16_u16_sel_leq_v_v_v_v_vb
#define v_bf16_u16_sel_leq_v_v_v_s_vb                        v_bf16_u16_sel_leq_v_v_v_v_vb

// u16 - i16
#define v_i16_u16_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u16_sel_leq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_u16_sel_leq_v_s_v_v_b                          v_i16_u16_sel_leq_v_v_v_v_b
#define v_i16_u16_sel_leq_v_v_v_s_b                          v_i16_u16_sel_leq_v_v_v_v_b
#define v_i16_u16_sel_leq_v_v_v_v(a, b, c, d)                v_i16_u16_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_u16_sel_leq_v_s_v_v                            v_i16_u16_sel_leq_v_v_v_v
#define v_i16_u16_sel_leq_v_v_v_s                            v_i16_u16_sel_leq_v_v_v_v

#define v_i16_u16_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u16_sel_leq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_u16_sel_leq_v_s_v_v_vb                         v_i16_u16_sel_leq_v_v_v_v_vb
#define v_i16_u16_sel_leq_v_v_v_s_vb                         v_i16_u16_sel_leq_v_v_v_v_vb

// u16 - u16
#define v_u16_u16_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u16_sel_leq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_u16_sel_leq_v_s_v_v_b                          v_u16_u16_sel_leq_v_v_v_v_b
#define v_u16_u16_sel_leq_v_v_v_s_b                          v_u16_u16_sel_leq_v_v_v_v_b
#define v_u16_u16_sel_leq_v_v_v_v(a, b, c, d)                v_u16_u16_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_u16_sel_leq_v_s_v_v                            v_u16_u16_sel_leq_v_v_v_v
#define v_u16_u16_sel_leq_v_v_v_s                            v_u16_u16_sel_leq_v_v_v_v

#define v_u16_u16_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u16_sel_leq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_u16_sel_leq_v_s_v_v_vb                         v_u16_u16_sel_leq_v_v_v_v_vb
#define v_u16_u16_sel_leq_v_v_v_s_vb                         v_u16_u16_sel_leq_v_v_v_v_vb

// i8 - u8
#define v_i8_i8_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)       v_i8_sel_leq_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel_leq_v_s_v_v_b                            v_i8_i8_sel_leq_v_v_v_v_b
#define v_i8_i8_sel_leq_v_v_v_s_b                            v_i8_i8_sel_leq_v_v_v_v_b
#define v_i8_i8_sel_leq_v_v_v_v(a, b, c, d)                  v_i8_i8_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i8_i8_sel_leq_v_s_v_v                              v_i8_i8_sel_leq_v_v_v_v
#define v_i8_i8_sel_leq_v_v_v_s                              v_i8_i8_sel_leq_v_v_v_v

#define v_i8_i8_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_i8_sel_leq_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel_leq_v_s_v_v_vb                           v_i8_i8_sel_leq_v_v_v_v_vb
#define v_i8_i8_sel_leq_v_v_v_s_vb                           v_i8_i8_sel_leq_v_v_v_v_vb

// i8 - u8
#define v_u8_i8_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)       v_i8_sel_leq_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel_leq_v_s_v_v_b                            v_u8_i8_sel_leq_v_v_v_v_b
#define v_u8_i8_sel_leq_v_v_v_s_b                            v_u8_i8_sel_leq_v_v_v_v_b
#define v_u8_i8_sel_leq_v_v_v_v(a, b, c, d)                  v_u8_i8_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u8_i8_sel_leq_v_s_v_v                              v_u8_i8_sel_leq_v_v_v_v
#define v_u8_i8_sel_leq_v_v_v_s                              v_u8_i8_sel_leq_v_v_v_v

#define v_u8_i8_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_i8_sel_leq_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel_leq_v_s_v_v_vb                           v_u8_i8_sel_leq_v_v_v_v_vb
#define v_u8_i8_sel_leq_v_v_v_s_vb                           v_u8_i8_sel_leq_v_v_v_v_vb

// u8 - i8
#define v_i8_u8_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)       v_u8_sel_leq_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel_leq_v_s_v_v_b                            v_i8_u8_sel_leq_v_v_v_v_b
#define v_i8_u8_sel_leq_v_v_v_s_b                            v_i8_u8_sel_leq_v_v_v_v_b
#define v_i8_u8_sel_leq_v_v_v_v(a, b, c, d)                  v_i8_u8_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i8_u8_sel_leq_v_s_v_v                              v_i8_u8_sel_leq_v_v_v_v
#define v_i8_u8_sel_leq_v_v_v_s                              v_i8_u8_sel_leq_v_v_v_v

#define v_i8_u8_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_u8_sel_leq_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel_leq_v_s_v_v_vb                           v_i8_u8_sel_leq_v_v_v_v_vb
#define v_i8_u8_sel_leq_v_v_v_s_vb                           v_i8_u8_sel_leq_v_v_v_v_vb

// u8 - u8
#define v_u8_u8_sel_leq_v_v_v_v_b(a, b, c, d, i, p, o)       v_u8_sel_leq_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel_leq_v_s_v_v_b                            v_u8_u8_sel_leq_v_v_v_v_b
#define v_u8_u8_sel_leq_v_v_v_s_b                            v_u8_u8_sel_leq_v_v_v_v_b
#define v_u8_u8_sel_leq_v_v_v_v(a, b, c, d)                  v_u8_u8_sel_leq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u8_u8_sel_leq_v_s_v_v                              v_u8_u8_sel_leq_v_v_v_v
#define v_u8_u8_sel_leq_v_v_v_s                              v_u8_u8_sel_leq_v_v_v_v

#define v_u8_u8_sel_leq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_u8_sel_leq_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel_leq_v_s_v_v_vb                           v_u8_u8_sel_leq_v_v_v_v_vb
#define v_u8_u8_sel_leq_v_v_v_s_vb                           v_u8_u8_sel_leq_v_v_v_v_vb

// SEL_GRT

// f32 - f32
#define v_f32_f32_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_grt_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_f32_sel_grt_v_s_v_v_b                          v_f32_f32_sel_grt_v_v_v_v_b
#define v_f32_f32_sel_grt_v_v_v_s_b                          v_f32_f32_sel_grt_v_v_v_v_b
#define v_f32_f32_sel_grt_v_v_v_v(a, b, c, d)                v_f32_f32_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_f32_sel_grt_v_s_v_v                            v_f32_f32_sel_grt_v_v_v_v
#define v_f32_f32_sel_grt_v_v_v_s                            v_f32_f32_sel_grt_v_v_v_v

#define v_f32_f32_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_grt_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_f32_sel_grt_v_s_v_v_vb                         v_f32_f32_sel_grt_v_v_v_v_vb
#define v_f32_f32_sel_grt_v_v_v_s_vb                         v_f32_f32_sel_grt_v_v_v_v_vb

// f32 - i32
#define v_i32_f32_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_grt_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_f32_sel_grt_v_s_v_v_b                          v_i32_f32_sel_grt_v_v_v_v_b
#define v_i32_f32_sel_grt_v_v_v_s_b                          v_i32_f32_sel_grt_v_v_v_v_b
#define v_i32_f32_sel_grt_v_v_v_v(a, b, c, d)                v_i32_f32_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_f32_sel_grt_v_s_v_v                            v_i32_f32_sel_grt_v_v_v_v
#define v_i32_f32_sel_grt_v_v_v_s                            v_i32_f32_sel_grt_v_v_v_v

#define v_i32_f32_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_grt_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_f32_sel_grt_v_s_v_v_vb                         v_i32_f32_sel_grt_v_v_v_v_vb
#define v_i32_f32_sel_grt_v_v_v_s_vb                         v_i32_f32_sel_grt_v_v_v_v_vb

// f32 - u32
#define v_u32_f32_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_grt_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_f32_sel_grt_v_s_v_v_b                          v_u32_f32_sel_grt_v_v_v_v_b
#define v_u32_f32_sel_grt_v_v_v_s_b                          v_u32_f32_sel_grt_v_v_v_v_b
#define v_u32_f32_sel_grt_v_v_v_v(a, b, c, d)                v_u32_f32_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_f32_sel_grt_v_s_v_v                            v_u32_f32_sel_grt_v_v_v_v
#define v_u32_f32_sel_grt_v_v_v_s                            v_u32_f32_sel_grt_v_v_v_v

#define v_u32_f32_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_grt_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_f32_sel_grt_v_s_v_v_vb                         v_u32_f32_sel_grt_v_v_v_v_vb
#define v_u32_f32_sel_grt_v_v_v_s_vb                         v_u32_f32_sel_grt_v_v_v_v_vb

// bf16 - bf16
#define v_bf16_bf16_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)   v_bf16_sel_grt_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_bf16_sel_grt_v_s_v_v_b                        v_bf16_bf16_sel_grt_v_v_v_v_b
#define v_bf16_bf16_sel_grt_v_v_v_s_b                        v_bf16_bf16_sel_grt_v_v_v_v_b
#define v_bf16_bf16_sel_grt_v_v_v_v(a, b, c, d)              v_bf16_bf16_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_bf16_sel_grt_v_s_v_v                          v_bf16_bf16_sel_grt_v_v_v_v
#define v_bf16_bf16_sel_grt_v_v_v_s                          v_bf16_bf16_sel_grt_v_v_v_v

#define v_bf16_bf16_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)  v_bf16_sel_grt_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_bf16_sel_grt_v_s_v_v_vb                       v_bf16_bf16_sel_grt_v_v_v_v_vb
#define v_bf16_bf16_sel_grt_v_v_v_s_vb                       v_bf16_bf16_sel_grt_v_v_v_v_vb

// bf16 - i16
#define v_i16_bf16_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel_grt_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_bf16_sel_grt_v_s_v_v_b                         v_i16_bf16_sel_grt_v_v_v_v_b
#define v_i16_bf16_sel_grt_v_v_v_s_b                         v_i16_bf16_sel_grt_v_v_v_v_b
#define v_i16_bf16_sel_grt_v_v_v_v(a, b, c, d)               v_i16_bf16_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_bf16_sel_grt_v_s_v_v                           v_i16_bf16_sel_grt_v_v_v_v
#define v_i16_bf16_sel_grt_v_v_v_s                           v_i16_bf16_sel_grt_v_v_v_v

#define v_i16_bf16_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel_grt_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_bf16_sel_grt_v_s_v_v_vb                        v_i16_bf16_sel_grt_v_v_v_v_vb
#define v_i16_bf16_sel_grt_v_v_v_s_vb                        v_i16_bf16_sel_grt_v_v_v_v_vb

// bf16 - u16
#define v_u16_bf16_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel_grt_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_bf16_sel_grt_v_s_v_v_b                         v_u16_bf16_sel_grt_v_v_v_v_b
#define v_u16_bf16_sel_grt_v_v_v_s_b                         v_u16_bf16_sel_grt_v_v_v_v_b
#define v_u16_bf16_sel_grt_v_v_v_v(a, b, c, d)               v_u16_bf16_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_bf16_sel_grt_v_s_v_v                           v_u16_bf16_sel_grt_v_v_v_v
#define v_u16_bf16_sel_grt_v_v_v_s                           v_u16_bf16_sel_grt_v_v_v_v

#define v_u16_bf16_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel_grt_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_bf16_sel_grt_v_s_v_v_vb                        v_u16_bf16_sel_grt_v_v_v_v_vb
#define v_u16_bf16_sel_grt_v_v_v_s_vb                        v_u16_bf16_sel_grt_v_v_v_v_vb

// i32 - f32
#define v_f32_i32_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_grt_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_i32_sel_grt_v_s_v_v_b                          v_f32_i32_sel_grt_v_v_v_v_b
#define v_f32_i32_sel_grt_v_v_v_s_b                          v_f32_i32_sel_grt_v_v_v_v_b
#define v_f32_i32_sel_grt_v_v_v_v(a, b, c, d)                v_f32_i32_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_i32_sel_grt_v_s_v_v                            v_f32_i32_sel_grt_v_v_v_v
#define v_f32_i32_sel_grt_v_v_v_s                            v_f32_i32_sel_grt_v_v_v_v

#define v_f32_i32_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_grt_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_i32_sel_grt_v_s_v_v_vb                         v_f32_i32_sel_grt_v_v_v_v_vb
#define v_f32_i32_sel_grt_v_v_v_s_vb                         v_f32_i32_sel_grt_v_v_v_v_vb

// i32 - i32
#define v_i32_i32_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_grt_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_i32_sel_grt_v_s_v_v_b                          v_i32_i32_sel_grt_v_v_v_v_b
#define v_i32_i32_sel_grt_v_v_v_s_b                          v_i32_i32_sel_grt_v_v_v_v_b
#define v_i32_i32_sel_grt_v_v_v_v(a, b, c, d)                v_i32_i32_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_i32_sel_grt_v_s_v_v                            v_i32_i32_sel_grt_v_v_v_v
#define v_i32_i32_sel_grt_v_v_v_s                            v_i32_i32_sel_grt_v_v_v_v

#define v_i32_i32_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_grt_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_i32_sel_grt_v_s_v_v_vb                         v_i32_i32_sel_grt_v_v_v_v_vb
#define v_i32_i32_sel_grt_v_v_v_s_vb                         v_i32_i32_sel_grt_v_v_v_v_vb

// i32 - u32
#define v_u32_i32_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_grt_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_i32_sel_grt_v_s_v_v_b                          v_u32_i32_sel_grt_v_v_v_v_b
#define v_u32_i32_sel_grt_v_v_v_s_b                          v_u32_i32_sel_grt_v_v_v_v_b
#define v_u32_i32_sel_grt_v_v_v_v(a, b, c, d)                v_u32_i32_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_i32_sel_grt_v_s_v_v                            v_u32_i32_sel_grt_v_v_v_v
#define v_u32_i32_sel_grt_v_v_v_s                            v_u32_i32_sel_grt_v_v_v_v

#define v_u32_i32_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_grt_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_i32_sel_grt_v_s_v_v_vb                         v_u32_i32_sel_grt_v_v_v_v_vb
#define v_u32_i32_sel_grt_v_v_v_s_vb                         v_u32_i32_sel_grt_v_v_v_v_vb

// u32 - f32
#define v_f32_u32_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_grt_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_u32_sel_grt_v_s_v_v_b                          v_f32_u32_sel_grt_v_v_v_v_b
#define v_f32_u32_sel_grt_v_v_v_s_b                          v_f32_u32_sel_grt_v_v_v_v_b
#define v_f32_u32_sel_grt_v_v_v_v(a, b, c, d)                v_f32_u32_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_u32_sel_grt_v_s_v_v                            v_f32_u32_sel_grt_v_v_v_v
#define v_f32_u32_sel_grt_v_v_v_s                            v_f32_u32_sel_grt_v_v_v_v

#define v_f32_u32_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_grt_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_u32_sel_grt_v_s_v_v_vb                         v_f32_u32_sel_grt_v_v_v_v_vb
#define v_f32_u32_sel_grt_v_v_v_s_vb                         v_f32_u32_sel_grt_v_v_v_v_vb

// u32 - i32
#define v_i32_u32_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_grt_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_u32_sel_grt_v_s_v_v_b                          v_i32_u32_sel_grt_v_v_v_v_b
#define v_i32_u32_sel_grt_v_v_v_s_b                          v_i32_u32_sel_grt_v_v_v_v_b
#define v_i32_u32_sel_grt_v_v_v_v(a, b, c, d)                v_i32_u32_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_u32_sel_grt_v_s_v_v                            v_i32_u32_sel_grt_v_v_v_v
#define v_i32_u32_sel_grt_v_v_v_s                            v_i32_u32_sel_grt_v_v_v_v

#define v_i32_u32_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_grt_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_u32_sel_grt_v_s_v_v_vb                         v_i32_u32_sel_grt_v_v_v_v_vb
#define v_i32_u32_sel_grt_v_v_v_s_vb                         v_i32_u32_sel_grt_v_v_v_v_vb

// u32 - u32
#define v_u32_u32_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_grt_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_u32_sel_grt_v_s_v_v_b                          v_u32_u32_sel_grt_v_v_v_v_b
#define v_u32_u32_sel_grt_v_v_v_s_b                          v_u32_u32_sel_grt_v_v_v_v_b
#define v_u32_u32_sel_grt_v_v_v_v(a, b, c, d)                v_u32_u32_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_u32_sel_grt_v_s_v_v                            v_u32_u32_sel_grt_v_v_v_v
#define v_u32_u32_sel_grt_v_v_v_s                            v_u32_u32_sel_grt_v_v_v_v

#define v_u32_u32_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_grt_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_u32_sel_grt_v_s_v_v_vb                         v_u32_u32_sel_grt_v_v_v_v_vb
#define v_u32_u32_sel_grt_v_v_v_s_vb                         v_u32_u32_sel_grt_v_v_v_v_vb

// i16 - bf16
#define v_bf16_i16_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel_grt_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_i16_sel_grt_v_s_v_v_b                         v_bf16_i16_sel_grt_v_v_v_v_b
#define v_bf16_i16_sel_grt_v_v_v_s_b                         v_bf16_i16_sel_grt_v_v_v_v_b
#define v_bf16_i16_sel_grt_v_v_v_v(a, b, c, d)               v_bf16_i16_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_i16_sel_grt_v_s_v_v                           v_bf16_i16_sel_grt_v_v_v_v
#define v_bf16_i16_sel_grt_v_v_v_s                           v_bf16_i16_sel_grt_v_v_v_v

#define v_bf16_i16_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel_grt_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_i16_sel_grt_v_s_v_v_vb                        v_bf16_i16_sel_grt_v_v_v_v_vb
#define v_bf16_i16_sel_grt_v_v_v_s_vb                        v_bf16_i16_sel_grt_v_v_v_v_vb

// i16 - i16
#define v_i16_i16_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)     v_i16_sel_grt_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_i16_sel_grt_v_s_v_v_b                          v_i16_i16_sel_grt_v_v_v_v_b
#define v_i16_i16_sel_grt_v_v_v_s_b                          v_i16_i16_sel_grt_v_v_v_v_b
#define v_i16_i16_sel_grt_v_v_v_v(a, b, c, d)                v_i16_i16_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_i16_sel_grt_v_s_v_v                            v_i16_i16_sel_grt_v_v_v_v
#define v_i16_i16_sel_grt_v_v_v_s                            v_i16_i16_sel_grt_v_v_v_v

#define v_i16_i16_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i16_sel_grt_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_i16_sel_grt_v_s_v_v_vb                         v_i16_i16_sel_grt_v_v_v_v_vb
#define v_i16_i16_sel_grt_v_v_v_s_vb                         v_i16_i16_sel_grt_v_v_v_v_vb

// i16 - u16
#define v_u16_i16_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)     v_i16_sel_grt_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_i16_sel_grt_v_s_v_v_b                          v_u16_i16_sel_grt_v_v_v_v_b
#define v_u16_i16_sel_grt_v_v_v_s_b                          v_u16_i16_sel_grt_v_v_v_v_b
#define v_u16_i16_sel_grt_v_v_v_v(a, b, c, d)                v_u16_i16_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_i16_sel_grt_v_s_v_v                            v_u16_i16_sel_grt_v_v_v_v
#define v_u16_i16_sel_grt_v_v_v_s                            v_u16_i16_sel_grt_v_v_v_v

#define v_u16_i16_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i16_sel_grt_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_i16_sel_grt_v_s_v_v_vb                         v_u16_i16_sel_grt_v_v_v_v_vb
#define v_u16_i16_sel_grt_v_v_v_s_vb                         v_u16_i16_sel_grt_v_v_v_v_vb

// u16 - bf16
#define v_bf16_u16_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel_grt_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_u16_sel_grt_v_s_v_v_b                         v_bf16_u16_sel_grt_v_v_v_v_b
#define v_bf16_u16_sel_grt_v_v_v_s_b                         v_bf16_u16_sel_grt_v_v_v_v_b
#define v_bf16_u16_sel_grt_v_v_v_v(a, b, c, d)               v_bf16_u16_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_u16_sel_grt_v_s_v_v                           v_bf16_u16_sel_grt_v_v_v_v
#define v_bf16_u16_sel_grt_v_v_v_s                           v_bf16_u16_sel_grt_v_v_v_v

#define v_bf16_u16_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel_grt_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_u16_sel_grt_v_s_v_v_vb                        v_bf16_u16_sel_grt_v_v_v_v_vb
#define v_bf16_u16_sel_grt_v_v_v_s_vb                        v_bf16_u16_sel_grt_v_v_v_v_vb

// u16 - i16
#define v_i16_u16_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)     v_u16_sel_grt_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_u16_sel_grt_v_s_v_v_b                          v_i16_u16_sel_grt_v_v_v_v_b
#define v_i16_u16_sel_grt_v_v_v_s_b                          v_i16_u16_sel_grt_v_v_v_v_b
#define v_i16_u16_sel_grt_v_v_v_v(a, b, c, d)                v_i16_u16_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_u16_sel_grt_v_s_v_v                            v_i16_u16_sel_grt_v_v_v_v
#define v_i16_u16_sel_grt_v_v_v_s                            v_i16_u16_sel_grt_v_v_v_v

#define v_i16_u16_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u16_sel_grt_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_u16_sel_grt_v_s_v_v_vb                         v_i16_u16_sel_grt_v_v_v_v_vb
#define v_i16_u16_sel_grt_v_v_v_s_vb                         v_i16_u16_sel_grt_v_v_v_v_vb

// u16 - u16
#define v_u16_u16_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)     v_u16_sel_grt_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_u16_sel_grt_v_s_v_v_b                          v_u16_u16_sel_grt_v_v_v_v_b
#define v_u16_u16_sel_grt_v_v_v_s_b                          v_u16_u16_sel_grt_v_v_v_v_b
#define v_u16_u16_sel_grt_v_v_v_v(a, b, c, d)                v_u16_u16_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_u16_sel_grt_v_s_v_v                            v_u16_u16_sel_grt_v_v_v_v
#define v_u16_u16_sel_grt_v_v_v_s                            v_u16_u16_sel_grt_v_v_v_v

#define v_u16_u16_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u16_sel_grt_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_u16_sel_grt_v_s_v_v_vb                         v_u16_u16_sel_grt_v_v_v_v_vb
#define v_u16_u16_sel_grt_v_v_v_s_vb                         v_u16_u16_sel_grt_v_v_v_v_vb

// i8 - u8
#define v_i8_i8_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)       v_i8_sel_grt_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel_grt_v_s_v_v_b                            v_i8_i8_sel_grt_v_v_v_v_b
#define v_i8_i8_sel_grt_v_v_v_s_b                            v_i8_i8_sel_grt_v_v_v_v_b
#define v_i8_i8_sel_grt_v_v_v_v(a, b, c, d)                  v_i8_i8_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i8_i8_sel_grt_v_s_v_v                              v_i8_i8_sel_grt_v_v_v_v
#define v_i8_i8_sel_grt_v_v_v_s                              v_i8_i8_sel_grt_v_v_v_v

#define v_i8_i8_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)      v_i8_sel_grt_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel_grt_v_s_v_v_vb                           v_i8_i8_sel_grt_v_v_v_v_vb
#define v_i8_i8_sel_grt_v_v_v_s_vb                           v_i8_i8_sel_grt_v_v_v_v_vb

// i8 - u8
#define v_u8_i8_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)       v_i8_sel_grt_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel_grt_v_s_v_v_b                            v_u8_i8_sel_grt_v_v_v_v_b
#define v_u8_i8_sel_grt_v_v_v_s_b                            v_u8_i8_sel_grt_v_v_v_v_b
#define v_u8_i8_sel_grt_v_v_v_v(a, b, c, d)                  v_u8_i8_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u8_i8_sel_grt_v_s_v_v                              v_u8_i8_sel_grt_v_v_v_v
#define v_u8_i8_sel_grt_v_v_v_s                              v_u8_i8_sel_grt_v_v_v_v

#define v_u8_i8_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)      v_i8_sel_grt_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel_grt_v_s_v_v_vb                           v_u8_i8_sel_grt_v_v_v_v_vb
#define v_u8_i8_sel_grt_v_v_v_s_vb                           v_u8_i8_sel_grt_v_v_v_v_vb

// u8 - i8
#define v_i8_u8_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)       v_u8_sel_grt_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel_grt_v_s_v_v_b                            v_i8_u8_sel_grt_v_v_v_v_b
#define v_i8_u8_sel_grt_v_v_v_s_b                            v_i8_u8_sel_grt_v_v_v_v_b
#define v_i8_u8_sel_grt_v_v_v_v(a, b, c, d)                  v_i8_u8_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i8_u8_sel_grt_v_s_v_v                              v_i8_u8_sel_grt_v_v_v_v
#define v_i8_u8_sel_grt_v_v_v_s                              v_i8_u8_sel_grt_v_v_v_v

#define v_i8_u8_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)      v_u8_sel_grt_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel_grt_v_s_v_v_vb                           v_i8_u8_sel_grt_v_v_v_v_vb
#define v_i8_u8_sel_grt_v_v_v_s_vb                           v_i8_u8_sel_grt_v_v_v_v_vb

// u8 - u8
#define v_u8_u8_sel_grt_v_v_v_v_b(a, b, c, d, i, p, o)       v_u8_sel_grt_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel_grt_v_s_v_v_b                            v_u8_u8_sel_grt_v_v_v_v_b
#define v_u8_u8_sel_grt_v_v_v_s_b                            v_u8_u8_sel_grt_v_v_v_v_b
#define v_u8_u8_sel_grt_v_v_v_v(a, b, c, d)                  v_u8_u8_sel_grt_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u8_u8_sel_grt_v_s_v_v                              v_u8_u8_sel_grt_v_v_v_v
#define v_u8_u8_sel_grt_v_v_v_s                              v_u8_u8_sel_grt_v_v_v_v

#define v_u8_u8_sel_grt_v_v_v_v_vb(a, b, c, d, i, p, o)      v_u8_sel_grt_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel_grt_v_s_v_v_vb                           v_u8_u8_sel_grt_v_v_v_v_vb
#define v_u8_u8_sel_grt_v_v_v_s_vb                           v_u8_u8_sel_grt_v_v_v_v_vb

// SEL_GEQ

// f32 - f32
#define v_f32_f32_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_geq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_f32_sel_geq_v_s_v_v_b                          v_f32_f32_sel_geq_v_v_v_v_b
#define v_f32_f32_sel_geq_v_v_v_s_b                          v_f32_f32_sel_geq_v_v_v_v_b
#define v_f32_f32_sel_geq_v_v_v_v(a, b, c, d)                v_f32_f32_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_f32_sel_geq_v_s_v_v                            v_f32_f32_sel_geq_v_v_v_v
#define v_f32_f32_sel_geq_v_v_v_s                            v_f32_f32_sel_geq_v_v_v_v

#define v_f32_f32_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_geq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_f32_sel_geq_v_s_v_v_vb                         v_f32_f32_sel_geq_v_v_v_v_vb
#define v_f32_f32_sel_geq_v_v_v_s_vb                         v_f32_f32_sel_geq_v_v_v_v_vb

// f32 - i32
#define v_i32_f32_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_geq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_f32_sel_geq_v_s_v_v_b                          v_i32_f32_sel_geq_v_v_v_v_b
#define v_i32_f32_sel_geq_v_v_v_s_b                          v_i32_f32_sel_geq_v_v_v_v_b
#define v_i32_f32_sel_geq_v_v_v_v(a, b, c, d)                v_i32_f32_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_f32_sel_geq_v_s_v_v                            v_i32_f32_sel_geq_v_v_v_v
#define v_i32_f32_sel_geq_v_v_v_s                            v_i32_f32_sel_geq_v_v_v_v

#define v_i32_f32_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_geq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_f32_sel_geq_v_s_v_v_vb                         v_i32_f32_sel_geq_v_v_v_v_vb
#define v_i32_f32_sel_geq_v_v_v_s_vb                         v_i32_f32_sel_geq_v_v_v_v_vb

// f32 - u32
#define v_u32_f32_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)     v_f32_sel_geq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_f32_sel_geq_v_s_v_v_b                          v_u32_f32_sel_geq_v_v_v_v_b
#define v_u32_f32_sel_geq_v_v_v_s_b                          v_u32_f32_sel_geq_v_v_v_v_b
#define v_u32_f32_sel_geq_v_v_v_v(a, b, c, d)                v_u32_f32_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_f32_sel_geq_v_s_v_v                            v_u32_f32_sel_geq_v_v_v_v
#define v_u32_f32_sel_geq_v_v_v_s                            v_u32_f32_sel_geq_v_v_v_v

#define v_u32_f32_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_f32_sel_geq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_f32_sel_geq_v_s_v_v_vb                         v_u32_f32_sel_geq_v_v_v_v_vb
#define v_u32_f32_sel_geq_v_v_v_s_vb                         v_u32_f32_sel_geq_v_v_v_v_vb

// bf16 - bf16
#define v_bf16_bf16_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)   v_bf16_sel_geq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_bf16_sel_geq_v_s_v_v_b                        v_bf16_bf16_sel_geq_v_v_v_v_b
#define v_bf16_bf16_sel_geq_v_v_v_s_b                        v_bf16_bf16_sel_geq_v_v_v_v_b
#define v_bf16_bf16_sel_geq_v_v_v_v(a, b, c, d)              v_bf16_bf16_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_bf16_sel_geq_v_s_v_v                          v_bf16_bf16_sel_geq_v_v_v_v
#define v_bf16_bf16_sel_geq_v_v_v_s                          v_bf16_bf16_sel_geq_v_v_v_v

#define v_bf16_bf16_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)  v_bf16_sel_geq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_bf16_sel_geq_v_s_v_v_vb                       v_bf16_bf16_sel_geq_v_v_v_v_vb
#define v_bf16_bf16_sel_geq_v_v_v_s_vb                       v_bf16_bf16_sel_geq_v_v_v_v_vb

// bf16 - i16
#define v_i16_bf16_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel_geq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_bf16_sel_geq_v_s_v_v_b                         v_i16_bf16_sel_geq_v_v_v_v_b
#define v_i16_bf16_sel_geq_v_v_v_s_b                         v_i16_bf16_sel_geq_v_v_v_v_b
#define v_i16_bf16_sel_geq_v_v_v_v(a, b, c, d)               v_i16_bf16_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_bf16_sel_geq_v_s_v_v                           v_i16_bf16_sel_geq_v_v_v_v
#define v_i16_bf16_sel_geq_v_v_v_s                           v_i16_bf16_sel_geq_v_v_v_v

#define v_i16_bf16_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel_geq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_bf16_sel_geq_v_s_v_v_vb                        v_i16_bf16_sel_geq_v_v_v_v_vb
#define v_i16_bf16_sel_geq_v_v_v_s_vb                        v_i16_bf16_sel_geq_v_v_v_v_vb

// bf16 - u16
#define v_u16_bf16_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel_geq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_bf16_sel_geq_v_s_v_v_b                         v_u16_bf16_sel_geq_v_v_v_v_b
#define v_u16_bf16_sel_geq_v_v_v_s_b                         v_u16_bf16_sel_geq_v_v_v_v_b
#define v_u16_bf16_sel_geq_v_v_v_v(a, b, c, d)               v_u16_bf16_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_bf16_sel_geq_v_s_v_v                           v_u16_bf16_sel_geq_v_v_v_v
#define v_u16_bf16_sel_geq_v_v_v_s                           v_u16_bf16_sel_geq_v_v_v_v

#define v_u16_bf16_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel_geq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_bf16_sel_geq_v_s_v_v_vb                        v_u16_bf16_sel_geq_v_v_v_v_vb
#define v_u16_bf16_sel_geq_v_v_v_s_vb                        v_u16_bf16_sel_geq_v_v_v_v_vb

// i32 - f32
#define v_f32_i32_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_geq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_i32_sel_geq_v_s_v_v_b                          v_f32_i32_sel_geq_v_v_v_v_b
#define v_f32_i32_sel_geq_v_v_v_s_b                          v_f32_i32_sel_geq_v_v_v_v_b
#define v_f32_i32_sel_geq_v_v_v_v(a, b, c, d)                v_f32_i32_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_i32_sel_geq_v_s_v_v                            v_f32_i32_sel_geq_v_v_v_v
#define v_f32_i32_sel_geq_v_v_v_s                            v_f32_i32_sel_geq_v_v_v_v

#define v_f32_i32_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_geq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_i32_sel_geq_v_s_v_v_vb                         v_f32_i32_sel_geq_v_v_v_v_vb
#define v_f32_i32_sel_geq_v_v_v_s_vb                         v_f32_i32_sel_geq_v_v_v_v_vb

// i32 - i32
#define v_i32_i32_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_geq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_i32_sel_geq_v_s_v_v_b                          v_i32_i32_sel_geq_v_v_v_v_b
#define v_i32_i32_sel_geq_v_v_v_s_b                          v_i32_i32_sel_geq_v_v_v_v_b
#define v_i32_i32_sel_geq_v_v_v_v(a, b, c, d)                v_i32_i32_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_i32_sel_geq_v_s_v_v                            v_i32_i32_sel_geq_v_v_v_v
#define v_i32_i32_sel_geq_v_v_v_s                            v_i32_i32_sel_geq_v_v_v_v

#define v_i32_i32_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_geq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_i32_sel_geq_v_s_v_v_vb                         v_i32_i32_sel_geq_v_v_v_v_vb
#define v_i32_i32_sel_geq_v_v_v_s_vb                         v_i32_i32_sel_geq_v_v_v_v_vb

// i32 - u32
#define v_u32_i32_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i32_sel_geq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_i32_sel_geq_v_s_v_v_b                          v_u32_i32_sel_geq_v_v_v_v_b
#define v_u32_i32_sel_geq_v_v_v_s_b                          v_u32_i32_sel_geq_v_v_v_v_b
#define v_u32_i32_sel_geq_v_v_v_v(a, b, c, d)                v_u32_i32_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_i32_sel_geq_v_s_v_v                            v_u32_i32_sel_geq_v_v_v_v
#define v_u32_i32_sel_geq_v_v_v_s                            v_u32_i32_sel_geq_v_v_v_v

#define v_u32_i32_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i32_sel_geq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_i32_sel_geq_v_s_v_v_vb                         v_u32_i32_sel_geq_v_v_v_v_vb
#define v_u32_i32_sel_geq_v_v_v_s_vb                         v_u32_i32_sel_geq_v_v_v_v_vb

// u32 - f32
#define v_f32_u32_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_geq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_u32_sel_geq_v_s_v_v_b                          v_f32_u32_sel_geq_v_v_v_v_b
#define v_f32_u32_sel_geq_v_v_v_s_b                          v_f32_u32_sel_geq_v_v_v_v_b
#define v_f32_u32_sel_geq_v_v_v_v(a, b, c, d)                v_f32_u32_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_f32_u32_sel_geq_v_s_v_v                            v_f32_u32_sel_geq_v_v_v_v
#define v_f32_u32_sel_geq_v_v_v_s                            v_f32_u32_sel_geq_v_v_v_v

#define v_f32_u32_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_geq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_u32_sel_geq_v_s_v_v_vb                         v_f32_u32_sel_geq_v_v_v_v_vb
#define v_f32_u32_sel_geq_v_v_v_s_vb                         v_f32_u32_sel_geq_v_v_v_v_vb

// u32 - i32
#define v_i32_u32_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_geq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_u32_sel_geq_v_s_v_v_b                          v_i32_u32_sel_geq_v_v_v_v_b
#define v_i32_u32_sel_geq_v_v_v_s_b                          v_i32_u32_sel_geq_v_v_v_v_b
#define v_i32_u32_sel_geq_v_v_v_v(a, b, c, d)                v_i32_u32_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i32_u32_sel_geq_v_s_v_v                            v_i32_u32_sel_geq_v_v_v_v
#define v_i32_u32_sel_geq_v_v_v_s                            v_i32_u32_sel_geq_v_v_v_v

#define v_i32_u32_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_geq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_u32_sel_geq_v_s_v_v_vb                         v_i32_u32_sel_geq_v_v_v_v_vb
#define v_i32_u32_sel_geq_v_v_v_s_vb                         v_i32_u32_sel_geq_v_v_v_v_vb

// u32 - u32
#define v_u32_u32_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u32_sel_geq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_u32_sel_geq_v_s_v_v_b                          v_u32_u32_sel_geq_v_v_v_v_b
#define v_u32_u32_sel_geq_v_v_v_s_b                          v_u32_u32_sel_geq_v_v_v_v_b
#define v_u32_u32_sel_geq_v_v_v_v(a, b, c, d)                v_u32_u32_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u32_u32_sel_geq_v_s_v_v                            v_u32_u32_sel_geq_v_v_v_v
#define v_u32_u32_sel_geq_v_v_v_s                            v_u32_u32_sel_geq_v_v_v_v

#define v_u32_u32_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u32_sel_geq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_u32_sel_geq_v_s_v_v_vb                         v_u32_u32_sel_geq_v_v_v_v_vb
#define v_u32_u32_sel_geq_v_v_v_s_vb                         v_u32_u32_sel_geq_v_v_v_v_vb

// i16 - bf16
#define v_bf16_i16_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel_geq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_i16_sel_geq_v_s_v_v_b                         v_bf16_i16_sel_geq_v_v_v_v_b
#define v_bf16_i16_sel_geq_v_v_v_s_b                         v_bf16_i16_sel_geq_v_v_v_v_b
#define v_bf16_i16_sel_geq_v_v_v_v(a, b, c, d)               v_bf16_i16_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_i16_sel_geq_v_s_v_v                           v_bf16_i16_sel_geq_v_v_v_v
#define v_bf16_i16_sel_geq_v_v_v_s                           v_bf16_i16_sel_geq_v_v_v_v

#define v_bf16_i16_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel_geq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_i16_sel_geq_v_s_v_v_vb                        v_bf16_i16_sel_geq_v_v_v_v_vb
#define v_bf16_i16_sel_geq_v_v_v_s_vb                        v_bf16_i16_sel_geq_v_v_v_v_vb

// i16 - i16
#define v_i16_i16_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i16_sel_geq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_i16_sel_geq_v_s_v_v_b                          v_i16_i16_sel_geq_v_v_v_v_b
#define v_i16_i16_sel_geq_v_v_v_s_b                          v_i16_i16_sel_geq_v_v_v_v_b
#define v_i16_i16_sel_geq_v_v_v_v(a, b, c, d)                v_i16_i16_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_i16_sel_geq_v_s_v_v                            v_i16_i16_sel_geq_v_v_v_v
#define v_i16_i16_sel_geq_v_v_v_s                            v_i16_i16_sel_geq_v_v_v_v

#define v_i16_i16_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i16_sel_geq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_i16_sel_geq_v_s_v_v_vb                         v_i16_i16_sel_geq_v_v_v_v_vb
#define v_i16_i16_sel_geq_v_v_v_s_vb                         v_i16_i16_sel_geq_v_v_v_v_vb

// i16 - u16
#define v_u16_i16_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)     v_i16_sel_geq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_i16_sel_geq_v_s_v_v_b                          v_u16_i16_sel_geq_v_v_v_v_b
#define v_u16_i16_sel_geq_v_v_v_s_b                          v_u16_i16_sel_geq_v_v_v_v_b
#define v_u16_i16_sel_geq_v_v_v_v(a, b, c, d)                v_u16_i16_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_i16_sel_geq_v_s_v_v                            v_u16_i16_sel_geq_v_v_v_v
#define v_u16_i16_sel_geq_v_v_v_s                            v_u16_i16_sel_geq_v_v_v_v

#define v_u16_i16_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_i16_sel_geq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_i16_sel_geq_v_s_v_v_vb                         v_u16_i16_sel_geq_v_v_v_v_vb
#define v_u16_i16_sel_geq_v_v_v_s_vb                         v_u16_i16_sel_geq_v_v_v_v_vb

// u16 - bf16
#define v_bf16_u16_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel_geq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_u16_sel_geq_v_s_v_v_b                         v_bf16_u16_sel_geq_v_v_v_v_b
#define v_bf16_u16_sel_geq_v_v_v_s_b                         v_bf16_u16_sel_geq_v_v_v_v_b
#define v_bf16_u16_sel_geq_v_v_v_v(a, b, c, d)               v_bf16_u16_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_bf16_u16_sel_geq_v_s_v_v                           v_bf16_u16_sel_geq_v_v_v_v
#define v_bf16_u16_sel_geq_v_v_v_s                           v_bf16_u16_sel_geq_v_v_v_v

#define v_bf16_u16_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel_geq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_u16_sel_geq_v_s_v_v_vb                        v_bf16_u16_sel_geq_v_v_v_v_vb
#define v_bf16_u16_sel_geq_v_v_v_s_vb                        v_bf16_u16_sel_geq_v_v_v_v_vb

// u16 - i16
#define v_i16_u16_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u16_sel_geq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_u16_sel_geq_v_s_v_v_b                          v_i16_u16_sel_geq_v_v_v_v_b
#define v_i16_u16_sel_geq_v_v_v_s_b                          v_i16_u16_sel_geq_v_v_v_v_b
#define v_i16_u16_sel_geq_v_v_v_v(a, b, c, d)                v_i16_u16_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i16_u16_sel_geq_v_s_v_v                            v_i16_u16_sel_geq_v_v_v_v
#define v_i16_u16_sel_geq_v_v_v_s                            v_i16_u16_sel_geq_v_v_v_v

#define v_i16_u16_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u16_sel_geq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_u16_sel_geq_v_s_v_v_vb                         v_i16_u16_sel_geq_v_v_v_v_vb
#define v_i16_u16_sel_geq_v_v_v_s_vb                         v_i16_u16_sel_geq_v_v_v_v_vb

// u16 - u16
#define v_u16_u16_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)     v_u16_sel_geq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_u16_sel_geq_v_s_v_v_b                          v_u16_u16_sel_geq_v_v_v_v_b
#define v_u16_u16_sel_geq_v_v_v_s_b                          v_u16_u16_sel_geq_v_v_v_v_b
#define v_u16_u16_sel_geq_v_v_v_v(a, b, c, d)                v_u16_u16_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u16_u16_sel_geq_v_s_v_v                            v_u16_u16_sel_geq_v_v_v_v
#define v_u16_u16_sel_geq_v_v_v_s                            v_u16_u16_sel_geq_v_v_v_v

#define v_u16_u16_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)    v_u16_sel_geq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_u16_sel_geq_v_s_v_v_vb                         v_u16_u16_sel_geq_v_v_v_v_vb
#define v_u16_u16_sel_geq_v_v_v_s_vb                         v_u16_u16_sel_geq_v_v_v_v_vb

// i8 - u8
#define v_i8_i8_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)       v_i8_sel_geq_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel_geq_v_s_v_v_b                            v_i8_i8_sel_geq_v_v_v_v_b
#define v_i8_i8_sel_geq_v_v_v_s_b                            v_i8_i8_sel_geq_v_v_v_v_b
#define v_i8_i8_sel_geq_v_v_v_v(a, b, c, d)                  v_i8_i8_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i8_i8_sel_geq_v_s_v_v                              v_i8_i8_sel_geq_v_v_v_v
#define v_i8_i8_sel_geq_v_v_v_s                              v_i8_i8_sel_geq_v_v_v_v

#define v_i8_i8_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_i8_sel_geq_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel_geq_v_s_v_v_vb                           v_i8_i8_sel_geq_v_v_v_v_vb
#define v_i8_i8_sel_geq_v_v_v_s_vb                           v_i8_i8_sel_geq_v_v_v_v_vb

// i8 - u8
#define v_u8_i8_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)       v_i8_sel_geq_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel_geq_v_s_v_v_b                            v_u8_i8_sel_geq_v_v_v_v_b
#define v_u8_i8_sel_geq_v_v_v_s_b                            v_u8_i8_sel_geq_v_v_v_v_b
#define v_u8_i8_sel_geq_v_v_v_v(a, b, c, d)                  v_u8_i8_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u8_i8_sel_geq_v_s_v_v                              v_u8_i8_sel_geq_v_v_v_v
#define v_u8_i8_sel_geq_v_v_v_s                              v_u8_i8_sel_geq_v_v_v_v

#define v_u8_i8_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_i8_sel_geq_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel_geq_v_s_v_v_vb                           v_u8_i8_sel_geq_v_v_v_v_vb
#define v_u8_i8_sel_geq_v_v_v_s_vb                           v_u8_i8_sel_geq_v_v_v_v_vb

// u8 - i8
#define v_i8_u8_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)       v_u8_sel_geq_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel_geq_v_s_v_v_b                            v_i8_u8_sel_geq_v_v_v_v_b
#define v_i8_u8_sel_geq_v_v_v_s_b                            v_i8_u8_sel_geq_v_v_v_v_b
#define v_i8_u8_sel_geq_v_v_v_v(a, b, c, d)                  v_i8_u8_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_i8_u8_sel_geq_v_s_v_v                              v_i8_u8_sel_geq_v_v_v_v
#define v_i8_u8_sel_geq_v_v_v_s                              v_i8_u8_sel_geq_v_v_v_v

#define v_i8_u8_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_u8_sel_geq_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel_geq_v_s_v_v_vb                           v_i8_u8_sel_geq_v_v_v_v_vb
#define v_i8_u8_sel_geq_v_v_v_s_vb                           v_i8_u8_sel_geq_v_v_v_v_vb

// u8 - u8
#define v_u8_u8_sel_geq_v_v_v_v_b(a, b, c, d, i, p, o)       v_u8_sel_geq_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel_geq_v_s_v_v_b                            v_u8_u8_sel_geq_v_v_v_v_b
#define v_u8_u8_sel_geq_v_v_v_s_b                            v_u8_u8_sel_geq_v_v_v_v_b
#define v_u8_u8_sel_geq_v_v_v_v(a, b, c, d)                  v_u8_u8_sel_geq_v_v_v_v_b(a, b, c, d, 0, 1, 0)
#define v_u8_u8_sel_geq_v_s_v_v                              v_u8_u8_sel_geq_v_v_v_v
#define v_u8_u8_sel_geq_v_v_v_s                              v_u8_u8_sel_geq_v_v_v_v

#define v_u8_u8_sel_geq_v_v_v_v_vb(a, b, c, d, i, p, o)      v_u8_sel_geq_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel_geq_v_s_v_v_vb                           v_u8_u8_sel_geq_v_v_v_v_vb
#define v_u8_u8_sel_geq_v_v_v_s_vb                           v_u8_u8_sel_geq_v_v_v_v_vb

// SEL2_GEQ

// f32 - f32
#define v_f32_f32_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_f32_sel2_geq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_f32_sel2_geq_v_s_v_v_b                         v_f32_f32_sel2_geq_v_v_v_v_b
#define v_f32_f32_sel2_geq_v_v_v_s_b                         v_f32_f32_sel2_geq_v_v_v_v_b
#define v_f32_f32_sel2_geq_v_v_v_v(a, b, c, d)               v_f32_f32_sel2_geq_v_v_v_v_b(a, b, c, d, (float64_pair_t){0}, 1, 0)
#define v_f32_f32_sel2_geq_v_s_v_v                           v_f32_f32_sel2_geq_v_v_v_v
#define v_f32_f32_sel2_geq_v_v_v_s                           v_f32_f32_sel2_geq_v_v_v_v

#define v_f32_f32_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_f32_sel2_geq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_f32_f32_sel2_geq_v_v_v_s_vb                        v_f32_f32_sel2_geq_v_v_v_v_vb
#define v_f32_f32_sel2_geq_v_s_v_v_vb                        v_f32_f32_sel2_geq_v_v_v_v_vb

// f32 - i32
#define v_f32_i32_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i32_sel2_geq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_i32_sel2_geq_v_s_v_v_b                         v_f32_i32_sel2_geq_v_v_v_v_b
#define v_f32_i32_sel2_geq_v_v_v_s_b                         v_f32_i32_sel2_geq_v_v_v_v_b
#define v_f32_i32_sel2_geq_v_v_v_v(a, b, c, d)               v_f32_i32_sel2_geq_v_v_v_v_b(a, b, c, d, (int64_float64_pair_t){0}, 1, 0)
#define v_f32_i32_sel2_geq_v_s_v_v                           v_f32_i32_sel2_geq_v_v_v_v
#define v_f32_i32_sel2_geq_v_v_v_s                           v_f32_i32_sel2_geq_v_v_v_v

#define v_f32_i32_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i32_sel2_geq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_i32_sel2_geq_v_v_v_s_vb                        v_f32_i32_sel2_geq_v_v_v_v_vb
#define v_f32_i32_sel2_geq_v_s_v_v_vb                        v_f32_i32_sel2_geq_v_v_v_v_vb

// f32 - u32
#define v_f32_u32_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u32_sel2_geq_f32_b(a, b, c, d, 0, i, p, o);
#define v_f32_u32_sel2_geq_v_s_v_v_b                         v_f32_u32_sel2_geq_v_v_v_v_b
#define v_f32_u32_sel2_geq_v_v_v_s_b                         v_f32_u32_sel2_geq_v_v_v_v_b
#define v_f32_u32_sel2_geq_v_v_v_v(a, b, c, d)               v_f32_u32_sel2_geq_v_v_v_v_b(a, b, c, d, (uint64_float64_pair_t){0}, 1, 0)
#define v_f32_u32_sel2_geq_v_s_v_v                           v_f32_u32_sel2_geq_v_v_v_v
#define v_f32_u32_sel2_geq_v_v_v_s                           v_f32_u32_sel2_geq_v_v_v_v

#define v_f32_u32_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u32_sel2_geq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_f32_u32_sel2_geq_v_s_v_v_vb                        v_f32_u32_sel2_geq_v_v_v_v_vb
#define v_f32_u32_sel2_geq_v_v_v_s_vb                        v_f32_u32_sel2_geq_v_v_v_v_vb

// bf16 - bf16
#define v_bf16_bf16_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel2_geq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_bf16_sel2_geq_v_s_v_v_b                         v_bf16_bf16_sel2_geq_v_v_v_v_b
#define v_bf16_bf16_sel2_geq_v_v_v_s_b                         v_bf16_bf16_sel2_geq_v_v_v_v_b
#define v_bf16_bf16_sel2_geq_v_v_v_v(a, b, c, d)               v_bf16_bf16_sel2_geq_v_v_v_v_b(a, b, c, d, (bfloat128_bfloat128_pair_t){0}, 1, 0)
#define v_bf16_bf16_sel2_geq_v_s_v_v                           v_bf16_bf16_sel2_geq_v_v_v_v
#define v_bf16_bf16_sel2_geq_v_v_v_s                           v_bf16_bf16_sel2_geq_v_v_v_v

#define v_bf16_bf16_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel2_geq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o);
#define v_bf16_bf16_sel2_geq_v_s_v_v_vb                        v_bf16_bf16_sel2_geq_v_v_v_v_vb
#define v_bf16_bf16_sel2_geq_v_v_v_s_vb                        v_bf16_bf16_sel2_geq_v_v_v_v_vb

// bf16 - i16
#define v_bf16_i16_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel2_geq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_i16_sel2_geq_v_s_v_v_b                         v_bf16_i16_sel2_geq_v_v_v_v_b
#define v_bf16_i16_sel2_geq_v_v_v_s_b                         v_bf16_i16_sel2_geq_v_v_v_v_b
#define v_bf16_i16_sel2_geq_v_v_v_v(a, b, c, d)               v_bf16_i16_sel2_geq_v_v_v_v_b(a, b, c, d, (short128_bfloat128_pair_t){0}, 1, 0)
#define v_bf16_i16_sel2_geq_v_s_v_v                           v_bf16_i16_sel2_geq_v_v_v_v
#define v_bf16_i16_sel2_geq_v_v_v_s                           v_bf16_i16_sel2_geq_v_v_v_v

#define v_bf16_i16_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel2_geq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o);
#define v_bf16_i16_sel2_geq_v_s_v_v_vb                        v_bf16_i16_sel2_geq_v_v_v_v_vb
#define v_bf16_i16_sel2_geq_v_v_v_s_vb                        v_bf16_i16_sel2_geq_v_v_v_v_vb

// bf16 - u16
#define v_bf16_u16_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel2_geq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_u16_sel2_geq_v_s_v_v_b                         v_bf16_u16_sel2_geq_v_v_v_v_b
#define v_bf16_u16_sel2_geq_v_v_v_s_b                         v_bf16_u16_sel2_geq_v_v_v_v_b
#define v_bf16_u16_sel2_geq_v_v_v_v(a, b, c, d)               v_bf16_u16_sel2_geq_v_v_v_v_b(a, b, c, d, (ushort128_bfloat128_pair_t){0}, 1, 0)
#define v_bf16_u16_sel2_geq_v_s_v_v                           v_bf16_u16_sel2_geq_v_v_v_v
#define v_bf16_u16_sel2_geq_v_v_v_s                           v_bf16_u16_sel2_geq_v_v_v_v

#define v_bf16_u16_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel2_geq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_u16_sel2_geq_v_s_v_v_vb                        v_bf16_u16_sel2_geq_v_v_v_v_vb
#define v_bf16_u16_sel2_geq_v_v_v_s_vb                        v_bf16_u16_sel2_geq_v_v_v_v_vb

// i32 - f32
#define v_i32_f32_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_f32_sel2_geq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_f32_sel2_geq_v_s_v_v_b                         v_i32_f32_sel2_geq_v_v_v_v_b
#define v_i32_f32_sel2_geq_v_v_v_s_b                         v_i32_f32_sel2_geq_v_v_v_v_b
#define v_i32_f32_sel2_geq_v_v_v_v(a, b, c, d)               v_i32_f32_sel2_geq_v_v_v_v_b(a, b, c, d, (float64_int64_pair_t){0}, 1, 0)
#define v_i32_f32_sel2_geq_v_s_v_v                           v_i32_f32_sel2_geq_v_v_v_v
#define v_i32_f32_sel2_geq_v_v_v_s                           v_i32_f32_sel2_geq_v_v_v_v

#define v_i32_f32_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_f32_sel2_geq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_f32_sel2_geq_v_s_v_v_vb                        v_i32_f32_sel2_geq_v_v_v_v_vb
#define v_i32_f32_sel2_geq_v_v_v_s_vb                        v_i32_f32_sel2_geq_v_v_v_v_vb

// i32 - i32
#define v_i32_i32_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i32_sel2_geq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_i32_sel2_geq_v_s_v_v_b                         v_i32_i32_sel2_geq_v_v_v_v_b
#define v_i32_i32_sel2_geq_v_v_v_s_b                         v_i32_i32_sel2_geq_v_v_v_v_b
#define v_i32_i32_sel2_geq_v_v_v_v(a, b, c, d)               v_i32_i32_sel2_geq_v_v_v_v_b(a, b, c, d, (int64_pair_t){0}, 1, 0)
#define v_i32_i32_sel2_geq_v_s_v_v                           v_i32_i32_sel2_geq_v_v_v_v
#define v_i32_i32_sel2_geq_v_v_v_s                           v_i32_i32_sel2_geq_v_v_v_v

#define v_i32_i32_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i32_sel2_geq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_i32_sel2_geq_v_s_v_v_vb                        v_i32_i32_sel2_geq_v_v_v_v_vb
#define v_i32_i32_sel2_geq_v_v_v_s_vb                        v_i32_i32_sel2_geq_v_v_v_v_vb

// i32 - u32
#define v_i32_u32_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u32_sel2_geq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_u32_sel2_geq_v_s_v_v_b                         v_i32_u32_sel2_geq_v_v_v_v_b
#define v_i32_u32_sel2_geq_v_v_v_s_b                         v_i32_u32_sel2_geq_v_v_v_v_b
#define v_i32_u32_sel2_geq_v_v_v_v(a, b, c, d)               v_i32_u32_sel2_geq_v_v_v_v_b(a, b, c, d, (uint64_int64_pair_t){0}, 1, 0)
#define v_i32_u32_sel2_geq_v_s_v_v                           v_i32_u32_sel2_geq_v_v_v_v
#define v_i32_u32_sel2_geq_v_v_v_s                           v_i32_u32_sel2_geq_v_v_v_v

#define v_i32_u32_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u32_sel2_geq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_u32_sel2_geq_v_s_v_v_vb                        v_i32_u32_sel2_geq_v_v_v_v_vb
#define v_i32_u32_sel2_geq_v_v_v_s_vb                        v_i32_u32_sel2_geq_v_v_v_v_vb

// u32 - f32
#define v_u32_f32_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_f32_sel2_geq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_f32_sel2_geq_v_s_v_v_b                         v_u32_f32_sel2_geq_v_v_v_v_b
#define v_u32_f32_sel2_geq_v_v_v_s_b                         v_u32_f32_sel2_geq_v_v_v_v_b
#define v_u32_f32_sel2_geq_v_v_v_v(a, b, c, d)               v_u32_f32_sel2_geq_v_v_v_v_b(a, b, c, d, (float64_uint64_pair_t){0}, 1, 0)
#define v_u32_f32_sel2_geq_v_s_v_v                           v_u32_f32_sel2_geq_v_v_v_v
#define v_u32_f32_sel2_geq_v_v_v_s                           v_u32_f32_sel2_geq_v_v_v_v

#define v_u32_f32_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_f32_sel2_geq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_u32_f32_sel2_geq_v_s_v_v_vb                        v_u32_f32_sel2_geq_v_v_v_v_vb
#define v_u32_f32_sel2_geq_v_v_v_s_vb                        v_u32_f32_sel2_geq_v_v_v_v_vb

// u32 - i32
#define v_u32_i32_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i32_sel2_geq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_i32_sel2_geq_v_s_v_v_b                         v_u32_i32_sel2_geq_v_v_v_v_b
#define v_u32_i32_sel2_geq_v_v_v_s_b                         v_u32_i32_sel2_geq_v_v_v_v_b
#define v_u32_i32_sel2_geq_v_v_v_v(a, b, c, d)               v_u32_i32_sel2_geq_v_v_v_v_b(a, b, c, d, (int64_uint64_pair_t){0}, 1, 0)
#define v_u32_i32_sel2_geq_v_s_v_v                           v_u32_i32_sel2_geq_v_v_v_v
#define v_u32_i32_sel2_geq_v_v_v_s                           v_u32_i32_sel2_geq_v_v_v_v

#define v_u32_i32_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i32_sel2_geq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_i32_sel2_geq_v_s_v_v_vb                        v_u32_i32_sel2_geq_v_v_v_v_vb
#define v_u32_i32_sel2_geq_v_v_v_s_vb                        v_u32_i32_sel2_geq_v_v_v_v_vb

// u32 - u32
#define v_u32_u32_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u32_sel2_geq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_u32_sel2_geq_v_s_v_v_b                         v_u32_u32_sel2_geq_v_v_v_v_b
#define v_u32_u32_sel2_geq_v_v_v_s_b                         v_u32_u32_sel2_geq_v_v_v_v_b
#define v_u32_u32_sel2_geq_v_v_v_v(a, b, c, d)               v_u32_u32_sel2_geq_v_v_v_v_b(a, b, c, d, (uint64_pair_t){0}, 1, 0)
#define v_u32_u32_sel2_geq_v_s_v_v                           v_u32_u32_sel2_geq_v_v_v_v
#define v_u32_u32_sel2_geq_v_v_v_s                           v_u32_u32_sel2_geq_v_v_v_v

#define v_u32_u32_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u32_sel2_geq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_u32_u32_sel2_geq_v_s_v_v_vb                        v_u32_u32_sel2_geq_v_v_v_v_vb
#define v_u32_u32_sel2_geq_v_v_v_s_vb                        v_u32_u32_sel2_geq_v_v_v_v_vb

// i16 - bf16
#define v_i16_bf16_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel2_geq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_bf16_sel2_geq_v_s_v_v_b                         v_i16_bf16_sel2_geq_v_v_v_v_b
#define v_i16_bf16_sel2_geq_v_v_v_s_b                         v_i16_bf16_sel2_geq_v_v_v_v_b
#define v_i16_bf16_sel2_geq_v_v_v_v(a, b, c, d)               v_i16_bf16_sel2_geq_v_v_v_v_b(a, b, c, d, (bfloat128_short128_pair_t){0}, 1, 0)
#define v_i16_bf16_sel2_geq_v_s_v_v                           v_i16_bf16_sel2_geq_v_v_v_v
#define v_i16_bf16_sel2_geq_v_v_v_s                           v_i16_bf16_sel2_geq_v_v_v_v

#define v_i16_bf16_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel2_geq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_bf16_sel2_geq_v_s_v_v_vb                        v_i16_bf16_sel2_geq_v_v_v_v_vb
#define v_i16_bf16_sel2_geq_v_v_v_s_vb                        v_i16_bf16_sel2_geq_v_v_v_v_vb

// i16 - i16
#define v_i16_i16_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel2_geq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_i16_sel2_geq_v_s_v_v_b                         v_i16_i16_sel2_geq_v_v_v_v_b
#define v_i16_i16_sel2_geq_v_v_v_s_b                         v_i16_i16_sel2_geq_v_v_v_v_b
#define v_i16_i16_sel2_geq_v_v_v_v(a, b, c, d)               v_i16_i16_sel2_geq_v_v_v_v_b(a, b, c, d, (short128_pair_t){0}, 1, 0)
#define v_i16_i16_sel2_geq_v_s_v_v                           v_i16_i16_sel2_geq_v_v_v_v
#define v_i16_i16_sel2_geq_v_v_v_s                           v_i16_i16_sel2_geq_v_v_v_v

#define v_i16_i16_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel2_geq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_i16_sel2_geq_v_s_v_v_vb                        v_i16_i16_sel2_geq_v_v_v_v_vb
#define v_i16_i16_sel2_geq_v_v_v_s_vb                        v_i16_i16_sel2_geq_v_v_v_v_vb

// i16 - u16
#define v_i16_u16_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel2_geq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_u16_sel2_geq_v_s_v_v_b                         v_i16_u16_sel2_geq_v_v_v_v_b
#define v_i16_u16_sel2_geq_v_v_v_s_b                         v_i16_u16_sel2_geq_v_v_v_v_b
#define v_i16_u16_sel2_geq_v_v_v_v(a, b, c, d)               v_i16_u16_sel2_geq_v_v_v_v_b(a, b, c, d, (ushort128_short128_pair_t){0}, 1, 0)
#define v_i16_u16_sel2_geq_v_s_v_v                           v_i16_u16_sel2_geq_v_v_v_v
#define v_i16_u16_sel2_geq_v_v_v_s                           v_i16_u16_sel2_geq_v_v_v_v

#define v_i16_u16_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel2_geq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_u16_sel2_geq_v_s_v_v_vb                        v_i16_u16_sel2_geq_v_v_v_v_vb
#define v_i16_u16_sel2_geq_v_v_v_s_vb                        v_i16_u16_sel2_geq_v_v_v_v_vb

// u16 - bf16
#define v_u16_bf16_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel2_geq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_bf16_sel2_geq_v_s_v_v_b                         v_u16_bf16_sel2_geq_v_v_v_v_b
#define v_u16_bf16_sel2_geq_v_v_v_s_b                         v_u16_bf16_sel2_geq_v_v_v_v_b
#define v_u16_bf16_sel2_geq_v_v_v_v(a, b, c, d)               v_u16_bf16_sel2_geq_v_v_v_v_b(a, b, c, d, (bfloat128_ushort128_pair_t){0}, 1, 0)
#define v_u16_bf16_sel2_geq_v_s_v_v                           v_u16_bf16_sel2_geq_v_v_v_v
#define v_u16_bf16_sel2_geq_v_v_v_s                           v_u16_bf16_sel2_geq_v_v_v_v

#define v_u16_bf16_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel2_geq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_bf16_sel2_geq_v_s_v_v_vb                        v_u16_bf16_sel2_geq_v_v_v_v_vb
#define v_u16_bf16_sel2_geq_v_v_v_s_vb                        v_u16_bf16_sel2_geq_v_v_v_v_vb

// u16 - i16
#define v_u16_i16_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel2_geq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_i16_sel2_geq_v_s_v_v_b                         v_u16_i16_sel2_geq_v_v_v_v_b
#define v_u16_i16_sel2_geq_v_v_v_s_b                         v_u16_i16_sel2_geq_v_v_v_v_b
#define v_u16_i16_sel2_geq_v_v_v_v(a, b, c, d)               v_u16_i16_sel2_geq_v_v_v_v_b(a, b, c, d, (short128_ushort128_pair_t){0}, 1, 0)
#define v_u16_i16_sel2_geq_v_s_v_v                           v_u16_i16_sel2_geq_v_v_v_v
#define v_u16_i16_sel2_geq_v_v_v_s                           v_u16_i16_sel2_geq_v_v_v_v

#define v_u16_i16_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel2_geq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_i16_sel2_geq_v_s_v_v_vb                        v_u16_i16_sel2_geq_v_v_v_v_vb
#define v_u16_i16_sel2_geq_v_v_v_s_vb                        v_u16_i16_sel2_geq_v_v_v_v_vb

// u16 - u16
#define v_u16_u16_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel2_geq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_u16_sel2_geq_v_s_v_v_b                         v_u16_u16_sel2_geq_v_v_v_v_b
#define v_u16_u16_sel2_geq_v_v_v_s_b                         v_u16_u16_sel2_geq_v_v_v_v_b
#define v_u16_u16_sel2_geq_v_v_v_v(a, b, c, d)               v_u16_u16_sel2_geq_v_v_v_v_b(a, b, c, d, (ushort128_pair_t){0}, 1, 0)
#define v_u16_u16_sel2_geq_v_s_v_v                           v_u16_u16_sel2_geq_v_v_v_v
#define v_u16_u16_sel2_geq_v_v_v_s                           v_u16_u16_sel2_geq_v_v_v_v

#define v_u16_u16_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel2_geq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_u16_sel2_geq_v_s_v_v_vb                        v_u16_u16_sel2_geq_v_v_v_v_vb
#define v_u16_u16_sel2_geq_v_v_v_s_vb                        v_u16_u16_sel2_geq_v_v_v_v_vb

// i8 - i8
#define v_i8_i8_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i8_sel2_geq_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel2_geq_v_s_v_v_b                         v_i8_i8_sel2_geq_v_v_v_v_b
#define v_i8_i8_sel2_geq_v_v_v_s_b                         v_i8_i8_sel2_geq_v_v_v_v_b
#define v_i8_i8_sel2_geq_v_v_v_v(a, b, c, d)               v_i8_i8_sel2_geq_v_v_v_v_b(a, b, c, d, (char256_pair_t){0}, 1, 0)
#define v_i8_i8_sel2_geq_v_s_v_v                           v_i8_i8_sel2_geq_v_v_v_v
#define v_i8_i8_sel2_geq_v_v_v_s                           v_i8_i8_sel2_geq_v_v_v_v

#define v_i8_i8_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i8_sel2_geq_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel2_geq_v_s_v_v_vb                        v_i8_i8_sel2_geq_v_v_v_v_vb
#define v_i8_i8_sel2_geq_v_v_v_s_vb                        v_i8_i8_sel2_geq_v_v_v_v_vb

// i8 - u8
#define v_i8_u8_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u8_sel2_geq_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel2_geq_v_s_v_v_b                         v_i8_u8_sel2_geq_v_v_v_v_b
#define v_i8_u8_sel2_geq_v_v_v_s_b                         v_i8_u8_sel2_geq_v_v_v_v_b
#define v_i8_u8_sel2_geq_v_v_v_v(a, b, c, d)               v_i8_u8_sel2_geq_v_v_v_v_b(a, b, c, d, (uchar256_char256_pair_t){0}, 1, 0)
#define v_i8_u8_sel2_geq_v_s_v_v                           v_i8_u8_sel2_geq_v_v_v_v
#define v_i8_u8_sel2_geq_v_v_v_s                           v_i8_u8_sel2_geq_v_v_v_v

#define v_i8_u8_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u8_sel2_geq_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel2_geq_v_s_v_v_vb                        v_i8_u8_sel2_geq_v_v_v_v_vb
#define v_i8_u8_sel2_geq_v_v_v_s_vb                        v_i8_u8_sel2_geq_v_v_v_v_vb

// u8 - i8
#define v_u8_i8_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i8_sel2_geq_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel2_geq_v_s_v_v_b                         v_u8_i8_sel2_geq_v_v_v_v_b
#define v_u8_i8_sel2_geq_v_v_v_s_b                         v_u8_i8_sel2_geq_v_v_v_v_b
#define v_u8_i8_sel2_geq_v_v_v_v(a, b, c, d)               v_u8_i8_sel2_geq_v_v_v_v_b(a, b, c, d, (char256_uchar256_pair_t){0}, 1, 0)
#define v_u8_i8_sel2_geq_v_s_v_v                           v_u8_i8_sel2_geq_v_v_v_v
#define v_u8_i8_sel2_geq_v_v_v_s                           v_u8_i8_sel2_geq_v_v_v_v

#define v_u8_i8_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i8_sel2_geq_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel2_geq_v_s_v_v_vb                        v_u8_i8_sel2_geq_v_v_v_v_vb
#define v_u8_i8_sel2_geq_v_v_v_s_vb                        v_u8_i8_sel2_geq_v_v_v_v_vb

// u8 - u8
#define v_u8_u8_sel2_geq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u8_sel2_geq_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel2_geq_v_s_v_v_b                         v_u8_u8_sel2_geq_v_v_v_v_b
#define v_u8_u8_sel2_geq_v_v_v_s_b                         v_u8_u8_sel2_geq_v_v_v_v_b
#define v_u8_u8_sel2_geq_v_v_v_v(a, b, c, d)               v_u8_u8_sel2_geq_v_v_v_v_b(a, b, c, d, (uchar256_pair_t){0}, 1, 0)
#define v_u8_u8_sel2_geq_v_s_v_v                           v_u8_u8_sel2_geq_v_v_v_v
#define v_u8_u8_sel2_geq_v_v_v_s                           v_u8_u8_sel2_geq_v_v_v_v

#define v_u8_u8_sel2_geq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u8_sel2_geq_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel2_geq_v_s_v_v_vb                        v_u8_u8_sel2_geq_v_v_v_v_vb
#define v_u8_u8_sel2_geq_v_v_v_s_vb                        v_u8_u8_sel2_geq_v_v_v_v_vb

// SEL2_GRT

// f32 - f32
#define v_f32_f32_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_f32_sel2_grt_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_f32_sel2_grt_v_s_v_v_b                         v_f32_f32_sel2_grt_v_v_v_v_b
#define v_f32_f32_sel2_grt_v_v_v_s_b                         v_f32_f32_sel2_grt_v_v_v_v_b
#define v_f32_f32_sel2_grt_v_v_v_v(a, b, c, d)               v_f32_f32_sel2_grt_v_v_v_v_b(a, b, c, d, (float64_pair_t){0}, 1, 0)
#define v_f32_f32_sel2_grt_v_s_v_v                           v_f32_f32_sel2_grt_v_v_v_v
#define v_f32_f32_sel2_grt_v_v_v_s                           v_f32_f32_sel2_grt_v_v_v_v

#define v_f32_f32_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_f32_sel2_grt_f32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_f32_f32_sel2_grt_v_v_v_s_vb                        v_f32_f32_sel2_grt_v_v_v_v_vb
#define v_f32_f32_sel2_grt_v_s_v_v_vb                        v_f32_f32_sel2_grt_v_v_v_v_vb

// f32 - i32
#define v_f32_i32_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_i32_sel2_grt_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_i32_sel2_grt_v_s_v_v_b                         v_f32_i32_sel2_grt_v_v_v_v_b
#define v_f32_i32_sel2_grt_v_v_v_s_b                         v_f32_i32_sel2_grt_v_v_v_v_b
#define v_f32_i32_sel2_grt_v_v_v_v(a, b, c, d)               v_f32_i32_sel2_grt_v_v_v_v_b(a, b, c, d, (int64_float64_pair_t){0}, 1, 0)
#define v_f32_i32_sel2_grt_v_s_v_v                           v_f32_i32_sel2_grt_v_v_v_v
#define v_f32_i32_sel2_grt_v_v_v_s                           v_f32_i32_sel2_grt_v_v_v_v

#define v_f32_i32_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i32_sel2_grt_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_i32_sel2_grt_v_v_v_s_vb                        v_f32_i32_sel2_grt_v_v_v_v_vb
#define v_f32_i32_sel2_grt_v_s_v_v_vb                        v_f32_i32_sel2_grt_v_v_v_v_vb

// f32 - u32
#define v_f32_u32_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_u32_sel2_grt_f32_b(a, b, c, d, 0, i, p, o);
#define v_f32_u32_sel2_grt_v_s_v_v_b                         v_f32_u32_sel2_grt_v_v_v_v_b
#define v_f32_u32_sel2_grt_v_v_v_s_b                         v_f32_u32_sel2_grt_v_v_v_v_b
#define v_f32_u32_sel2_grt_v_v_v_v(a, b, c, d)               v_f32_u32_sel2_grt_v_v_v_v_b(a, b, c, d, (uint64_float64_pair_t){0}, 1, 0)
#define v_f32_u32_sel2_grt_v_s_v_v                           v_f32_u32_sel2_grt_v_v_v_v
#define v_f32_u32_sel2_grt_v_v_v_s                           v_f32_u32_sel2_grt_v_v_v_v

#define v_f32_u32_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u32_sel2_grt_f32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_f32_u32_sel2_grt_v_s_v_v_vb                        v_f32_u32_sel2_grt_v_v_v_v_vb
#define v_f32_u32_sel2_grt_v_v_v_s_vb                        v_f32_u32_sel2_grt_v_v_v_v_vb

// bf16 - bf16
#define v_bf16_bf16_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel2_grt_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_bf16_sel2_grt_v_s_v_v_b                         v_bf16_bf16_sel2_grt_v_v_v_v_b
#define v_bf16_bf16_sel2_grt_v_v_v_s_b                         v_bf16_bf16_sel2_grt_v_v_v_v_b
#define v_bf16_bf16_sel2_grt_v_v_v_v(a, b, c, d)               v_bf16_bf16_sel2_grt_v_v_v_v_b(a, b, c, d, (bfloat128_bfloat128_pair_t){0}, 1, 0)
#define v_bf16_bf16_sel2_grt_v_s_v_v                           v_bf16_bf16_sel2_grt_v_v_v_v
#define v_bf16_bf16_sel2_grt_v_v_v_s                           v_bf16_bf16_sel2_grt_v_v_v_v

#define v_bf16_bf16_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel2_grt_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o);
#define v_bf16_bf16_sel2_grt_v_s_v_v_vb                        v_bf16_bf16_sel2_grt_v_v_v_v_vb
#define v_bf16_bf16_sel2_grt_v_v_v_s_vb                        v_bf16_bf16_sel2_grt_v_v_v_v_vb

// bf16 - i16
#define v_bf16_i16_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel2_grt_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_i16_sel2_grt_v_s_v_v_b                         v_bf16_i16_sel2_grt_v_v_v_v_b
#define v_bf16_i16_sel2_grt_v_v_v_s_b                         v_bf16_i16_sel2_grt_v_v_v_v_b
#define v_bf16_i16_sel2_grt_v_v_v_v(a, b, c, d)               v_bf16_i16_sel2_grt_v_v_v_v_b(a, b, c, d, (short128_bfloat128_pair_t){0}, 1, 0)
#define v_bf16_i16_sel2_grt_v_s_v_v                           v_bf16_i16_sel2_grt_v_v_v_v
#define v_bf16_i16_sel2_grt_v_v_v_s                           v_bf16_i16_sel2_grt_v_v_v_v

#define v_bf16_i16_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel2_grt_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o);
#define v_bf16_i16_sel2_grt_v_s_v_v_vb                        v_bf16_i16_sel2_grt_v_v_v_v_vb
#define v_bf16_i16_sel2_grt_v_v_v_s_vb                        v_bf16_i16_sel2_grt_v_v_v_v_vb

// bf16 - u16
#define v_bf16_u16_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel2_grt_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_u16_sel2_grt_v_s_v_v_b                         v_bf16_u16_sel2_grt_v_v_v_v_b
#define v_bf16_u16_sel2_grt_v_v_v_s_b                         v_bf16_u16_sel2_grt_v_v_v_v_b
#define v_bf16_u16_sel2_grt_v_v_v_v(a, b, c, d)               v_bf16_u16_sel2_grt_v_v_v_v_b(a, b, c, d, (ushort128_bfloat128_pair_t){0}, 1, 0)
#define v_bf16_u16_sel2_grt_v_s_v_v                           v_bf16_u16_sel2_grt_v_v_v_v
#define v_bf16_u16_sel2_grt_v_v_v_s                           v_bf16_u16_sel2_grt_v_v_v_v

#define v_bf16_u16_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel2_grt_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_u16_sel2_grt_v_s_v_v_vb                        v_bf16_u16_sel2_grt_v_v_v_v_vb
#define v_bf16_u16_sel2_grt_v_v_v_s_vb                        v_bf16_u16_sel2_grt_v_v_v_v_vb

// i32 - f32
#define v_i32_f32_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_f32_sel2_grt_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_f32_sel2_grt_v_s_v_v_b                         v_i32_f32_sel2_grt_v_v_v_v_b
#define v_i32_f32_sel2_grt_v_v_v_s_b                         v_i32_f32_sel2_grt_v_v_v_v_b
#define v_i32_f32_sel2_grt_v_v_v_v(a, b, c, d)               v_i32_f32_sel2_grt_v_v_v_v_b(a, b, c, d, (float64_int64_pair_t){0}, 1, 0)
#define v_i32_f32_sel2_grt_v_s_v_v                           v_i32_f32_sel2_grt_v_v_v_v
#define v_i32_f32_sel2_grt_v_v_v_s                           v_i32_f32_sel2_grt_v_v_v_v

#define v_i32_f32_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_f32_sel2_grt_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_f32_sel2_grt_v_s_v_v_vb                        v_i32_f32_sel2_grt_v_v_v_v_vb
#define v_i32_f32_sel2_grt_v_v_v_s_vb                        v_i32_f32_sel2_grt_v_v_v_v_vb

// i32 - i32
#define v_i32_i32_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_i32_sel2_grt_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_i32_sel2_grt_v_s_v_v_b                         v_i32_i32_sel2_grt_v_v_v_v_b
#define v_i32_i32_sel2_grt_v_v_v_s_b                         v_i32_i32_sel2_grt_v_v_v_v_b
#define v_i32_i32_sel2_grt_v_v_v_v(a, b, c, d)               v_i32_i32_sel2_grt_v_v_v_v_b(a, b, c, d, (int64_pair_t){0}, 1, 0)
#define v_i32_i32_sel2_grt_v_s_v_v                           v_i32_i32_sel2_grt_v_v_v_v
#define v_i32_i32_sel2_grt_v_v_v_s                           v_i32_i32_sel2_grt_v_v_v_v

#define v_i32_i32_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i32_sel2_grt_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_i32_sel2_grt_v_s_v_v_vb                        v_i32_i32_sel2_grt_v_v_v_v_vb
#define v_i32_i32_sel2_grt_v_v_v_s_vb                        v_i32_i32_sel2_grt_v_v_v_v_vb

// i32 - u32
#define v_i32_u32_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_u32_sel2_grt_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_u32_sel2_grt_v_s_v_v_b                         v_i32_u32_sel2_grt_v_v_v_v_b
#define v_i32_u32_sel2_grt_v_v_v_s_b                         v_i32_u32_sel2_grt_v_v_v_v_b
#define v_i32_u32_sel2_grt_v_v_v_v(a, b, c, d)               v_i32_u32_sel2_grt_v_v_v_v_b(a, b, c, d, (uint64_int64_pair_t){0}, 1, 0)
#define v_i32_u32_sel2_grt_v_s_v_v                           v_i32_u32_sel2_grt_v_v_v_v
#define v_i32_u32_sel2_grt_v_v_v_s                           v_i32_u32_sel2_grt_v_v_v_v

#define v_i32_u32_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u32_sel2_grt_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_u32_sel2_grt_v_s_v_v_vb                        v_i32_u32_sel2_grt_v_v_v_v_vb
#define v_i32_u32_sel2_grt_v_v_v_s_vb                        v_i32_u32_sel2_grt_v_v_v_v_vb

// u32 - f32
#define v_u32_f32_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_f32_sel2_grt_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_f32_sel2_grt_v_s_v_v_b                         v_u32_f32_sel2_grt_v_v_v_v_b
#define v_u32_f32_sel2_grt_v_v_v_s_b                         v_u32_f32_sel2_grt_v_v_v_v_b
#define v_u32_f32_sel2_grt_v_v_v_v(a, b, c, d)               v_u32_f32_sel2_grt_v_v_v_v_b(a, b, c, d, (float64_uint64_pair_t){0}, 1, 0)
#define v_u32_f32_sel2_grt_v_s_v_v                           v_u32_f32_sel2_grt_v_v_v_v
#define v_u32_f32_sel2_grt_v_v_v_s                           v_u32_f32_sel2_grt_v_v_v_v

#define v_u32_f32_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_f32_sel2_grt_u32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_u32_f32_sel2_grt_v_s_v_v_vb                        v_u32_f32_sel2_grt_v_v_v_v_vb
#define v_u32_f32_sel2_grt_v_v_v_s_vb                        v_u32_f32_sel2_grt_v_v_v_v_vb

// u32 - i32
#define v_u32_i32_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_i32_sel2_grt_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_i32_sel2_grt_v_s_v_v_b                         v_u32_i32_sel2_grt_v_v_v_v_b
#define v_u32_i32_sel2_grt_v_v_v_s_b                         v_u32_i32_sel2_grt_v_v_v_v_b
#define v_u32_i32_sel2_grt_v_v_v_v(a, b, c, d)               v_u32_i32_sel2_grt_v_v_v_v_b(a, b, c, d, (int64_uint64_pair_t){0}, 1, 0)
#define v_u32_i32_sel2_grt_v_s_v_v                           v_u32_i32_sel2_grt_v_v_v_v
#define v_u32_i32_sel2_grt_v_v_v_s                           v_u32_i32_sel2_grt_v_v_v_v

#define v_u32_i32_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i32_sel2_grt_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_i32_sel2_grt_v_s_v_v_vb                        v_u32_i32_sel2_grt_v_v_v_v_vb
#define v_u32_i32_sel2_grt_v_v_v_s_vb                        v_u32_i32_sel2_grt_v_v_v_v_vb

// u32 - u32
#define v_u32_u32_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_u32_sel2_grt_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_u32_sel2_grt_v_s_v_v_b                         v_u32_u32_sel2_grt_v_v_v_v_b
#define v_u32_u32_sel2_grt_v_v_v_s_b                         v_u32_u32_sel2_grt_v_v_v_v_b
#define v_u32_u32_sel2_grt_v_v_v_v(a, b, c, d)               v_u32_u32_sel2_grt_v_v_v_v_b(a, b, c, d, (uint64_pair_t){0}, 1, 0)
#define v_u32_u32_sel2_grt_v_s_v_v                           v_u32_u32_sel2_grt_v_v_v_v
#define v_u32_u32_sel2_grt_v_v_v_s                           v_u32_u32_sel2_grt_v_v_v_v

#define v_u32_u32_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u32_sel2_grt_u32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_u32_u32_sel2_grt_v_s_v_v_vb                        v_u32_u32_sel2_grt_v_v_v_v_vb
#define v_u32_u32_sel2_grt_v_v_v_s_vb                        v_u32_u32_sel2_grt_v_v_v_v_vb

// i16 - bf16
#define v_i16_bf16_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel2_grt_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_bf16_sel2_grt_v_s_v_v_b                         v_i16_bf16_sel2_grt_v_v_v_v_b
#define v_i16_bf16_sel2_grt_v_v_v_s_b                         v_i16_bf16_sel2_grt_v_v_v_v_b
#define v_i16_bf16_sel2_grt_v_v_v_v(a, b, c, d)               v_i16_bf16_sel2_grt_v_v_v_v_b(a, b, c, d, (bfloat128_short128_pair_t){0}, 1, 0)
#define v_i16_bf16_sel2_grt_v_s_v_v                           v_i16_bf16_sel2_grt_v_v_v_v
#define v_i16_bf16_sel2_grt_v_v_v_s                           v_i16_bf16_sel2_grt_v_v_v_v

#define v_i16_bf16_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel2_grt_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_bf16_sel2_grt_v_s_v_v_vb                        v_i16_bf16_sel2_grt_v_v_v_v_vb
#define v_i16_bf16_sel2_grt_v_v_v_s_vb                        v_i16_bf16_sel2_grt_v_v_v_v_vb

// i16 - i16
#define v_i16_i16_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel2_grt_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_i16_sel2_grt_v_s_v_v_b                         v_i16_i16_sel2_grt_v_v_v_v_b
#define v_i16_i16_sel2_grt_v_v_v_s_b                         v_i16_i16_sel2_grt_v_v_v_v_b
#define v_i16_i16_sel2_grt_v_v_v_v(a, b, c, d)               v_i16_i16_sel2_grt_v_v_v_v_b(a, b, c, d, (short128_pair_t){0}, 1, 0)
#define v_i16_i16_sel2_grt_v_s_v_v                           v_i16_i16_sel2_grt_v_v_v_v
#define v_i16_i16_sel2_grt_v_v_v_s                           v_i16_i16_sel2_grt_v_v_v_v

#define v_i16_i16_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel2_grt_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_i16_sel2_grt_v_s_v_v_vb                        v_i16_i16_sel2_grt_v_v_v_v_vb
#define v_i16_i16_sel2_grt_v_v_v_s_vb                        v_i16_i16_sel2_grt_v_v_v_v_vb

// i16 - u16
#define v_i16_u16_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel2_grt_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_u16_sel2_grt_v_s_v_v_b                         v_i16_u16_sel2_grt_v_v_v_v_b
#define v_i16_u16_sel2_grt_v_v_v_s_b                         v_i16_u16_sel2_grt_v_v_v_v_b
#define v_i16_u16_sel2_grt_v_v_v_v(a, b, c, d)               v_i16_u16_sel2_grt_v_v_v_v_b(a, b, c, d, (ushort128_short128_pair_t){0}, 1, 0)
#define v_i16_u16_sel2_grt_v_s_v_v                           v_i16_u16_sel2_grt_v_v_v_v
#define v_i16_u16_sel2_grt_v_v_v_s                           v_i16_u16_sel2_grt_v_v_v_v

#define v_i16_u16_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel2_grt_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_u16_sel2_grt_v_s_v_v_vb                        v_i16_u16_sel2_grt_v_v_v_v_vb
#define v_i16_u16_sel2_grt_v_v_v_s_vb                        v_i16_u16_sel2_grt_v_v_v_v_vb

// u16 - bf16
#define v_u16_bf16_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel2_grt_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_bf16_sel2_grt_v_s_v_v_b                         v_u16_bf16_sel2_grt_v_v_v_v_b
#define v_u16_bf16_sel2_grt_v_v_v_s_b                         v_u16_bf16_sel2_grt_v_v_v_v_b
#define v_u16_bf16_sel2_grt_v_v_v_v(a, b, c, d)               v_u16_bf16_sel2_grt_v_v_v_v_b(a, b, c, d, (bfloat128_ushort128_pair_t){0}, 1, 0)
#define v_u16_bf16_sel2_grt_v_s_v_v                           v_u16_bf16_sel2_grt_v_v_v_v
#define v_u16_bf16_sel2_grt_v_v_v_s                           v_u16_bf16_sel2_grt_v_v_v_v

#define v_u16_bf16_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel2_grt_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_bf16_sel2_grt_v_s_v_v_vb                        v_u16_bf16_sel2_grt_v_v_v_v_vb
#define v_u16_bf16_sel2_grt_v_v_v_s_vb                        v_u16_bf16_sel2_grt_v_v_v_v_vb

// u16 - i16
#define v_u16_i16_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel2_grt_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_i16_sel2_grt_v_s_v_v_b                         v_u16_i16_sel2_grt_v_v_v_v_b
#define v_u16_i16_sel2_grt_v_v_v_s_b                         v_u16_i16_sel2_grt_v_v_v_v_b
#define v_u16_i16_sel2_grt_v_v_v_v(a, b, c, d)               v_u16_i16_sel2_grt_v_v_v_v_b(a, b, c, d, (short128_ushort128_pair_t){0}, 1, 0)
#define v_u16_i16_sel2_grt_v_s_v_v                           v_u16_i16_sel2_grt_v_v_v_v
#define v_u16_i16_sel2_grt_v_v_v_s                           v_u16_i16_sel2_grt_v_v_v_v

#define v_u16_i16_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel2_grt_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_i16_sel2_grt_v_s_v_v_vb                        v_u16_i16_sel2_grt_v_v_v_v_vb
#define v_u16_i16_sel2_grt_v_v_v_s_vb                        v_u16_i16_sel2_grt_v_v_v_v_vb

// u16 - u16
#define v_u16_u16_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel2_grt_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_u16_sel2_grt_v_s_v_v_b                         v_u16_u16_sel2_grt_v_v_v_v_b
#define v_u16_u16_sel2_grt_v_v_v_s_b                         v_u16_u16_sel2_grt_v_v_v_v_b
#define v_u16_u16_sel2_grt_v_v_v_v(a, b, c, d)               v_u16_u16_sel2_grt_v_v_v_v_b(a, b, c, d, (ushort128_pair_t){0}, 1, 0)
#define v_u16_u16_sel2_grt_v_s_v_v                           v_u16_u16_sel2_grt_v_v_v_v
#define v_u16_u16_sel2_grt_v_v_v_s                           v_u16_u16_sel2_grt_v_v_v_v

#define v_u16_u16_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel2_grt_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_u16_sel2_grt_v_s_v_v_vb                        v_u16_u16_sel2_grt_v_v_v_v_vb
#define v_u16_u16_sel2_grt_v_v_v_s_vb                        v_u16_u16_sel2_grt_v_v_v_v_vb

// i8 - i8
#define v_i8_i8_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_i8_sel2_grt_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel2_grt_v_s_v_v_b                         v_i8_i8_sel2_grt_v_v_v_v_b
#define v_i8_i8_sel2_grt_v_v_v_s_b                         v_i8_i8_sel2_grt_v_v_v_v_b
#define v_i8_i8_sel2_grt_v_v_v_v(a, b, c, d)               v_i8_i8_sel2_grt_v_v_v_v_b(a, b, c, d, (char256_pair_t){0}, 1, 0)
#define v_i8_i8_sel2_grt_v_s_v_v                           v_i8_i8_sel2_grt_v_v_v_v
#define v_i8_i8_sel2_grt_v_v_v_s                           v_i8_i8_sel2_grt_v_v_v_v

#define v_i8_i8_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i8_sel2_grt_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel2_grt_v_s_v_v_vb                        v_i8_i8_sel2_grt_v_v_v_v_vb
#define v_i8_i8_sel2_grt_v_v_v_s_vb                        v_i8_i8_sel2_grt_v_v_v_v_vb

// i8 - u8
#define v_i8_u8_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_u8_sel2_grt_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel2_grt_v_s_v_v_b                         v_i8_u8_sel2_grt_v_v_v_v_b
#define v_i8_u8_sel2_grt_v_v_v_s_b                         v_i8_u8_sel2_grt_v_v_v_v_b
#define v_i8_u8_sel2_grt_v_v_v_v(a, b, c, d)               v_i8_u8_sel2_grt_v_v_v_v_b(a, b, c, d, (uchar256_char256_pair_t){0}, 1, 0)
#define v_i8_u8_sel2_grt_v_s_v_v                           v_i8_u8_sel2_grt_v_v_v_v
#define v_i8_u8_sel2_grt_v_v_v_s                           v_i8_u8_sel2_grt_v_v_v_v

#define v_i8_u8_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u8_sel2_grt_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel2_grt_v_s_v_v_vb                        v_i8_u8_sel2_grt_v_v_v_v_vb
#define v_i8_u8_sel2_grt_v_v_v_s_vb                        v_i8_u8_sel2_grt_v_v_v_v_vb

// u8 - i8
#define v_u8_i8_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_i8_sel2_grt_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel2_grt_v_s_v_v_b                         v_u8_i8_sel2_grt_v_v_v_v_b
#define v_u8_i8_sel2_grt_v_v_v_s_b                         v_u8_i8_sel2_grt_v_v_v_v_b
#define v_u8_i8_sel2_grt_v_v_v_v(a, b, c, d)               v_u8_i8_sel2_grt_v_v_v_v_b(a, b, c, d, (char256_uchar256_pair_t){0}, 1, 0)
#define v_u8_i8_sel2_grt_v_s_v_v                           v_u8_i8_sel2_grt_v_v_v_v
#define v_u8_i8_sel2_grt_v_v_v_s                           v_u8_i8_sel2_grt_v_v_v_v

#define v_u8_i8_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i8_sel2_grt_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel2_grt_v_s_v_v_vb                        v_u8_i8_sel2_grt_v_v_v_v_vb
#define v_u8_i8_sel2_grt_v_v_v_s_vb                        v_u8_i8_sel2_grt_v_v_v_v_vb

// u8 - u8
#define v_u8_u8_sel2_grt_v_v_v_v_b(a, b, c, d, i, p, o)    v_u8_sel2_grt_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel2_grt_v_s_v_v_b                         v_u8_u8_sel2_grt_v_v_v_v_b
#define v_u8_u8_sel2_grt_v_v_v_s_b                         v_u8_u8_sel2_grt_v_v_v_v_b
#define v_u8_u8_sel2_grt_v_v_v_v(a, b, c, d)               v_u8_u8_sel2_grt_v_v_v_v_b(a, b, c, d, (uchar256_pair_t){0}, 1, 0)
#define v_u8_u8_sel2_grt_v_s_v_v                           v_u8_u8_sel2_grt_v_v_v_v
#define v_u8_u8_sel2_grt_v_v_v_s                           v_u8_u8_sel2_grt_v_v_v_v

#define v_u8_u8_sel2_grt_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u8_sel2_grt_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel2_grt_v_s_v_v_vb                        v_u8_u8_sel2_grt_v_v_v_v_vb
#define v_u8_u8_sel2_grt_v_v_v_s_vb                        v_u8_u8_sel2_grt_v_v_v_v_vb

// SEL2_LEQ

// f32 - f32
#define v_f32_f32_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_f32_sel2_leq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_f32_sel2_leq_v_s_v_v_b                         v_f32_f32_sel2_leq_v_v_v_v_b
#define v_f32_f32_sel2_leq_v_v_v_s_b                         v_f32_f32_sel2_leq_v_v_v_v_b
#define v_f32_f32_sel2_leq_v_v_v_v(a, b, c, d)               v_f32_f32_sel2_leq_v_v_v_v_b(a, b, c, d, (float64_pair_t){0}, 1, 0)
#define v_f32_f32_sel2_leq_v_s_v_v                           v_f32_f32_sel2_leq_v_v_v_v
#define v_f32_f32_sel2_leq_v_v_v_s                           v_f32_f32_sel2_leq_v_v_v_v

#define v_f32_f32_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_f32_sel2_leq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_f32_f32_sel2_leq_v_v_v_s_vb                        v_f32_f32_sel2_leq_v_v_v_v_vb
#define v_f32_f32_sel2_leq_v_s_v_v_vb                        v_f32_f32_sel2_leq_v_v_v_v_vb

// f32 - i32
#define v_f32_i32_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i32_sel2_leq_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_i32_sel2_leq_v_s_v_v_b                         v_f32_i32_sel2_leq_v_v_v_v_b
#define v_f32_i32_sel2_leq_v_v_v_s_b                         v_f32_i32_sel2_leq_v_v_v_v_b
#define v_f32_i32_sel2_leq_v_v_v_v(a, b, c, d)               v_f32_i32_sel2_leq_v_v_v_v_b(a, b, c, d, (int64_float64_pair_t){0}, 1, 0)
#define v_f32_i32_sel2_leq_v_s_v_v                           v_f32_i32_sel2_leq_v_v_v_v
#define v_f32_i32_sel2_leq_v_v_v_s                           v_f32_i32_sel2_leq_v_v_v_v

#define v_f32_i32_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i32_sel2_leq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_i32_sel2_leq_v_v_v_s_vb                        v_f32_i32_sel2_leq_v_v_v_v_vb
#define v_f32_i32_sel2_leq_v_s_v_v_vb                        v_f32_i32_sel2_leq_v_v_v_v_vb

// f32 - u32
#define v_f32_u32_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u32_sel2_leq_f32_b(a, b, c, d, 0, i, p, o);
#define v_f32_u32_sel2_leq_v_s_v_v_b                         v_f32_u32_sel2_leq_v_v_v_v_b
#define v_f32_u32_sel2_leq_v_v_v_s_b                         v_f32_u32_sel2_leq_v_v_v_v_b
#define v_f32_u32_sel2_leq_v_v_v_v(a, b, c, d)               v_f32_u32_sel2_leq_v_v_v_v_b(a, b, c, d, (uint64_float64_pair_t){0}, 1, 0)
#define v_f32_u32_sel2_leq_v_s_v_v                           v_f32_u32_sel2_leq_v_v_v_v
#define v_f32_u32_sel2_leq_v_v_v_s                           v_f32_u32_sel2_leq_v_v_v_v

#define v_f32_u32_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u32_sel2_leq_f32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_f32_u32_sel2_leq_v_s_v_v_vb                        v_f32_u32_sel2_leq_v_v_v_v_vb
#define v_f32_u32_sel2_leq_v_v_v_s_vb                        v_f32_u32_sel2_leq_v_v_v_v_vb

// bf16 - bf16
#define v_bf16_bf16_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel2_leq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_bf16_sel2_leq_v_s_v_v_b                         v_bf16_bf16_sel2_leq_v_v_v_v_b
#define v_bf16_bf16_sel2_leq_v_v_v_s_b                         v_bf16_bf16_sel2_leq_v_v_v_v_b
#define v_bf16_bf16_sel2_leq_v_v_v_v(a, b, c, d)               v_bf16_bf16_sel2_leq_v_v_v_v_b(a, b, c, d, (bfloat128_bfloat128_pair_t){0}, 1, 0)
#define v_bf16_bf16_sel2_leq_v_s_v_v                           v_bf16_bf16_sel2_leq_v_v_v_v
#define v_bf16_bf16_sel2_leq_v_v_v_s                           v_bf16_bf16_sel2_leq_v_v_v_v

#define v_bf16_bf16_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel2_leq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o);
#define v_bf16_bf16_sel2_leq_v_s_v_v_vb                        v_bf16_bf16_sel2_leq_v_v_v_v_vb
#define v_bf16_bf16_sel2_leq_v_v_v_s_vb                        v_bf16_bf16_sel2_leq_v_v_v_v_vb

// bf16 - i16
#define v_bf16_i16_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel2_leq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_i16_sel2_leq_v_s_v_v_b                         v_bf16_i16_sel2_leq_v_v_v_v_b
#define v_bf16_i16_sel2_leq_v_v_v_s_b                         v_bf16_i16_sel2_leq_v_v_v_v_b
#define v_bf16_i16_sel2_leq_v_v_v_v(a, b, c, d)               v_bf16_i16_sel2_leq_v_v_v_v_b(a, b, c, d, (short128_bfloat128_pair_t){0}, 1, 0)
#define v_bf16_i16_sel2_leq_v_s_v_v                           v_bf16_i16_sel2_leq_v_v_v_v
#define v_bf16_i16_sel2_leq_v_v_v_s                           v_bf16_i16_sel2_leq_v_v_v_v

#define v_bf16_i16_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel2_leq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o);
#define v_bf16_i16_sel2_leq_v_s_v_v_vb                        v_bf16_i16_sel2_leq_v_v_v_v_vb
#define v_bf16_i16_sel2_leq_v_v_v_s_vb                        v_bf16_i16_sel2_leq_v_v_v_v_vb

// bf16 - u16
#define v_bf16_u16_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel2_leq_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_u16_sel2_leq_v_s_v_v_b                         v_bf16_u16_sel2_leq_v_v_v_v_b
#define v_bf16_u16_sel2_leq_v_v_v_s_b                         v_bf16_u16_sel2_leq_v_v_v_v_b
#define v_bf16_u16_sel2_leq_v_v_v_v(a, b, c, d)               v_bf16_u16_sel2_leq_v_v_v_v_b(a, b, c, d, (ushort128_bfloat128_pair_t){0}, 1, 0)
#define v_bf16_u16_sel2_leq_v_s_v_v                           v_bf16_u16_sel2_leq_v_v_v_v
#define v_bf16_u16_sel2_leq_v_v_v_s                           v_bf16_u16_sel2_leq_v_v_v_v

#define v_bf16_u16_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel2_leq_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_u16_sel2_leq_v_s_v_v_vb                        v_bf16_u16_sel2_leq_v_v_v_v_vb
#define v_bf16_u16_sel2_leq_v_v_v_s_vb                        v_bf16_u16_sel2_leq_v_v_v_v_vb

// i32 - f32
#define v_i32_f32_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_f32_sel2_leq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_f32_sel2_leq_v_s_v_v_b                         v_i32_f32_sel2_leq_v_v_v_v_b
#define v_i32_f32_sel2_leq_v_v_v_s_b                         v_i32_f32_sel2_leq_v_v_v_v_b
#define v_i32_f32_sel2_leq_v_v_v_v(a, b, c, d)               v_i32_f32_sel2_leq_v_v_v_v_b(a, b, c, d, (float64_int64_pair_t){0}, 1, 0)
#define v_i32_f32_sel2_leq_v_s_v_v                           v_i32_f32_sel2_leq_v_v_v_v
#define v_i32_f32_sel2_leq_v_v_v_s                           v_i32_f32_sel2_leq_v_v_v_v

#define v_i32_f32_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_f32_sel2_leq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_f32_sel2_leq_v_s_v_v_vb                        v_i32_f32_sel2_leq_v_v_v_v_vb
#define v_i32_f32_sel2_leq_v_v_v_s_vb                        v_i32_f32_sel2_leq_v_v_v_v_vb

// i32 - i32
#define v_i32_i32_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i32_sel2_leq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_i32_sel2_leq_v_s_v_v_b                         v_i32_i32_sel2_leq_v_v_v_v_b
#define v_i32_i32_sel2_leq_v_v_v_s_b                         v_i32_i32_sel2_leq_v_v_v_v_b
#define v_i32_i32_sel2_leq_v_v_v_v(a, b, c, d)               v_i32_i32_sel2_leq_v_v_v_v_b(a, b, c, d, (int64_pair_t){0}, 1, 0)
#define v_i32_i32_sel2_leq_v_s_v_v                           v_i32_i32_sel2_leq_v_v_v_v
#define v_i32_i32_sel2_leq_v_v_v_s                           v_i32_i32_sel2_leq_v_v_v_v

#define v_i32_i32_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i32_sel2_leq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_i32_sel2_leq_v_s_v_v_vb                        v_i32_i32_sel2_leq_v_v_v_v_vb
#define v_i32_i32_sel2_leq_v_v_v_s_vb                        v_i32_i32_sel2_leq_v_v_v_v_vb

// i32 - u32
#define v_i32_u32_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u32_sel2_leq_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_u32_sel2_leq_v_s_v_v_b                         v_i32_u32_sel2_leq_v_v_v_v_b
#define v_i32_u32_sel2_leq_v_v_v_s_b                         v_i32_u32_sel2_leq_v_v_v_v_b
#define v_i32_u32_sel2_leq_v_v_v_v(a, b, c, d)               v_i32_u32_sel2_leq_v_v_v_v_b(a, b, c, d, (uint64_int64_pair_t){0}, 1, 0)
#define v_i32_u32_sel2_leq_v_s_v_v                           v_i32_u32_sel2_leq_v_v_v_v
#define v_i32_u32_sel2_leq_v_v_v_s                           v_i32_u32_sel2_leq_v_v_v_v

#define v_i32_u32_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u32_sel2_leq_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_u32_sel2_leq_v_s_v_v_vb                        v_i32_u32_sel2_leq_v_v_v_v_vb
#define v_i32_u32_sel2_leq_v_v_v_s_vb                        v_i32_u32_sel2_leq_v_v_v_v_vb

// u32 - f32
#define v_u32_f32_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_f32_sel2_leq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_f32_sel2_leq_v_s_v_v_b                         v_u32_f32_sel2_leq_v_v_v_v_b
#define v_u32_f32_sel2_leq_v_v_v_s_b                         v_u32_f32_sel2_leq_v_v_v_v_b
#define v_u32_f32_sel2_leq_v_v_v_v(a, b, c, d)               v_u32_f32_sel2_leq_v_v_v_v_b(a, b, c, d, (float64_uint64_pair_t){0}, 1, 0)
#define v_u32_f32_sel2_leq_v_s_v_v                           v_u32_f32_sel2_leq_v_v_v_v
#define v_u32_f32_sel2_leq_v_v_v_s                           v_u32_f32_sel2_leq_v_v_v_v

#define v_u32_f32_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_f32_sel2_leq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_u32_f32_sel2_leq_v_s_v_v_vb                        v_u32_f32_sel2_leq_v_v_v_v_vb
#define v_u32_f32_sel2_leq_v_v_v_s_vb                        v_u32_f32_sel2_leq_v_v_v_v_vb

// u32 - i32
#define v_u32_i32_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i32_sel2_leq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_i32_sel2_leq_v_s_v_v_b                         v_u32_i32_sel2_leq_v_v_v_v_b
#define v_u32_i32_sel2_leq_v_v_v_s_b                         v_u32_i32_sel2_leq_v_v_v_v_b
#define v_u32_i32_sel2_leq_v_v_v_v(a, b, c, d)               v_u32_i32_sel2_leq_v_v_v_v_b(a, b, c, d, (int64_uint64_pair_t){0}, 1, 0)
#define v_u32_i32_sel2_leq_v_s_v_v                           v_u32_i32_sel2_leq_v_v_v_v
#define v_u32_i32_sel2_leq_v_v_v_s                           v_u32_i32_sel2_leq_v_v_v_v

#define v_u32_i32_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i32_sel2_leq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_i32_sel2_leq_v_s_v_v_vb                        v_u32_i32_sel2_leq_v_v_v_v_vb
#define v_u32_i32_sel2_leq_v_v_v_s_vb                        v_u32_i32_sel2_leq_v_v_v_v_vb

// u32 - u32
#define v_u32_u32_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u32_sel2_leq_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_u32_sel2_leq_v_s_v_v_b                         v_u32_u32_sel2_leq_v_v_v_v_b
#define v_u32_u32_sel2_leq_v_v_v_s_b                         v_u32_u32_sel2_leq_v_v_v_v_b
#define v_u32_u32_sel2_leq_v_v_v_v(a, b, c, d)               v_u32_u32_sel2_leq_v_v_v_v_b(a, b, c, d, (uint64_pair_t){0}, 1, 0)
#define v_u32_u32_sel2_leq_v_s_v_v                           v_u32_u32_sel2_leq_v_v_v_v
#define v_u32_u32_sel2_leq_v_v_v_s                           v_u32_u32_sel2_leq_v_v_v_v

#define v_u32_u32_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u32_sel2_leq_u32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_u32_u32_sel2_leq_v_s_v_v_vb                        v_u32_u32_sel2_leq_v_v_v_v_vb
#define v_u32_u32_sel2_leq_v_v_v_s_vb                        v_u32_u32_sel2_leq_v_v_v_v_vb

// i16 - bf16
#define v_i16_bf16_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel2_leq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_bf16_sel2_leq_v_s_v_v_b                         v_i16_bf16_sel2_leq_v_v_v_v_b
#define v_i16_bf16_sel2_leq_v_v_v_s_b                         v_i16_bf16_sel2_leq_v_v_v_v_b
#define v_i16_bf16_sel2_leq_v_v_v_v(a, b, c, d)               v_i16_bf16_sel2_leq_v_v_v_v_b(a, b, c, d, (bfloat128_short128_pair_t){0}, 1, 0)
#define v_i16_bf16_sel2_leq_v_s_v_v                           v_i16_bf16_sel2_leq_v_v_v_v
#define v_i16_bf16_sel2_leq_v_v_v_s                           v_i16_bf16_sel2_leq_v_v_v_v

#define v_i16_bf16_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel2_leq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_bf16_sel2_leq_v_s_v_v_vb                        v_i16_bf16_sel2_leq_v_v_v_v_vb
#define v_i16_bf16_sel2_leq_v_v_v_s_vb                        v_i16_bf16_sel2_leq_v_v_v_v_vb

// i16 - i16
#define v_i16_i16_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel2_leq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_i16_sel2_leq_v_s_v_v_b                         v_i16_i16_sel2_leq_v_v_v_v_b
#define v_i16_i16_sel2_leq_v_v_v_s_b                         v_i16_i16_sel2_leq_v_v_v_v_b
#define v_i16_i16_sel2_leq_v_v_v_v(a, b, c, d)               v_i16_i16_sel2_leq_v_v_v_v_b(a, b, c, d, (short128_pair_t){0}, 1, 0)
#define v_i16_i16_sel2_leq_v_s_v_v                           v_i16_i16_sel2_leq_v_v_v_v
#define v_i16_i16_sel2_leq_v_v_v_s                           v_i16_i16_sel2_leq_v_v_v_v

#define v_i16_i16_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel2_leq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_i16_sel2_leq_v_s_v_v_vb                        v_i16_i16_sel2_leq_v_v_v_v_vb
#define v_i16_i16_sel2_leq_v_v_v_s_vb                        v_i16_i16_sel2_leq_v_v_v_v_vb

// i16 - u16
#define v_i16_u16_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel2_leq_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_u16_sel2_leq_v_s_v_v_b                         v_i16_u16_sel2_leq_v_v_v_v_b
#define v_i16_u16_sel2_leq_v_v_v_s_b                         v_i16_u16_sel2_leq_v_v_v_v_b
#define v_i16_u16_sel2_leq_v_v_v_v(a, b, c, d)               v_i16_u16_sel2_leq_v_v_v_v_b(a, b, c, d, (ushort128_short128_pair_t){0}, 1, 0)
#define v_i16_u16_sel2_leq_v_s_v_v                           v_i16_u16_sel2_leq_v_v_v_v
#define v_i16_u16_sel2_leq_v_v_v_s                           v_i16_u16_sel2_leq_v_v_v_v

#define v_i16_u16_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel2_leq_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_u16_sel2_leq_v_s_v_v_vb                        v_i16_u16_sel2_leq_v_v_v_v_vb
#define v_i16_u16_sel2_leq_v_v_v_s_vb                        v_i16_u16_sel2_leq_v_v_v_v_vb

// u16 - bf16
#define v_u16_bf16_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel2_leq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_bf16_sel2_leq_v_s_v_v_b                         v_u16_bf16_sel2_leq_v_v_v_v_b
#define v_u16_bf16_sel2_leq_v_v_v_s_b                         v_u16_bf16_sel2_leq_v_v_v_v_b
#define v_u16_bf16_sel2_leq_v_v_v_v(a, b, c, d)               v_u16_bf16_sel2_leq_v_v_v_v_b(a, b, c, d, (bfloat128_ushort128_pair_t){0}, 1, 0)
#define v_u16_bf16_sel2_leq_v_s_v_v                           v_u16_bf16_sel2_leq_v_v_v_v
#define v_u16_bf16_sel2_leq_v_v_v_s                           v_u16_bf16_sel2_leq_v_v_v_v

#define v_u16_bf16_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel2_leq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_bf16_sel2_leq_v_s_v_v_vb                        v_u16_bf16_sel2_leq_v_v_v_v_vb
#define v_u16_bf16_sel2_leq_v_v_v_s_vb                        v_u16_bf16_sel2_leq_v_v_v_v_vb

// u16 - i16
#define v_u16_i16_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel2_leq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_i16_sel2_leq_v_s_v_v_b                         v_u16_i16_sel2_leq_v_v_v_v_b
#define v_u16_i16_sel2_leq_v_v_v_s_b                         v_u16_i16_sel2_leq_v_v_v_v_b
#define v_u16_i16_sel2_leq_v_v_v_v(a, b, c, d)               v_u16_i16_sel2_leq_v_v_v_v_b(a, b, c, d, (short128_ushort128_pair_t){0}, 1, 0)
#define v_u16_i16_sel2_leq_v_s_v_v                           v_u16_i16_sel2_leq_v_v_v_v
#define v_u16_i16_sel2_leq_v_v_v_s                           v_u16_i16_sel2_leq_v_v_v_v

#define v_u16_i16_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel2_leq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_i16_sel2_leq_v_s_v_v_vb                        v_u16_i16_sel2_leq_v_v_v_v_vb
#define v_u16_i16_sel2_leq_v_v_v_s_vb                        v_u16_i16_sel2_leq_v_v_v_v_vb

// u16 - u16
#define v_u16_u16_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel2_leq_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_u16_sel2_leq_v_s_v_v_b                         v_u16_u16_sel2_leq_v_v_v_v_b
#define v_u16_u16_sel2_leq_v_v_v_s_b                         v_u16_u16_sel2_leq_v_v_v_v_b
#define v_u16_u16_sel2_leq_v_v_v_v(a, b, c, d)               v_u16_u16_sel2_leq_v_v_v_v_b(a, b, c, d, (ushort128_pair_t){0}, 1, 0)
#define v_u16_u16_sel2_leq_v_s_v_v                           v_u16_u16_sel2_leq_v_v_v_v
#define v_u16_u16_sel2_leq_v_v_v_s                           v_u16_u16_sel2_leq_v_v_v_v

#define v_u16_u16_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel2_leq_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_u16_sel2_leq_v_s_v_v_vb                        v_u16_u16_sel2_leq_v_v_v_v_vb
#define v_u16_u16_sel2_leq_v_v_v_s_vb                        v_u16_u16_sel2_leq_v_v_v_v_vb

// i8 - i8
#define v_i8_i8_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i8_sel2_leq_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel2_leq_v_s_v_v_b                         v_i8_i8_sel2_leq_v_v_v_v_b
#define v_i8_i8_sel2_leq_v_v_v_s_b                         v_i8_i8_sel2_leq_v_v_v_v_b
#define v_i8_i8_sel2_leq_v_v_v_v(a, b, c, d)               v_i8_i8_sel2_leq_v_v_v_v_b(a, b, c, d, (char256_pair_t){0}, 1, 0)
#define v_i8_i8_sel2_leq_v_s_v_v                           v_i8_i8_sel2_leq_v_v_v_v
#define v_i8_i8_sel2_leq_v_v_v_s                           v_i8_i8_sel2_leq_v_v_v_v

#define v_i8_i8_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i8_sel2_leq_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel2_leq_v_s_v_v_vb                        v_i8_i8_sel2_leq_v_v_v_v_vb
#define v_i8_i8_sel2_leq_v_v_v_s_vb                        v_i8_i8_sel2_leq_v_v_v_v_vb

// i8 - u8
#define v_i8_u8_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u8_sel2_leq_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel2_leq_v_s_v_v_b                         v_i8_u8_sel2_leq_v_v_v_v_b
#define v_i8_u8_sel2_leq_v_v_v_s_b                         v_i8_u8_sel2_leq_v_v_v_v_b
#define v_i8_u8_sel2_leq_v_v_v_v(a, b, c, d)               v_i8_u8_sel2_leq_v_v_v_v_b(a, b, c, d, (uchar256_char256_pair_t){0}, 1, 0)
#define v_i8_u8_sel2_leq_v_s_v_v                           v_i8_u8_sel2_leq_v_v_v_v
#define v_i8_u8_sel2_leq_v_v_v_s                           v_i8_u8_sel2_leq_v_v_v_v

#define v_i8_u8_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u8_sel2_leq_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel2_leq_v_s_v_v_vb                        v_i8_u8_sel2_leq_v_v_v_v_vb
#define v_i8_u8_sel2_leq_v_v_v_s_vb                        v_i8_u8_sel2_leq_v_v_v_v_vb

// u8 - i8
#define v_u8_i8_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_i8_sel2_leq_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel2_leq_v_s_v_v_b                         v_u8_i8_sel2_leq_v_v_v_v_b
#define v_u8_i8_sel2_leq_v_v_v_s_b                         v_u8_i8_sel2_leq_v_v_v_v_b
#define v_u8_i8_sel2_leq_v_v_v_v(a, b, c, d)               v_u8_i8_sel2_leq_v_v_v_v_b(a, b, c, d, (char256_uchar256_pair_t){0}, 1, 0)
#define v_u8_i8_sel2_leq_v_s_v_v                           v_u8_i8_sel2_leq_v_v_v_v
#define v_u8_i8_sel2_leq_v_v_v_s                           v_u8_i8_sel2_leq_v_v_v_v

#define v_u8_i8_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i8_sel2_leq_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel2_leq_v_s_v_v_vb                        v_u8_i8_sel2_leq_v_v_v_v_vb
#define v_u8_i8_sel2_leq_v_v_v_s_vb                        v_u8_i8_sel2_leq_v_v_v_v_vb

// u8 - u8
#define v_u8_u8_sel2_leq_v_v_v_v_b(a, b, c, d, i, p, o)    v_u8_sel2_leq_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel2_leq_v_s_v_v_b                         v_u8_u8_sel2_leq_v_v_v_v_b
#define v_u8_u8_sel2_leq_v_v_v_s_b                         v_u8_u8_sel2_leq_v_v_v_v_b
#define v_u8_u8_sel2_leq_v_v_v_v(a, b, c, d)               v_u8_u8_sel2_leq_v_v_v_v_b(a, b, c, d, (uchar256_pair_t){0}, 1, 0)
#define v_u8_u8_sel2_leq_v_s_v_v                           v_u8_u8_sel2_leq_v_v_v_v
#define v_u8_u8_sel2_leq_v_v_v_s                           v_u8_u8_sel2_leq_v_v_v_v

#define v_u8_u8_sel2_leq_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u8_sel2_leq_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel2_leq_v_s_v_v_vb                        v_u8_u8_sel2_leq_v_v_v_v_vb
#define v_u8_u8_sel2_leq_v_v_v_s_vb                        v_u8_u8_sel2_leq_v_v_v_v_vb

// SEL2_LESS

// f32 - f32
#define v_f32_f32_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_f32_sel2_less_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_f32_sel2_less_v_s_v_v_b                         v_f32_f32_sel2_less_v_v_v_v_b
#define v_f32_f32_sel2_less_v_v_v_s_b                         v_f32_f32_sel2_less_v_v_v_v_b
#define v_f32_f32_sel2_less_v_v_v_v(a, b, c, d)               v_f32_f32_sel2_less_v_v_v_v_b(a, b, c, d, (float64_pair_t){0}, 1, 0)
#define v_f32_f32_sel2_less_v_s_v_v                           v_f32_f32_sel2_less_v_v_v_v
#define v_f32_f32_sel2_less_v_v_v_s                           v_f32_f32_sel2_less_v_v_v_v

#define v_f32_f32_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_f32_sel2_less_f32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_f32_f32_sel2_less_v_v_v_s_vb                        v_f32_f32_sel2_less_v_v_v_v_vb
#define v_f32_f32_sel2_less_v_s_v_v_vb                        v_f32_f32_sel2_less_v_v_v_v_vb

// f32 - i32
#define v_f32_i32_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_i32_sel2_less_f32_b(a, b, c, d, 0, i, p, o)
#define v_f32_i32_sel2_less_v_s_v_v_b                         v_f32_i32_sel2_less_v_v_v_v_b
#define v_f32_i32_sel2_less_v_v_v_s_b                         v_f32_i32_sel2_less_v_v_v_v_b
#define v_f32_i32_sel2_less_v_v_v_v(a, b, c, d)               v_f32_i32_sel2_less_v_v_v_v_b(a, b, c, d, (int64_float64_pair_t){0}, 1, 0)
#define v_f32_i32_sel2_less_v_s_v_v                           v_f32_i32_sel2_less_v_v_v_v
#define v_f32_i32_sel2_less_v_v_v_s                           v_f32_i32_sel2_less_v_v_v_v

#define v_f32_i32_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i32_sel2_less_f32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_f32_i32_sel2_less_v_v_v_s_vb                        v_f32_i32_sel2_less_v_v_v_v_vb
#define v_f32_i32_sel2_less_v_s_v_v_vb                        v_f32_i32_sel2_less_v_v_v_v_vb

// f32 - u32
#define v_f32_u32_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_u32_sel2_less_f32_b(a, b, c, d, 0, i, p, o);
#define v_f32_u32_sel2_less_v_s_v_v_b                         v_f32_u32_sel2_less_v_v_v_v_b
#define v_f32_u32_sel2_less_v_v_v_s_b                         v_f32_u32_sel2_less_v_v_v_v_b
#define v_f32_u32_sel2_less_v_v_v_v(a, b, c, d)               v_f32_u32_sel2_less_v_v_v_v_b(a, b, c, d, (uint64_float64_pair_t){0}, 1, 0)
#define v_f32_u32_sel2_less_v_s_v_v                           v_f32_u32_sel2_less_v_v_v_v
#define v_f32_u32_sel2_less_v_v_v_s                           v_f32_u32_sel2_less_v_v_v_v

#define v_f32_u32_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u32_sel2_less_f32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_f32_u32_sel2_less_v_s_v_v_vb                        v_f32_u32_sel2_less_v_v_v_v_vb
#define v_f32_u32_sel2_less_v_v_v_s_vb                        v_f32_u32_sel2_less_v_v_v_v_vb

// bf16 - bf16
#define v_bf16_bf16_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel2_less_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_bf16_sel2_less_v_s_v_v_b                         v_bf16_bf16_sel2_less_v_v_v_v_b
#define v_bf16_bf16_sel2_less_v_v_v_s_b                         v_bf16_bf16_sel2_less_v_v_v_v_b
#define v_bf16_bf16_sel2_less_v_v_v_v(a, b, c, d)               v_bf16_bf16_sel2_less_v_v_v_v_b(a, b, c, d, (bfloat128_bfloat128_pair_t){0}, 1, 0)
#define v_bf16_bf16_sel2_less_v_s_v_v                           v_bf16_bf16_sel2_less_v_v_v_v
#define v_bf16_bf16_sel2_less_v_v_v_s                           v_bf16_bf16_sel2_less_v_v_v_v

#define v_bf16_bf16_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel2_less_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o);
#define v_bf16_bf16_sel2_less_v_s_v_v_vb                        v_bf16_bf16_sel2_less_v_v_v_v_vb
#define v_bf16_bf16_sel2_less_v_v_v_s_vb                        v_bf16_bf16_sel2_less_v_v_v_v_vb

// bf16 - i16
#define v_bf16_i16_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel2_less_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_i16_sel2_less_v_s_v_v_b                         v_bf16_i16_sel2_less_v_v_v_v_b
#define v_bf16_i16_sel2_less_v_v_v_s_b                         v_bf16_i16_sel2_less_v_v_v_v_b
#define v_bf16_i16_sel2_less_v_v_v_v(a, b, c, d)               v_bf16_i16_sel2_less_v_v_v_v_b(a, b, c, d, (short128_bfloat128_pair_t){0}, 1, 0)
#define v_bf16_i16_sel2_less_v_s_v_v                           v_bf16_i16_sel2_less_v_v_v_v
#define v_bf16_i16_sel2_less_v_v_v_s                           v_bf16_i16_sel2_less_v_v_v_v

#define v_bf16_i16_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel2_less_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o);
#define v_bf16_i16_sel2_less_v_s_v_v_vb                        v_bf16_i16_sel2_less_v_v_v_v_vb
#define v_bf16_i16_sel2_less_v_v_v_s_vb                        v_bf16_i16_sel2_less_v_v_v_v_vb

// bf16 - u16
#define v_bf16_u16_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel2_less_bf16_b(a, b, c, d, 0, i, p, o)
#define v_bf16_u16_sel2_less_v_s_v_v_b                         v_bf16_u16_sel2_less_v_v_v_v_b
#define v_bf16_u16_sel2_less_v_v_v_s_b                         v_bf16_u16_sel2_less_v_v_v_v_b
#define v_bf16_u16_sel2_less_v_v_v_v(a, b, c, d)               v_bf16_u16_sel2_less_v_v_v_v_b(a, b, c, d, (ushort128_bfloat128_pair_t){0}, 1, 0)
#define v_bf16_u16_sel2_less_v_s_v_v                           v_bf16_u16_sel2_less_v_v_v_v
#define v_bf16_u16_sel2_less_v_v_v_s                           v_bf16_u16_sel2_less_v_v_v_v

#define v_bf16_u16_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel2_less_bf16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_bf16_u16_sel2_less_v_s_v_v_vb                        v_bf16_u16_sel2_less_v_v_v_v_vb
#define v_bf16_u16_sel2_less_v_v_v_s_vb                        v_bf16_u16_sel2_less_v_v_v_v_vb

// i32 - f32
#define v_i32_f32_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_f32_sel2_less_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_f32_sel2_less_v_s_v_v_b                         v_i32_f32_sel2_less_v_v_v_v_b
#define v_i32_f32_sel2_less_v_v_v_s_b                         v_i32_f32_sel2_less_v_v_v_v_b
#define v_i32_f32_sel2_less_v_v_v_v(a, b, c, d)               v_i32_f32_sel2_less_v_v_v_v_b(a, b, c, d, (float64_int64_pair_t){0}, 1, 0)
#define v_i32_f32_sel2_less_v_s_v_v                           v_i32_f32_sel2_less_v_v_v_v
#define v_i32_f32_sel2_less_v_v_v_s                           v_i32_f32_sel2_less_v_v_v_v

#define v_i32_f32_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_f32_sel2_less_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_f32_sel2_less_v_s_v_v_vb                        v_i32_f32_sel2_less_v_v_v_v_vb
#define v_i32_f32_sel2_less_v_v_v_s_vb                        v_i32_f32_sel2_less_v_v_v_v_vb

// i32 - i32
#define v_i32_i32_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_i32_sel2_less_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_i32_sel2_less_v_s_v_v_b                         v_i32_i32_sel2_less_v_v_v_v_b
#define v_i32_i32_sel2_less_v_v_v_s_b                         v_i32_i32_sel2_less_v_v_v_v_b
#define v_i32_i32_sel2_less_v_v_v_v(a, b, c, d)               v_i32_i32_sel2_less_v_v_v_v_b(a, b, c, d, (int64_pair_t){0}, 1, 0)
#define v_i32_i32_sel2_less_v_s_v_v                           v_i32_i32_sel2_less_v_v_v_v
#define v_i32_i32_sel2_less_v_v_v_s                           v_i32_i32_sel2_less_v_v_v_v

#define v_i32_i32_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i32_sel2_less_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_i32_sel2_less_v_s_v_v_vb                        v_i32_i32_sel2_less_v_v_v_v_vb
#define v_i32_i32_sel2_less_v_v_v_s_vb                        v_i32_i32_sel2_less_v_v_v_v_vb

// i32 - u32
#define v_i32_u32_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_u32_sel2_less_i32_b(a, b, c, d, 0, i, p, o)
#define v_i32_u32_sel2_less_v_s_v_v_b                         v_i32_u32_sel2_less_v_v_v_v_b
#define v_i32_u32_sel2_less_v_v_v_s_b                         v_i32_u32_sel2_less_v_v_v_v_b
#define v_i32_u32_sel2_less_v_v_v_v(a, b, c, d)               v_i32_u32_sel2_less_v_v_v_v_b(a, b, c, d, (uint64_int64_pair_t){0}, 1, 0)
#define v_i32_u32_sel2_less_v_s_v_v                           v_i32_u32_sel2_less_v_v_v_v
#define v_i32_u32_sel2_less_v_v_v_s                           v_i32_u32_sel2_less_v_v_v_v

#define v_i32_u32_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u32_sel2_less_i32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_i32_u32_sel2_less_v_s_v_v_vb                        v_i32_u32_sel2_less_v_v_v_v_vb
#define v_i32_u32_sel2_less_v_v_v_s_vb                        v_i32_u32_sel2_less_v_v_v_v_vb

// u32 - f32
#define v_u32_f32_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_f32_sel2_less_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_f32_sel2_less_v_s_v_v_b                         v_u32_f32_sel2_less_v_v_v_v_b
#define v_u32_f32_sel2_less_v_v_v_s_b                         v_u32_f32_sel2_less_v_v_v_v_b
#define v_u32_f32_sel2_less_v_v_v_v(a, b, c, d)               v_u32_f32_sel2_less_v_v_v_v_b(a, b, c, d, (float64_uint64_pair_t){0}, 1, 0)
#define v_u32_f32_sel2_less_v_s_v_v                           v_u32_f32_sel2_less_v_v_v_v
#define v_u32_f32_sel2_less_v_v_v_s                           v_u32_f32_sel2_less_v_v_v_v

#define v_u32_f32_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_f32_sel2_less_u32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_u32_f32_sel2_less_v_s_v_v_vb                        v_u32_f32_sel2_less_v_v_v_v_vb
#define v_u32_f32_sel2_less_v_v_v_s_vb                        v_u32_f32_sel2_less_v_v_v_v_vb

// u32 - i32
#define v_u32_i32_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_i32_sel2_less_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_i32_sel2_less_v_s_v_v_b                         v_u32_i32_sel2_less_v_v_v_v_b
#define v_u32_i32_sel2_less_v_v_v_s_b                         v_u32_i32_sel2_less_v_v_v_v_b
#define v_u32_i32_sel2_less_v_v_v_v(a, b, c, d)               v_u32_i32_sel2_less_v_v_v_v_b(a, b, c, d, (int64_uint64_pair_t){0}, 1, 0)
#define v_u32_i32_sel2_less_v_s_v_v                           v_u32_i32_sel2_less_v_v_v_v
#define v_u32_i32_sel2_less_v_v_v_s                           v_u32_i32_sel2_less_v_v_v_v

#define v_u32_i32_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i32_sel2_less_u32_vb(a, b, c, d, 0, i, to_bool64(p), o)
#define v_u32_i32_sel2_less_v_s_v_v_vb                        v_u32_i32_sel2_less_v_v_v_v_vb
#define v_u32_i32_sel2_less_v_v_v_s_vb                        v_u32_i32_sel2_less_v_v_v_v_vb

// u32 - u32
#define v_u32_u32_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_u32_sel2_less_u32_b(a, b, c, d, 0, i, p, o)
#define v_u32_u32_sel2_less_v_s_v_v_b                         v_u32_u32_sel2_less_v_v_v_v_b
#define v_u32_u32_sel2_less_v_v_v_s_b                         v_u32_u32_sel2_less_v_v_v_v_b
#define v_u32_u32_sel2_less_v_v_v_v(a, b, c, d)               v_u32_u32_sel2_less_v_v_v_v_b(a, b, c, d, (uint64_pair_t){0}, 1, 0)
#define v_u32_u32_sel2_less_v_s_v_v                           v_u32_u32_sel2_less_v_v_v_v
#define v_u32_u32_sel2_less_v_v_v_s                           v_u32_u32_sel2_less_v_v_v_v

#define v_u32_u32_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u32_sel2_less_u32_vb(a, b, c, d, 0, i, to_bool64(p), o);
#define v_u32_u32_sel2_less_v_s_v_v_vb                        v_u32_u32_sel2_less_v_v_v_v_vb
#define v_u32_u32_sel2_less_v_v_v_s_vb                        v_u32_u32_sel2_less_v_v_v_v_vb

// i16 - bf16
#define v_i16_bf16_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel2_less_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_bf16_sel2_less_v_s_v_v_b                         v_i16_bf16_sel2_less_v_v_v_v_b
#define v_i16_bf16_sel2_less_v_v_v_s_b                         v_i16_bf16_sel2_less_v_v_v_v_b
#define v_i16_bf16_sel2_less_v_v_v_v(a, b, c, d)               v_i16_bf16_sel2_less_v_v_v_v_b(a, b, c, d, (bfloat128_short128_pair_t){0}, 1, 0)
#define v_i16_bf16_sel2_less_v_s_v_v                           v_i16_bf16_sel2_less_v_v_v_v
#define v_i16_bf16_sel2_less_v_v_v_s                           v_i16_bf16_sel2_less_v_v_v_v

#define v_i16_bf16_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel2_less_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_bf16_sel2_less_v_s_v_v_vb                        v_i16_bf16_sel2_less_v_v_v_v_vb
#define v_i16_bf16_sel2_less_v_v_v_s_vb                        v_i16_bf16_sel2_less_v_v_v_v_vb

// i16 - i16
#define v_i16_i16_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel2_less_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_i16_sel2_less_v_s_v_v_b                         v_i16_i16_sel2_less_v_v_v_v_b
#define v_i16_i16_sel2_less_v_v_v_s_b                         v_i16_i16_sel2_less_v_v_v_v_b
#define v_i16_i16_sel2_less_v_v_v_v(a, b, c, d)               v_i16_i16_sel2_less_v_v_v_v_b(a, b, c, d, (short128_pair_t){0}, 1, 0)
#define v_i16_i16_sel2_less_v_s_v_v                           v_i16_i16_sel2_less_v_v_v_v
#define v_i16_i16_sel2_less_v_v_v_s                           v_i16_i16_sel2_less_v_v_v_v

#define v_i16_i16_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel2_less_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_i16_sel2_less_v_s_v_v_vb                        v_i16_i16_sel2_less_v_v_v_v_vb
#define v_i16_i16_sel2_less_v_v_v_s_vb                        v_i16_i16_sel2_less_v_v_v_v_vb

// i16 - u16
#define v_i16_u16_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel2_less_i16_b(a, b, c, d, 0, i, p, o)
#define v_i16_u16_sel2_less_v_s_v_v_b                         v_i16_u16_sel2_less_v_v_v_v_b
#define v_i16_u16_sel2_less_v_v_v_s_b                         v_i16_u16_sel2_less_v_v_v_v_b
#define v_i16_u16_sel2_less_v_v_v_v(a, b, c, d)               v_i16_u16_sel2_less_v_v_v_v_b(a, b, c, d, (ushort128_short128_pair_t){0}, 1, 0)
#define v_i16_u16_sel2_less_v_s_v_v                           v_i16_u16_sel2_less_v_v_v_v
#define v_i16_u16_sel2_less_v_v_v_s                           v_i16_u16_sel2_less_v_v_v_v

#define v_i16_u16_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel2_less_i16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_i16_u16_sel2_less_v_s_v_v_vb                        v_i16_u16_sel2_less_v_v_v_v_vb
#define v_i16_u16_sel2_less_v_v_v_s_vb                        v_i16_u16_sel2_less_v_v_v_v_vb

// u16 - bf16
#define v_u16_bf16_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_bf16_sel2_less_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_bf16_sel2_less_v_s_v_v_b                         v_u16_bf16_sel2_less_v_v_v_v_b
#define v_u16_bf16_sel2_less_v_v_v_s_b                         v_u16_bf16_sel2_less_v_v_v_v_b
#define v_u16_bf16_sel2_less_v_v_v_v(a, b, c, d)               v_u16_bf16_sel2_less_v_v_v_v_b(a, b, c, d, (bfloat128_ushort128_pair_t){0}, 1, 0)
#define v_u16_bf16_sel2_less_v_s_v_v                           v_u16_bf16_sel2_less_v_v_v_v
#define v_u16_bf16_sel2_less_v_v_v_s                           v_u16_bf16_sel2_less_v_v_v_v

#define v_u16_bf16_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_bf16_sel2_less_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_bf16_sel2_less_v_s_v_v_vb                        v_u16_bf16_sel2_less_v_v_v_v_vb
#define v_u16_bf16_sel2_less_v_v_v_s_vb                        v_u16_bf16_sel2_less_v_v_v_v_vb

// u16 - i16
#define v_u16_i16_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_i16_sel2_less_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_i16_sel2_less_v_s_v_v_b                         v_u16_i16_sel2_less_v_v_v_v_b
#define v_u16_i16_sel2_less_v_v_v_s_b                         v_u16_i16_sel2_less_v_v_v_v_b
#define v_u16_i16_sel2_less_v_v_v_v(a, b, c, d)               v_u16_i16_sel2_less_v_v_v_v_b(a, b, c, d, (short128_ushort128_pair_t){0}, 1, 0)
#define v_u16_i16_sel2_less_v_s_v_v                           v_u16_i16_sel2_less_v_v_v_v
#define v_u16_i16_sel2_less_v_v_v_s                           v_u16_i16_sel2_less_v_v_v_v

#define v_u16_i16_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i16_sel2_less_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_i16_sel2_less_v_s_v_v_vb                        v_u16_i16_sel2_less_v_v_v_v_vb
#define v_u16_i16_sel2_less_v_v_v_s_vb                        v_u16_i16_sel2_less_v_v_v_v_vb

// u16 - u16
#define v_u16_u16_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_u16_sel2_less_u16_b(a, b, c, d, 0, i, p, o)
#define v_u16_u16_sel2_less_v_s_v_v_b                         v_u16_u16_sel2_less_v_v_v_v_b
#define v_u16_u16_sel2_less_v_v_v_s_b                         v_u16_u16_sel2_less_v_v_v_v_b
#define v_u16_u16_sel2_less_v_v_v_v(a, b, c, d)               v_u16_u16_sel2_less_v_v_v_v_b(a, b, c, d, (ushort128_pair_t){0}, 1, 0)
#define v_u16_u16_sel2_less_v_s_v_v                           v_u16_u16_sel2_less_v_v_v_v
#define v_u16_u16_sel2_less_v_v_v_s                           v_u16_u16_sel2_less_v_v_v_v

#define v_u16_u16_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u16_sel2_less_u16_vb(a, b, c, d, 0, i, to_bool128(p), o)
#define v_u16_u16_sel2_less_v_s_v_v_vb                        v_u16_u16_sel2_less_v_v_v_v_vb
#define v_u16_u16_sel2_less_v_v_v_s_vb                        v_u16_u16_sel2_less_v_v_v_v_vb

// i8 - i8
#define v_i8_i8_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_i8_sel2_less_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel2_less_v_s_v_v_b                         v_i8_i8_sel2_less_v_v_v_v_b
#define v_i8_i8_sel2_less_v_v_v_s_b                         v_i8_i8_sel2_less_v_v_v_v_b
#define v_i8_i8_sel2_less_v_v_v_v(a, b, c, d)               v_i8_i8_sel2_less_v_v_v_v_b(a, b, c, d, (char256_pair_t){0}, 1, 0)
#define v_i8_i8_sel2_less_v_s_v_v                           v_i8_i8_sel2_less_v_v_v_v
#define v_i8_i8_sel2_less_v_v_v_s                           v_i8_i8_sel2_less_v_v_v_v

#define v_i8_i8_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i8_sel2_less_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_i8_sel2_less_v_s_v_v_vb                        v_i8_i8_sel2_less_v_v_v_v_vb
#define v_i8_i8_sel2_less_v_v_v_s_vb                        v_i8_i8_sel2_less_v_v_v_v_vb

// i8 - u8
#define v_i8_u8_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_u8_sel2_less_i8_b(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel2_less_v_s_v_v_b                         v_i8_u8_sel2_less_v_v_v_v_b
#define v_i8_u8_sel2_less_v_v_v_s_b                         v_i8_u8_sel2_less_v_v_v_v_b
#define v_i8_u8_sel2_less_v_v_v_v(a, b, c, d)               v_i8_u8_sel2_less_v_v_v_v_b(a, b, c, d, (uchar256_char256_pair_t){0}, 1, 0)
#define v_i8_u8_sel2_less_v_s_v_v                           v_i8_u8_sel2_less_v_v_v_v
#define v_i8_u8_sel2_less_v_v_v_s                           v_i8_u8_sel2_less_v_v_v_v

#define v_i8_u8_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u8_sel2_less_i8_vb(a, b, c, d, 0, i, p, o)
#define v_i8_u8_sel2_less_v_s_v_v_vb                        v_i8_u8_sel2_less_v_v_v_v_vb
#define v_i8_u8_sel2_less_v_v_v_s_vb                        v_i8_u8_sel2_less_v_v_v_v_vb

// u8 - i8
#define v_u8_i8_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_i8_sel2_less_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel2_less_v_s_v_v_b                         v_u8_i8_sel2_less_v_v_v_v_b
#define v_u8_i8_sel2_less_v_v_v_s_b                         v_u8_i8_sel2_less_v_v_v_v_b
#define v_u8_i8_sel2_less_v_v_v_v(a, b, c, d)               v_u8_i8_sel2_less_v_v_v_v_b(a, b, c, d, (char256_uchar256_pair_t){0}, 1, 0)
#define v_u8_i8_sel2_less_v_s_v_v                           v_u8_i8_sel2_less_v_v_v_v
#define v_u8_i8_sel2_less_v_v_v_s                           v_u8_i8_sel2_less_v_v_v_v

#define v_u8_i8_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_i8_sel2_less_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_i8_sel2_less_v_s_v_v_vb                        v_u8_i8_sel2_less_v_v_v_v_vb
#define v_u8_i8_sel2_less_v_v_v_s_vb                        v_u8_i8_sel2_less_v_v_v_v_vb

// u8 - u8
#define v_u8_u8_sel2_less_v_v_v_v_b(a, b, c, d, i, p, o)    v_u8_sel2_less_u8_b(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel2_less_v_s_v_v_b                         v_u8_u8_sel2_less_v_v_v_v_b
#define v_u8_u8_sel2_less_v_v_v_s_b                         v_u8_u8_sel2_less_v_v_v_v_b
#define v_u8_u8_sel2_less_v_v_v_v(a, b, c, d)               v_u8_u8_sel2_less_v_v_v_v_b(a, b, c, d, (uchar256_pair_t){0}, 1, 0)
#define v_u8_u8_sel2_less_v_s_v_v                           v_u8_u8_sel2_less_v_v_v_v
#define v_u8_u8_sel2_less_v_v_v_s                           v_u8_u8_sel2_less_v_v_v_v

#define v_u8_u8_sel2_less_v_v_v_v_vb(a, b, c, d, i, p, o)   v_u8_sel2_less_u8_vb(a, b, c, d, 0, i, p, o)
#define v_u8_u8_sel2_less_v_s_v_v_vb                        v_u8_u8_sel2_less_v_v_v_v_vb
#define v_u8_u8_sel2_less_v_v_v_s_vb                        v_u8_u8_sel2_less_v_v_v_v_vb

// SUB

#define s_f32_sub_s_s_b(a, b, i, s, p, o)           s_f32_sub(a, b, s << 1, i, p, o)
#define s_bf16_sub_s_s_b(a, b, i, s, p, o)          s_bf16_sub(a, b, s << 1, i, p, o)

#define s_i32_sub_s_s_b(a, b, i, s, p, o)           s_i32_sub(a, b, s, i, p, o)
#define s_u32_sub_s_s_b(a, b, i, s, p, o)           s_u32_sub(a, b, s, i, p, o)
#define s_i16_sub_s_s_b(a, b, i, s, p, o)           s_i16_sub(a, b, s, i, p, o)
#define s_u16_sub_s_s_b(a, b, i, s, p, o)           s_u16_sub(a, b, s, i, p, o)
#define s_i8_sub_s_s_b(a, b, i, s, p, o)            s_i8_sub(a, b, s, i, p, o)
#define s_u8_sub_s_s_b(a, b, i, s, p, o)            s_u8_sub(a, b, s, i, p, o)

#define s_f32_sub_s_s(a, b, s)                      s_f32_sub_s_s_b(a, b, 0, s, 1, 0)
#define s_bf16_sub_s_s(a, b, s)                     s_bf16_sub_s_s_b(a, b, 0, s, 1, 0)
#define s_i32_sub_s_s(a, b, s)                      s_i32_sub_s_s_b(a, b, 0, s, 1, 0)
#define s_u32_sub_s_s(a, b, s)                      s_u32_sub_s_s_b(a, b, 0, s, 1, 0)
#define s_i16_sub_s_s(a, b, s)                      s_i16_sub_s_s_b(a, b, 0, s, 1, 0)
#define s_u16_sub_s_s(a, b, s)                      s_u16_sub_s_s_b(a, b, 0, s, 1, 0)
#define s_i8_sub_s_s(a, b, s)                       s_i8_sub_s_s_b(a, b, 0, s, 1, 0)
#define s_u8_sub_s_s(a, b, s)                       s_u8_sub_s_s_b(a, b, 0, s, 1, 0)

#define i_i32_sub_i_i_b(a, b, i, m, p, o)           i_i32_sub(a, b, m, 0, i, p, o)
#define i_i32_sub_s_i_b(a, b, i, m, p, o)           i_i32_sub_i_i_b(a, b, i, m, p, o)
#define i_i32_sub_i_i(a, b, i, m)                   i_i32_sub_i_i_b(a, b, i, m, 1, 0)
#define i_i32_sub_s_i(a, b, i, m)                   i_i32_sub_i_i(a, b, i, m)

#define v_f32_sub_v_v_vb(a, b, i, s, p, o)          v_f32_sub_vb(a, b, s << 1, i, to_bool64(p), o)
#define v_bf16_sub_v_v_vb(a, b, i, s, p, o)         v_bf16_sub_vb(a, b, s << 1, i, to_bool128(p), o)
#define v_i32_sub_v_v_vb(a, b, i, s, p, o)          v_i32_sub_vb(a, b, s, i, to_bool64(p), o)
#define v_u32_sub_v_v_vb(a, b, i, s, p, o)          v_u32_sub_vb(a, b, s, i, to_bool64(p), o)
#define v_i16_sub_v_v_vb(a, b, i, s, p, o)          v_i16_sub_vb(a, b, s, i, to_bool128(p), o)
#define v_u16_sub_v_v_vb(a, b, i, s, p, o)          v_u16_sub_vb(a, b, s, i, to_bool128(p), o)
#define v_i8_sub_v_v_vb(a, b, i, s, p, o)           v_i8_sub_vb(a, b, s, i, p, o)
#define v_u8_sub_v_v_vb(a, b, i, s, p, o)           v_u8_sub_vb(a, b, s, i, p, o)

#define v_f32_sub_v_v_b(a, b, i, s, p, o)           v_f32_sub_b(a, b, s << 1, i, p, o)
#define v_bf16_sub_v_v_b(a, b, i, s, p, o)          v_bf16_sub_b(a, b, s << 1, i, p, o)
#define v_i32_sub_v_v_b(a, b, i, s, p, o)           v_i32_sub_b(a, b, s, i, p, o)
#define v_u32_sub_v_v_b(a, b, i, s, p, o)           v_u32_sub_b(a, b, s, i, p, o)
#define v_i16_sub_v_v_b(a, b, i, s, p, o)           v_i16_sub_b(a, b, s, i, p, o)
#define v_u16_sub_v_v_b(a, b, i, s, p, o)           v_u16_sub_b(a, b, s, i, p, o)
#define v_i8_sub_v_v_b(a, b, i, s, p, o)            v_i8_sub_b(a, b, s, i, p, o)
#define v_u8_sub_v_v_b(a, b, i, s, p, o)            v_u8_sub_b(a, b, s, i, p, o)

#define v_f32_sub_v_s_vb(a, b, i, s, p, o)          v_f32_sub_v_v_vb(a, b, i, s, p, o)
#define v_bf16_sub_v_s_vb(a, b, i, s, p, o)         v_bf16_sub_v_v_vb(a, b, i, s, p, o)
#define v_i32_sub_v_s_vb(a, b, i, s, p, o)          v_i32_sub_v_v_vb(a, b, i, s, p, o)
#define v_u32_sub_v_s_vb(a, b, i, s, p, o)          v_u32_sub_v_v_vb(a, b, i, s, p, o)
#define v_i16_sub_v_s_vb(a, b, i, s, p, o)          v_i16_sub_v_v_vb(a, b, i, s, p, o)
#define v_u16_sub_v_s_vb(a, b, i, s, p, o)          v_u16_sub_v_v_vb(a, b, i, s, p, o)
#define v_i8_sub_v_s_vb(a, b, i, s, p, o)           v_i8_sub_v_v_vb(a, b, i, s, p, o)
#define v_u8_sub_v_s_vb(a, b, i, s, p, o)           v_u8_sub_v_v_vb(a, b, i, s, p, o)

#define v_f32_sub_v_s_b(a, b, i, s, p, o)           v_f32_sub_v_v_b(a, b, i, s, p, o)
#define v_bf16_sub_v_s_b(a, b, i, s, p, o)          v_bf16_sub_v_v_b(a, b, i, s, p, o)
#define v_i32_sub_v_s_b(a, b, i, s, p, o)           v_i32_sub_v_v_b(a, b, i, s, p, o)
#define v_u32_sub_v_s_b(a, b, i, s, p, o)           v_u32_sub_v_v_b(a, b, i, s, p, o)
#define v_i16_sub_v_s_b(a, b, i, s, p, o)           v_i16_sub_v_v_b(a, b, i, s, p, o)
#define v_u16_sub_v_s_b(a, b, i, s, p, o)           v_u16_sub_v_v_b(a, b, i, s, p, o)
#define v_i8_sub_v_s_b(a, b, i, s, p, o)            v_i8_sub_v_v_b(a, b, i, s, p, o)
#define v_u8_sub_v_s_b(a, b, i, s, p, o)            v_u8_sub_v_v_b(a, b, i, s, p, o)

#define v_f32_sub_v_v(a, b, s)                      v_f32_sub_v_v_b(a, b, 0, s, 1, 0)
#define v_bf16_sub_v_v(a, b, s)                     v_bf16_sub_v_v_b(a, b, 0, s, 1, 0)
#define v_i32_sub_v_v(a, b, s)                      v_i32_sub_v_v_b(a, b, 0, s, 1, 0)
#define v_u32_sub_v_v(a, b, s)                      v_u32_sub_v_v_b(a, b, 0, s, 1, 0)
#define v_i16_sub_v_v(a, b, s)                      v_i16_sub_v_v_b(a, b, 0, s, 1, 0)
#define v_u16_sub_v_v(a, b, s)                      v_u16_sub_v_v_b(a, b, 0, s, 1, 0)
#define v_i8_sub_v_v(a, b, s)                       v_i8_sub_v_v_b(a, b, 0, s, 1, 0)
#define v_u8_sub_v_v(a, b, s)                       v_u8_sub_v_v_b(a, b, 0, s, 1, 0)

#define v_f32_sub_v_s(a, b, s)                      v_f32_sub_v_v(a, b, s)
#define v_bf16_sub_v_s(a, b, s)                     v_bf16_sub_v_v(a, b, s)
#define v_i32_sub_v_s(a, b, s)                      v_i32_sub_v_v(a, b, s)
#define v_u32_sub_v_s(a, b, s)                      v_u32_sub_v_v(a, b, s)
#define v_i16_sub_v_s(a, b, s)                      v_i16_sub_v_v(a, b, s)
#define v_u16_sub_v_s(a, b, s)                      v_u16_sub_v_v(a, b, s)
#define v_i8_sub_v_s(a, b, s)                       v_i8_sub_v_v(a, b, s)
#define v_u8_sub_v_s(a, b, s)                       v_u8_sub_v_v(a, b, s)


// MAX

#define s_f32_max_s_s_b(a, b, i, p, o)              s_f32_max(a, b, 0, i, p, o)
#define s_bf16_max_s_s_b(a, b, i, p, o)             s_bf16_max(a, b, 0, i, p, o)
#define s_i32_max_s_s_b(a, b, i, p, o)              s_i32_max(a, b, 0, i, p, o)
#define s_u32_max_s_s_b(a, b, i, p, o)              s_u32_max(a, b, 0, i, p, o)
#define s_i16_max_s_s_b(a, b, i, p, o)              s_i16_max(a, b, 0, i, p, o)
#define s_u16_max_s_s_b(a, b, i, p, o)              s_u16_max(a, b, 0, i, p, o)
#define s_i8_max_s_s_b(a, b, i, p, o)               s_i8_max(a, b, 0, i, p, o)
#define s_u8_max_s_s_b(a, b, i, p, o)               s_u8_max(a, b, 0, i, p, o)

#define s_f32_max_s_s(a, b)                         s_f32_max_s_s_b(a, b, 0, 1, 0)
#define s_bf16_max_s_s(a, b)                        s_bf16_max_s_s_b(a, b, 0, 1, 0)
#define s_i32_max_s_s(a, b)                         s_i32_max_s_s_b(a, b, 0, 1, 0)
#define s_u32_max_s_s(a, b)                         s_u32_max_s_s_b(a, b, 0, 1, 0)
#define s_i16_max_s_s(a, b)                         s_i16_max_s_s_b(a, b, 0, 1, 0)
#define s_u16_max_s_s(a, b)                         s_u16_max_s_s_b(a, b, 0, 1, 0)
#define s_i8_max_s_s(a, b)                          s_i8_max_s_s_b(a, b, 0, 1, 0)
#define s_u8_max_s_s(a, b)                          s_u8_max_s_s_b(a, b, 0, 1, 0)

#define i_i32_max_i_i_b(a, b, i, m, p, o)           i_i32_max(a, b, m, 0,i, p, o)
#define i_i32_max_s_i_b(a, b, i, m, p, o)           i_i32_max_i_i_b(a, b, i, m, p, o)
#define i_i32_max_i_i(a, b, i, m)                   i_i32_max_i_i_b(a, b, i, m, 1, 0)
#define i_i32_max_s_i(a, b, i, m)                   i_i32_max_i_i(a, b, i, m)

#define v_f32_max_v_v_vb(a, b, i, p, o)             v_f32_max_vb(a, b, 0, i, to_bool64(p), o)
#define v_bf16_max_v_v_vb(a, b, i, p, o)            v_bf16_max_vb(a, b, 0, i, to_bool128(p), o)
#define v_i32_max_v_v_vb(a, b, i, p, o)             v_i32_max_vb(a, b, 0, i, to_bool64(p), o)
#define v_u32_max_v_v_vb(a, b, i, p, o)             v_u32_max_vb(a, b, 0, i, to_bool64(p), o)
#define v_i16_max_v_v_vb(a, b, i, p, o)             v_i16_max_vb(a, b, 0, i, to_bool128(p), o)
#define v_u16_max_v_v_vb(a, b, i, p, o)             v_u16_max_vb(a, b, 0, i, to_bool128(p), o)
#define v_i8_max_v_v_vb(a, b, i, p, o)              v_i8_max_vb(a, b, 0, i, p, o)
#define v_u8_max_v_v_vb(a, b, i, p, o)              v_u8_max_vb(a, b, 0, i, p, o)

#define v_f32_max_v_v_b(a, b, i, p, o)              v_f32_max_b(a, b, 0, i, p, o)
#define v_bf16_max_v_v_b(a, b, i, p, o)             v_bf16_max_b(a, b, 0, i, p, o)
#define v_i32_max_v_v_b(a, b, i, p, o)              v_i32_max_b(a, b, 0, i, p, o)
#define v_u32_max_v_v_b(a, b, i, p, o)              v_u32_max_b(a, b, 0, i, p, o)
#define v_i16_max_v_v_b(a, b, i, p, o)              v_i16_max_b(a, b, 0, i, p, o)
#define v_u16_max_v_v_b(a, b, i, p, o)              v_u16_max_b(a, b, 0, i, p, o)
#define v_i8_max_v_v_b(a, b, i, p, o)               v_i8_max_b(a, b, 0, i, p, o)
#define v_u8_max_v_v_b(a, b, i, p, o)               v_u8_max_b(a, b, 0, i, p, o)

#define v_f32_max_v_s_vb(a, b, i, p, o)             v_f32_max_v_v_vb(a, b, i, p, o)
#define v_bf16_max_v_s_vb(a, b, i, p, o)            v_bf16_max_v_v_vb(a, b, i, p, o)
#define v_i32_max_v_s_vb(a, b, i, p, o)             v_i32_max_v_v_vb(a, b, i, p, o)
#define v_u32_max_v_s_vb(a, b, i, p, o)             v_u32_max_v_v_vb(a, b, i, p, o)
#define v_i16_max_v_s_vb(a, b, i, p, o)             v_i16_max_v_v_vb(a, b, i, p, o)
#define v_u16_max_v_s_vb(a, b, i, p, o)             v_u16_max_v_v_vb(a, b, i, p, o)
#define v_i8_max_v_s_vb(a, b, i, p, o)              v_i8_max_v_v_vb(a, b, i, p, o)
#define v_u8_max_v_s_vb(a, b, i, p, o)              v_u8_max_v_v_vb(a, b, i, p, o)

#define v_f32_max_v_s_b(a, b, i, p, o)              v_f32_max_v_v_b(a, b, i, p, o)
#define v_bf16_max_v_s_b(a, b, i, p, o)             v_bf16_max_v_v_b(a, b, i, p, o)
#define v_i32_max_v_s_b(a, b, i, p, o)              v_i32_max_v_v_b(a, b, i, p, o)
#define v_u32_max_v_s_b(a, b, i, p, o)              v_u32_max_v_v_b(a, b, i, p, o)
#define v_i16_max_v_s_b(a, b, i, p, o)              v_i16_max_v_v_b(a, b, i, p, o)
#define v_u16_max_v_s_b(a, b, i, p, o)              v_u16_max_v_v_b(a, b, i, p, o)
#define v_i8_max_v_s_b(a, b, i, p, o)               v_i8_max_v_v_b(a, b, i, p, o)
#define v_u8_max_v_s_b(a, b, i, p, o)               v_u8_max_v_v_b(a, b, i, p, o)

#define v_f32_max_v_v(a, b)                         v_f32_max_v_v_b(a, b, 0, 1, 0)
#define v_bf16_max_v_v(a, b)                        v_bf16_max_v_v_b(a, b, 0, 1, 0)
#define v_i32_max_v_v(a, b)                         v_i32_max_v_v_b(a, b, 0, 1, 0)
#define v_u32_max_v_v(a, b)                         v_u32_max_v_v_b(a, b, 0, 1, 0)
#define v_i16_max_v_v(a, b)                         v_i16_max_v_v_b(a, b, 0, 1, 0)
#define v_u16_max_v_v(a, b)                         v_u16_max_v_v_b(a, b, 0, 1, 0)
#define v_i8_max_v_v(a, b)                          v_i8_max_v_v_b(a, b, 0, 1, 0)
#define v_u8_max_v_v(a, b)                          v_u8_max_v_v_b(a, b, 0, 1, 0)

#define v_f32_max_v_s(a, b)                         v_f32_max_v_v(a, b)
#define v_bf16_max_v_s(a, b)                        v_bf16_max_v_v(a, b)
#define v_i32_max_v_s(a, b)                         v_i32_max_v_v(a, b)
#define v_u32_max_v_s(a, b)                         v_u32_max_v_v(a, b)
#define v_i16_max_v_s(a, b)                         v_i16_max_v_v(a, b)
#define v_u16_max_v_s(a, b)                         v_u16_max_v_v(a, b)
#define v_i8_max_v_s(a, b)                          v_i8_max_v_v(a, b)
#define v_u8_max_v_s(a, b)                          v_u8_max_v_v(a, b)


// MIN

#define s_f32_min_s_s_b(a, b, i, p, o)              s_f32_min(a, b, 0, i, p, o)
#define s_bf16_min_s_s_b(a, b, i, p, o)             s_bf16_min(a, b, 0, i, p, o)
#define s_i32_min_s_s_b(a, b, i, p, o)              s_i32_min(a, b, 0, i, p, o)
#define s_u32_min_s_s_b(a, b, i, p, o)              s_u32_min(a, b, 0, i, p, o)
#define s_i16_min_s_s_b(a, b, i, p, o)              s_i16_min(a, b, 0, i, p, o)
#define s_u16_min_s_s_b(a, b, i, p, o)              s_u16_min(a, b, 0, i, p, o)
#define s_i8_min_s_s_b(a, b, i, p, o)               s_i8_min(a, b, 0, i, p, o)
#define s_u8_min_s_s_b(a, b, i, p, o)               s_u8_min(a, b, 0, i, p, o)

#define s_f32_min_s_s(a, b)                         s_f32_min_s_s_b(a, b, 0, 1, 0)
#define s_bf16_min_s_s(a, b)                        s_bf16_min_s_s_b(a, b, 0, 1, 0)
#define s_i32_min_s_s(a, b)                         s_i32_min_s_s_b(a, b, 0, 1, 0)
#define s_u32_min_s_s(a, b)                         s_u32_min_s_s_b(a, b, 0, 1, 0)
#define s_i16_min_s_s(a, b)                         s_i16_min_s_s_b(a, b, 0, 1, 0)
#define s_u16_min_s_s(a, b)                         s_u16_min_s_s_b(a, b, 0, 1, 0)
#define s_i8_min_s_s(a, b)                          s_i8_min_s_s_b(a, b, 0, 1, 0)
#define s_u8_min_s_s(a, b)                          s_u8_min_s_s_b(a, b, 0, 1, 0)

#define i_i32_min_i_i_b(a, b, i, m, p, o)           i_i32_min(a, b, m, 0, i, p, o)
#define i_i32_min_s_i_b(a, b, i, m, p, o)           i_i32_min_i_i_b(a, b, i, m, p, o)
#define i_i32_min_i_i(a, b, i, m)                   i_i32_min_i_i_b(a, b, i, m, 1, 0)
#define i_i32_min_s_i(a, b, i, m)                   i_i32_min_i_i(a, b, i, m)

#define v_f32_min_v_v_vb(a, b, i, p, o)             v_f32_min_vb(a, b, 0, i, to_bool64(p), o)
#define v_bf16_min_v_v_vb(a, b, i, p, o)            v_bf16_min_vb(a, b, 0, i, to_bool128(p), o)
#define v_i32_min_v_v_vb(a, b, i, p, o)             v_i32_min_vb(a, b, 0, i, to_bool64(p), o)
#define v_u32_min_v_v_vb(a, b, i, p, o)             v_u32_min_vb(a, b, 0, i, to_bool64(p), o)
#define v_i16_min_v_v_vb(a, b, i, p, o)             v_i16_min_vb(a, b, 0, i, to_bool128(p), o)
#define v_u16_min_v_v_vb(a, b, i, p, o)             v_u16_min_vb(a, b, 0, i, to_bool128(p), o)
#define v_i8_min_v_v_vb(a, b, i, p, o)              v_i8_min_vb(a, b, 0, i, p, o)
#define v_u8_min_v_v_vb(a, b, i, p, o)              v_u8_min_vb(a, b, 0, i, p, o)

#define v_f32_min_v_v_b(a, b, i, p, o)              v_f32_min_b(a, b, 0, i, p, o)
#define v_bf16_min_v_v_b(a, b, i, p, o)             v_bf16_min_b(a, b, 0, i, p, o)
#define v_i32_min_v_v_b(a, b, i, p, o)              v_i32_min_b(a, b, 0, i, p, o)
#define v_u32_min_v_v_b(a, b, i, p, o)              v_u32_min_b(a, b, 0, i, p, o)
#define v_i16_min_v_v_b(a, b, i, p, o)              v_i16_min_b(a, b, 0, i, p, o)
#define v_u16_min_v_v_b(a, b, i, p, o)              v_u16_min_b(a, b, 0, i, p, o)
#define v_i8_min_v_v_b(a, b, i, p, o)               v_i8_min_b(a, b, 0, i, p, o)
#define v_u8_min_v_v_b(a, b, i, p, o)               v_u8_min_b(a, b, 0, i, p, o)

#define v_f32_min_v_s_vb(a, b, i, p, o)             v_f32_min_v_v_vb(a, b, i, p, o)
#define v_bf16_min_v_s_vb(a, b, i, p, o)            v_bf16_min_v_v_vb(a, b, i, p, o)
#define v_i32_min_v_s_vb(a, b, i, p, o)             v_i32_min_v_v_vb(a, b, i, p, o)
#define v_u32_min_v_s_vb(a, b, i, p, o)             v_u32_min_v_v_vb(a, b, i, p, o)
#define v_i16_min_v_s_vb(a, b, i, p, o)             v_i16_min_v_v_vb(a, b, i, p, o)
#define v_u16_min_v_s_vb(a, b, i, p, o)             v_u16_min_v_v_vb(a, b, i, p, o)
#define v_i8_min_v_s_vb(a, b, i, p, o)              v_i8_min_v_v_vb(a, b, i, p, o)
#define v_u8_min_v_s_vb(a, b, i, p, o)              v_u8_min_v_v_vb(a, b, i, p, o)

#define v_f32_min_v_s_b(a, b, i, p, o)              v_f32_min_v_v_b(a, b, i, p, o)
#define v_bf16_min_v_s_b(a, b, i, p, o)             v_bf16_min_v_v_b(a, b, i, p, o)
#define v_i32_min_v_s_b(a, b, i, p, o)              v_i32_min_v_v_b(a, b, i, p, o)
#define v_u32_min_v_s_b(a, b, i, p, o)              v_u32_min_v_v_b(a, b, i, p, o)
#define v_i16_min_v_s_b(a, b, i, p, o)              v_i16_min_v_v_b(a, b, i, p, o)
#define v_u16_min_v_s_b(a, b, i, p, o)              v_u16_min_v_v_b(a, b, i, p, o)
#define v_i8_min_v_s_b(a, b, i, p, o)               v_i8_min_v_v_b(a, b, i, p, o)
#define v_u8_min_v_s_b(a, b, i, p, o)               v_u8_min_v_v_b(a, b, i, p, o)

#define v_f32_min_v_v(a, b)                         v_f32_min_v_v_b(a, b, 0, 1, 0)
#define v_bf16_min_v_v(a, b)                        v_bf16_min_v_v_b(a, b, 0, 1, 0)
#define v_i32_min_v_v(a, b)                         v_i32_min_v_v_b(a, b, 0, 1, 0)
#define v_u32_min_v_v(a, b)                         v_u32_min_v_v_b(a, b, 0, 1, 0)
#define v_i16_min_v_v(a, b)                         v_i16_min_v_v_b(a, b, 0, 1, 0)
#define v_u16_min_v_v(a, b)                         v_u16_min_v_v_b(a, b, 0, 1, 0)
#define v_i8_min_v_v(a, b)                          v_i8_min_v_v_b(a, b, 0, 1, 0)
#define v_u8_min_v_v(a, b)                          v_u8_min_v_v_b(a, b, 0, 1, 0)

#define v_f32_min_v_s(a, b)                         v_f32_min_v_v(a, b)
#define v_bf16_min_v_s(a, b)                        v_bf16_min_v_v(a, b)
#define v_i32_min_v_s(a, b)                         v_i32_min_v_v(a, b)
#define v_u32_min_v_s(a, b)                         v_u32_min_v_v(a, b)
#define v_i16_min_v_s(a, b)                         v_i16_min_v_v(a, b)
#define v_u16_min_v_s(a, b)                         v_u16_min_v_v(a, b)
#define v_i8_min_v_s(a, b)                          v_i8_min_v_v(a, b)
#define v_u8_min_v_s(a, b)                          v_u8_min_v_v(a, b)

// AND

#define s_f32_and_s_s_b(a, b, i, p, o)              s_f32_and(a, b, 0, i, p, o)
#define s_bf16_and_s_s_b(a, b, i, p, o)             s_bf16_and(a, b, 0, i, p, o)
#define s_i32_and_s_s_b(a, b, i, p, o)              s_i32_and(a, b, 0, i, p, o)
#define s_u32_and_s_s_b(a, b, i, p, o)              s_u32_and(a, b, 0, i, p, o)
#define s_i16_and_s_s_b(a, b, i, p, o)              s_i16_and(a, b, 0, i, p, o)
#define s_u16_and_s_s_b(a, b, i, p, o)              s_u16_and(a, b, 0, i, p, o)
#define s_i8_and_s_s_b(a, b, i, p, o)               s_i8_and(a, b, 0, i, p, o)
#define s_u8_and_s_s_b(a, b, i, p, o)               s_u8_and(a, b, 0, i, p, o)
#define b_b_and_b_b_b(a, b, i, p, o)                s_i1_and(a, b, 0, i, p, o)


#define s_f32_and_s_s(a, b)                         s_f32_and_s_s_b(a, b, 0, 1, 0)
#define s_bf16_and_s_s(a, b)                        s_bf16_and_s_s_b(a, b, 0, 1, 0)
#define s_i32_and_s_s(a, b)                         s_i32_and_s_s_b(a, b, 0, 1, 0)
#define s_u32_and_s_s(a, b)                         s_u32_and_s_s_b(a, b, 0, 1, 0)
#define s_i16_and_s_s(a, b)                         s_i16_and_s_s_b(a, b, 0, 1, 0)
#define s_u16_and_s_s(a, b)                         s_u16_and_s_s_b(a, b, 0, 1, 0)
#define s_i8_and_s_s(a, b)                          s_i8_and_s_s_b(a, b, 0, 1, 0)
#define s_u8_and_s_s(a, b)                          s_u8_and_s_s_b(a, b, 0, 1, 0)
#define b_b_and_b_b(a, b)                           b_b_and_b_b_b(a, b, 0, 1, 0)

#define i_i32_and_i_i_b(a, b, i, m, p, o)           i_i32_and(a, b, m, 0, i, p, o)
#define i_i32_and_s_i_b(a, b, i, m, p, o)           i_i32_and_i_i_b(a, b, i, m, p, o)
#define i_i32_and_i_i(a, b, i, m)                   i_i32_and_i_i_b(a, b, i, m, 1, 0)
#define i_i32_and_s_i(a, b, i, m)                   i_i32_and_i_i(a, b, i, m)

#define v_f32_and_v_v_vb(a, b, i, p, o)             v_f32_and_vb(a, b, 0, i, to_bool64(p), o)
#define v_bf16_and_v_v_vb(a, b, i, p, o)            v_bf16_and_vb(a, b, 0, i, to_bool128(p), o)
#define v_i32_and_v_v_vb(a, b, i, p, o)             v_i32_and_vb(a, b, 0, i, to_bool64(p), o)
#define v_u32_and_v_v_vb(a, b, i, p, o)             v_u32_and_vb(a, b, 0, i, to_bool64(p), o)
#define v_i16_and_v_v_vb(a, b, i, p, o)             v_i16_and_vb(a, b, 0, i, to_bool128(p), o)
#define v_u16_and_v_v_vb(a, b, i, p, o)             v_u16_and_vb(a, b, 0, i, to_bool128(p), o)
#define v_i8_and_v_v_vb(a, b, i, p, o)              v_i8_and_vb(a, b, 0, i, p, o)
#define v_u8_and_v_v_vb(a, b, i, p, o)              v_u8_and_vb(a, b, 0, i, p, o)
#define bv_b_and_bv_bv_vb(a, b, i, p, o)            v_i1_and_vb(a, b, 0, i, p, o)

#define v_f32_and_v_v_b(a, b, i, p, o)              v_f32_and_b(a, b, 0, i, p, o)
#define v_bf16_and_v_v_b(a, b, i, p, o)             v_bf16_and_b(a, b, 0, i, p, o)
#define v_i32_and_v_v_b(a, b, i, p, o)              v_i32_and_b(a, b, 0, i, p, o)
#define v_u32_and_v_v_b(a, b, i, p, o)              v_u32_and_b(a, b, 0, i, p, o)
#define v_i16_and_v_v_b(a, b, i, p, o)              v_i16_and_b(a, b, 0, i, p, o)
#define v_u16_and_v_v_b(a, b, i, p, o)              v_u16_and_b(a, b, 0, i, p, o)
#define v_i8_and_v_v_b(a, b, i, p, o)               v_i8_and_b(a, b, 0, i, p, o)
#define v_u8_and_v_v_b(a, b, i, p, o)               v_u8_and_b(a, b, 0, i, p, o)
#define bv_b_and_bv_bv_b(a, b, i, p, o)             v_i1_and_b(a, b, 0, i, p, o)

#define v_f32_and_v_s_vb(a, b, i, p, o)             v_f32_and_v_v_vb(a, b, i, p, o)
#define v_bf16_and_v_s_vb(a, b, i, p, o)            v_bf16_and_v_v_vb(a, b, i, p, o)
#define v_i32_and_v_s_vb(a, b, i, p, o)             v_i32_and_v_v_vb(a, b, i, p, o)
#define v_u32_and_v_s_vb(a, b, i, p, o)             v_u32_and_v_v_vb(a, b, i, p, o)
#define v_i16_and_v_s_vb(a, b, i, p, o)             v_i16_and_v_v_vb(a, b, i, p, o)
#define v_u16_and_v_s_vb(a, b, i, p, o)             v_u16_and_v_v_vb(a, b, i, p, o)
#define v_i8_and_v_s_vb(a, b, i, p, o)              v_i8_and_v_v_vb(a, b, i, p, o)
#define v_u8_and_v_s_vb(a, b, i, p, o)              v_u8_and_v_v_vb(a, b, i, p, o)

#define v_f32_and_v_s_b(a, b, i, p, o)              v_f32_and_v_v_b(a, b, i, p, o)
#define v_bf16_and_v_s_b(a, b, i, p, o)             v_bf16_and_v_v_b(a, b, i, p, o)
#define v_i32_and_v_s_b(a, b, i, p, o)              v_i32_and_v_v_b(a, b, i, p, o)
#define v_u32_and_v_s_b(a, b, i, p, o)              v_u32_and_v_v_b(a, b, i, p, o)
#define v_i16_and_v_s_b(a, b, i, p, o)              v_i16_and_v_v_b(a, b, i, p, o)
#define v_u16_and_v_s_b(a, b, i, p, o)              v_u16_and_v_v_b(a, b, i, p, o)
#define v_i8_and_v_s_b(a, b, i, p, o)               v_i8_and_v_v_b(a, b, i, p, o)
#define v_u8_and_v_s_b(a, b, i, p, o)               v_u8_and_v_v_b(a, b, i, p, o)

#define v_f32_and_v_v(a, b)                         v_f32_and_v_v_b(a, b, 0, 1, 0)
#define v_bf16_and_v_v(a, b)                        v_bf16_and_v_v_b(a, b, 0, 1, 0)
#define v_i32_and_v_v(a, b)                         v_i32_and_v_v_b(a, b, 0, 1, 0)
#define v_u32_and_v_v(a, b)                         v_u32_and_v_v_b(a, b, 0, 1, 0)
#define v_i16_and_v_v(a, b)                         v_i16_and_v_v_b(a, b, 0, 1, 0)
#define v_u16_and_v_v(a, b)                         v_u16_and_v_v_b(a, b, 0, 1, 0)
#define v_i8_and_v_v(a, b)                          v_i8_and_v_v_b(a, b, 0, 1, 0)
#define v_u8_and_v_v(a, b)                          v_u8_and_v_v_b(a, b, 0, 1, 0)
#define bv_b_and_bv_bv(a, b)                        bv_b_and_bv_bv_b(a, b, (bool256){0}, 1, 0)

#define v_f32_and_v_s(a, b)                         v_f32_and_v_v(a, b)
#define v_bf16_and_v_s(a, b)                        v_bf16_and_v_v(a, b)
#define v_i32_and_v_s(a, b)                         v_i32_and_v_v(a, b)
#define v_u32_and_v_s(a, b)                         v_u32_and_v_v(a, b)
#define v_i16_and_v_s(a, b)                         v_i16_and_v_v(a, b)
#define v_u16_and_v_s(a, b)                         v_u16_and_v_v(a, b)
#define v_i8_and_v_s(a, b)                          v_i8_and_v_v(a, b)
#define v_u8_and_v_s(a, b)                          v_u8_and_v_v(a, b)

// OR

#define s_f32_or_s_s_b(a, b, i, p, o)              s_f32_or(a, b, 0, i, p, o)
#define s_bf16_or_s_s_b(a, b, i, p, o)             s_bf16_or(a, b, 0, i, p, o)
#define s_i32_or_s_s_b(a, b, i, p, o)              s_i32_or(a, b, 0, i, p, o)
#define s_u32_or_s_s_b(a, b, i, p, o)              s_u32_or(a, b, 0, i, p, o)
#define s_i16_or_s_s_b(a, b, i, p, o)              s_i16_or(a, b, 0, i, p, o)
#define s_u16_or_s_s_b(a, b, i, p, o)              s_u16_or(a, b, 0, i, p, o)
#define s_i8_or_s_s_b(a, b, i, p, o)               s_i8_or(a, b, 0, i, p, o)
#define s_u8_or_s_s_b(a, b, i, p, o)               s_u8_or(a, b, 0, i, p, o)
#define b_b_or_b_b_b(a, b, i, p, o)                s_i1_or(a, b, 0, i, p, o)

#define s_f32_or_s_s(a, b)                         s_f32_or_s_s_b(a, b, 0, 1, 0)
#define s_bf16_or_s_s(a, b)                        s_bf16_or_s_s_b(a, b, 0, 1, 0)
#define s_i32_or_s_s(a, b)                         s_i32_or_s_s_b(a, b, 0, 1, 0)
#define s_u32_or_s_s(a, b)                         s_u32_or_s_s_b(a, b, 0, 1, 0)
#define s_i16_or_s_s(a, b)                         s_i16_or_s_s_b(a, b, 0, 1, 0)
#define s_u16_or_s_s(a, b)                         s_u16_or_s_s_b(a, b, 0, 1, 0)
#define s_i8_or_s_s(a, b)                          s_i8_or_s_s_b(a, b, 0, 1, 0)
#define s_u8_or_s_s(a, b)                          s_u8_or_s_s_b(a, b, 0, 1, 0)
#define b_b_or_b_b(a, b)                           b_b_or_b_b_b(a, b, 0, 1, 0)

#define i_i32_or_i_i_b(a, b, i, m, p, o)           i_i32_or(a, b, m, 0, i, p, o)
#define i_i32_or_s_i_b(a, b, i, m, p, o)           i_i32_or_i_i_b(a, b, i, m, p, o)
#define i_i32_or_i_i(a, b, i, m)                   i_i32_or_i_i_b(a, b, i, m, 1, 0)
#define i_i32_or_s_i(a, b, i, m)                   i_i32_or_i_i(a, b, i, m)

#define v_f32_or_v_v_vb(a, b, i, p, o)             v_f32_or_vb(a, b, 0, i, to_bool64(p), o)
#define v_bf16_or_v_v_vb(a, b, i, p, o)            v_bf16_or_vb(a, b, 0, i, to_bool128(p), o)
#define v_i32_or_v_v_vb(a, b, i, p, o)             v_i32_or_vb(a, b, 0, i, to_bool64(p), o)
#define v_u32_or_v_v_vb(a, b, i, p, o)             v_u32_or_vb(a, b, 0, i, to_bool64(p), o)
#define v_i16_or_v_v_vb(a, b, i, p, o)             v_i16_or_vb(a, b, 0, i, to_bool128(p), o)
#define v_u16_or_v_v_vb(a, b, i, p, o)             v_u16_or_vb(a, b, 0, i, to_bool128(p), o)
#define v_i8_or_v_v_vb(a, b, i, p, o)              v_i8_or_vb(a, b, 0, i, p, o)
#define v_u8_or_v_v_vb(a, b, i, p, o)              v_u8_or_vb(a, b, 0, i, p, o)
#define bv_b_or_bv_bv_vb(a, b, i, p, o)            v_i1_or_vb(a, b, 0, i, p, o)

#define v_f32_or_v_v_b(a, b, i, p, o)              v_f32_or_b(a, b, 0, i, p, o)
#define v_bf16_or_v_v_b(a, b, i, p, o)             v_bf16_or_b(a, b, 0, i, p, o)
#define v_i32_or_v_v_b(a, b, i, p, o)              v_i32_or_b(a, b, 0, i, p, o)
#define v_u32_or_v_v_b(a, b, i, p, o)              v_u32_or_b(a, b, 0, i, p, o)
#define v_i16_or_v_v_b(a, b, i, p, o)              v_i16_or_b(a, b, 0, i, p, o)
#define v_u16_or_v_v_b(a, b, i, p, o)              v_u16_or_b(a, b, 0, i, p, o)
#define v_i8_or_v_v_b(a, b, i, p, o)               v_i8_or_b(a, b, 0, i, p, o)
#define v_u8_or_v_v_b(a, b, i, p, o)               v_u8_or_b(a, b, 0, i, p, o)
#define bv_b_or_bv_bv_b(a, b, i, p, o)             v_i1_or_b(a, b, 0, i, p, o)

#define v_f32_or_v_s_vb(a, b, i, p, o)             v_f32_or_v_v_vb(a, b, i, p, o)
#define v_bf16_or_v_s_vb(a, b, i, p, o)            v_bf16_or_v_v_vb(a, b, i, p, o)
#define v_i32_or_v_s_vb(a, b, i, p, o)             v_i32_or_v_v_vb(a, b, i, p, o)
#define v_u32_or_v_s_vb(a, b, i, p, o)             v_u32_or_v_v_vb(a, b, i, p, o)
#define v_i16_or_v_s_vb(a, b, i, p, o)             v_i16_or_v_v_vb(a, b, i, p, o)
#define v_u16_or_v_s_vb(a, b, i, p, o)             v_u16_or_v_v_vb(a, b, i, p, o)
#define v_i8_or_v_s_vb(a, b, i, p, o)              v_i8_or_v_v_vb(a, b, i, p, o)
#define v_u8_or_v_s_vb(a, b, i, p, o)              v_u8_or_v_v_vb(a, b, i, p, o)

#define v_f32_or_v_s_b(a, b, i, p, o)              v_f32_or_v_v_b(a, b, i, p, o)
#define v_bf16_or_v_s_b(a, b, i, p, o)             v_bf16_or_v_v_b(a, b, i, p, o)
#define v_i32_or_v_s_b(a, b, i, p, o)              v_i32_or_v_v_b(a, b, i, p, o)
#define v_u32_or_v_s_b(a, b, i, p, o)              v_u32_or_v_v_b(a, b, i, p, o)
#define v_i16_or_v_s_b(a, b, i, p, o)              v_i16_or_v_v_b(a, b, i, p, o)
#define v_u16_or_v_s_b(a, b, i, p, o)              v_u16_or_v_v_b(a, b, i, p, o)
#define v_i8_or_v_s_b(a, b, i, p, o)               v_i8_or_v_v_b(a, b, i, p, o)
#define v_u8_or_v_s_b(a, b, i, p, o)               v_u8_or_v_v_b(a, b, i, p, o)

#define v_f32_or_v_v(a, b)                         v_f32_or_v_v_b(a, b, 0, 1, 0)
#define v_bf16_or_v_v(a, b)                        v_bf16_or_v_v_b(a, b, 0, 1, 0)
#define v_i32_or_v_v(a, b)                         v_i32_or_v_v_b(a, b, 0, 1, 0)
#define v_u32_or_v_v(a, b)                         v_u32_or_v_v_b(a, b, 0, 1, 0)
#define v_i16_or_v_v(a, b)                         v_i16_or_v_v_b(a, b, 0, 1, 0)
#define v_u16_or_v_v(a, b)                         v_u16_or_v_v_b(a, b, 0, 1, 0)
#define v_i8_or_v_v(a, b)                          v_i8_or_v_v_b(a, b, 0, 1, 0)
#define v_u8_or_v_v(a, b)                          v_u8_or_v_v_b(a, b, 0, 1, 0)
#define bv_b_or_bv_bv(a, b)                        bv_b_or_bv_bv_b(a, b, (bool256){0}, 1, 0)

#define v_f32_or_v_s(a, b)                         v_f32_or_v_v(a, b)
#define v_bf16_or_v_s(a, b)                        v_bf16_or_v_v(a, b)
#define v_i32_or_v_s(a, b)                         v_i32_or_v_v(a, b)
#define v_u32_or_v_s(a, b)                         v_u32_or_v_v(a, b)
#define v_i16_or_v_s(a, b)                         v_i16_or_v_v(a, b)
#define v_u16_or_v_s(a, b)                         v_u16_or_v_v(a, b)
#define v_i8_or_v_s(a, b)                          v_i8_or_v_v(a, b)
#define v_u8_or_v_s(a, b)                          v_u8_or_v_v(a, b)

// XOR

#define s_f32_xor_s_s_b(a, b, i, p, o)              s_f32_xor(a, b, 0, i, p, o)
#define s_bf16_xor_s_s_b(a, b, i, p, o)             s_bf16_xor(a, b, 0, i, p, o)
#define s_i32_xor_s_s_b(a, b, i, p, o)              s_i32_xor(a, b, 0, i, p, o)
#define s_u32_xor_s_s_b(a, b, i, p, o)              s_u32_xor(a, b, 0, i, p, o)
#define s_i16_xor_s_s_b(a, b, i, p, o)              s_i16_xor(a, b, 0, i, p, o)
#define s_u16_xor_s_s_b(a, b, i, p, o)              s_u16_xor(a, b, 0, i, p, o)
#define s_i8_xor_s_s_b(a, b, i, p, o)               s_i8_xor(a, b, 0, i, p, o)
#define s_u8_xor_s_s_b(a, b, i, p, o)               s_u8_xor(a, b, 0, i, p, o)
#define b_b_xor_b_b_b(a, b, i, p, o)                s_i1_xor(a, b, 0, i, p, o)

#define s_f32_xor_s_s(a, b)                         s_f32_xor_s_s_b(a, b, 0, 1, 0)
#define s_bf16_xor_s_s(a, b)                        s_bf16_xor_s_s_b(a, b, 0, 1, 0)
#define s_i32_xor_s_s(a, b)                         s_i32_xor_s_s_b(a, b, 0, 1, 0)
#define s_u32_xor_s_s(a, b)                         s_u32_xor_s_s_b(a, b, 0, 1, 0)
#define s_i16_xor_s_s(a, b)                         s_i16_xor_s_s_b(a, b, 0, 1, 0)
#define s_u16_xor_s_s(a, b)                         s_u16_xor_s_s_b(a, b, 0, 1, 0)
#define s_i8_xor_s_s(a, b)                          s_i8_xor_s_s_b(a, b, 0, 1, 0)
#define s_u8_xor_s_s(a, b)                          s_u8_xor_s_s_b(a, b, 0, 1, 0)
#define b_b_xor_b_b(a, b)                           b_b_xor_b_b_b(a, b, 0, 1, 0)

#define i_i32_xor_i_i_b(a, b, i, m, p, o)           i_i32_xor(a, b, m, 0, i, p, o)
#define i_i32_xor_s_i_b(a, b, i, m, p, o)           i_i32_xor_i_i_b(a, b, i, m, p, o)
#define i_i32_xor_i_i(a, b, i, m)                   i_i32_xor_i_i_b(a, b, i, m, 1, 0)
#define i_i32_xor_s_i(a, b, i, m)                   i_i32_xor_i_i(a, b, i, m)

#define v_f32_xor_v_v_vb(a, b, i, p, o)             v_f32_xor_vb(a, b, 0, i, to_bool64(p), o)
#define v_bf16_xor_v_v_vb(a, b, i, p, o)            v_bf16_xor_vb(a, b, 0, i, to_bool128(p), o)
#define v_i32_xor_v_v_vb(a, b, i, p, o)             v_i32_xor_vb(a, b, 0, i, to_bool64(p), o)
#define v_u32_xor_v_v_vb(a, b, i, p, o)             v_u32_xor_vb(a, b, 0, i, to_bool64(p), o)
#define v_i16_xor_v_v_vb(a, b, i, p, o)             v_i16_xor_vb(a, b, 0, i, to_bool128(p), o)
#define v_u16_xor_v_v_vb(a, b, i, p, o)             v_u16_xor_vb(a, b, 0, i, to_bool128(p), o)
#define v_i8_xor_v_v_vb(a, b, i, p, o)              v_i8_xor_vb(a, b, 0, i, p, o)
#define v_u8_xor_v_v_vb(a, b, i, p, o)              v_u8_xor_vb(a, b, 0, i, p, o)
#define bv_b_xor_bv_bv_vb(a, b, i, p, o)            v_i1_xor_vb(a, b, 0, i, p, o)

#define v_f32_xor_v_v_b(a, b, i, p, o)              v_f32_xor_b(a, b, 0, i, p, o)
#define v_bf16_xor_v_v_b(a, b, i, p, o)             v_bf16_xor_b(a, b, 0, i, p, o)
#define v_i32_xor_v_v_b(a, b, i, p, o)              v_i32_xor_b(a, b, 0, i, p, o)
#define v_u32_xor_v_v_b(a, b, i, p, o)              v_u32_xor_b(a, b, 0, i, p, o)
#define v_i16_xor_v_v_b(a, b, i, p, o)              v_i16_xor_b(a, b, 0, i, p, o)
#define v_u16_xor_v_v_b(a, b, i, p, o)              v_u16_xor_b(a, b, 0, i, p, o)
#define v_i8_xor_v_v_b(a, b, i, p, o)               v_i8_xor_b(a, b, 0, i, p, o)
#define v_u8_xor_v_v_b(a, b, i, p, o)               v_u8_xor_b(a, b, 0, i, p, o)
#define bv_b_xor_bv_bv_b(a, b, i, p, o)             v_i1_xor_b(a, b, 0, i, p, o)

#define v_f32_xor_v_s_vb(a, b, i, p, o)             v_f32_xor_v_v_vb(a, b, i, p, o)
#define v_bf16_xor_v_s_vb(a, b, i, p, o)            v_bf16_xor_v_v_vb(a, b, i, p, o)
#define v_i32_xor_v_s_vb(a, b, i, p, o)             v_i32_xor_v_v_vb(a, b, i, p, o)
#define v_u32_xor_v_s_vb(a, b, i, p, o)             v_u32_xor_v_v_vb(a, b, i, p, o)
#define v_i16_xor_v_s_vb(a, b, i, p, o)             v_i16_xor_v_v_vb(a, b, i, p, o)
#define v_u16_xor_v_s_vb(a, b, i, p, o)             v_u16_xor_v_v_vb(a, b, i, p, o)
#define v_i8_xor_v_s_vb(a, b, i, p, o)              v_i8_xor_v_v_vb(a, b, i, p, o)
#define v_u8_xor_v_s_vb(a, b, i, p, o)              v_u8_xor_v_v_vb(a, b, i, p, o)

#define v_f32_xor_v_s_b(a, b, i, p, o)              v_f32_xor_v_v_b(a, b, i, p, o)
#define v_bf16_xor_v_s_b(a, b, i, p, o)             v_bf16_xor_v_v_b(a, b, i, p, o)
#define v_i32_xor_v_s_b(a, b, i, p, o)              v_i32_xor_v_v_b(a, b, i, p, o)
#define v_u32_xor_v_s_b(a, b, i, p, o)              v_u32_xor_v_v_b(a, b, i, p, o)
#define v_i16_xor_v_s_b(a, b, i, p, o)              v_i16_xor_v_v_b(a, b, i, p, o)
#define v_u16_xor_v_s_b(a, b, i, p, o)              v_u16_xor_v_v_b(a, b, i, p, o)
#define v_i8_xor_v_s_b(a, b, i, p, o)               v_i8_xor_v_v_b(a, b, i, p, o)
#define v_u8_xor_v_s_b(a, b, i, p, o)               v_u8_xor_v_v_b(a, b, i, p, o)

#define v_f32_xor_v_v(a, b)                         v_f32_xor_v_v_b(a, b, 0, 1, 0)
#define v_bf16_xor_v_v(a, b)                        v_bf16_xor_v_v_b(a, b, 0, 1, 0)
#define v_i32_xor_v_v(a, b)                         v_i32_xor_v_v_b(a, b, 0, 1, 0)
#define v_u32_xor_v_v(a, b)                         v_u32_xor_v_v_b(a, b, 0, 1, 0)
#define v_i16_xor_v_v(a, b)                         v_i16_xor_v_v_b(a, b, 0, 1, 0)
#define v_u16_xor_v_v(a, b)                         v_u16_xor_v_v_b(a, b, 0, 1, 0)
#define v_i8_xor_v_v(a, b)                          v_i8_xor_v_v_b(a, b, 0, 1, 0)
#define v_u8_xor_v_v(a, b)                          v_u8_xor_v_v_b(a, b, 0, 1, 0)
#define bv_b_xor_bv_bv(a, b)                        bv_b_xor_bv_bv_b(a, b, (bool256){0}, 1, 0)

#define v_f32_xor_v_s(a, b)                         v_f32_xor_v_v(a, b)
#define v_bf16_xor_v_s(a, b)                        v_bf16_xor_v_v(a, b)
#define v_i32_xor_v_s(a, b)                         v_i32_xor_v_v(a, b)
#define v_u32_xor_v_s(a, b)                         v_u32_xor_v_v(a, b)
#define v_i16_xor_v_s(a, b)                         v_i16_xor_v_v(a, b)
#define v_u16_xor_v_s(a, b)                         v_u16_xor_v_v(a, b)
#define v_i8_xor_v_s(a, b)                          v_i8_xor_v_v(a, b)
#define v_u8_xor_v_s(a, b)                          v_u8_xor_v_v(a, b)


// CMP_EQ

#define b_f32_cmp_eq_s_s_b(a, b, i, p, o)         s_f32_cmp_eq(a, b, 0, i, p, o)
#define b_bf16_cmp_eq_s_s_b(a, b, i, p, o)        s_bf16_cmp_eq(a, b, 0, i, p, o)
#define b_i32_cmp_eq_s_s_b(a, b, i, p, o)         s_i32_cmp_eq(a, b, 0, i, p, o)
#define b_u32_cmp_eq_s_s_b(a, b, i, p, o)         s_u32_cmp_eq(a, b, 0, i, p, o)
#define b_i16_cmp_eq_s_s_b(a, b, i, p, o)         s_i16_cmp_eq(a, b, 0, i, p, o)
#define b_u16_cmp_eq_s_s_b(a, b, i, p, o)         s_u16_cmp_eq(a, b, 0, i, p, o)
#define b_i8_cmp_eq_s_s_b(a, b, i, p, o)          s_i8_cmp_eq(a, b, 0, i, p, o)
#define b_u8_cmp_eq_s_s_b(a, b, i, p, o)          s_u8_cmp_eq(a, b, 0, i, p, o)

#define b_f32_cmp_eq_s_s(a, b)                    b_f32_cmp_eq_s_s_b(a, b, 0, 1, 0)
#define b_bf16_cmp_eq_s_s(a, b)                   b_bf16_cmp_eq_s_s_b(a, b, 0, 1, 0)
#define b_i32_cmp_eq_s_s(a, b)                    b_i32_cmp_eq_s_s_b(a, b, 0, 1, 0)
#define b_u32_cmp_eq_s_s(a, b)                    b_u32_cmp_eq_s_s_b(a, b, 0, 1, 0)
#define b_i16_cmp_eq_s_s(a, b)                    b_i16_cmp_eq_s_s_b(a, b, 0, 1, 0)
#define b_u16_cmp_eq_s_s(a, b)                    b_u16_cmp_eq_s_s_b(a, b, 0, 1, 0)
#define b_i8_cmp_eq_s_s(a, b)                     b_i8_cmp_eq_s_s_b(a, b, 0, 1, 0)
#define b_u8_cmp_eq_s_s(a, b)                     b_u8_cmp_eq_s_s_b(a, b, 0, 1, 0)

#define bv_f32_cmp_eq_v_v_vb(a, b, i, p, o)       from_bool64(v_f32_cmp_eq_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_bf16_cmp_eq_v_v_vb(a, b, i, p, o)      from_bool128(v_bf16_cmp_eq_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_i32_cmp_eq_v_v_vb(a, b, i, p, o)       from_bool64(v_i32_cmp_eq_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_u32_cmp_eq_v_v_vb(a, b, i, p, o)       from_bool64(v_u32_cmp_eq_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_i16_cmp_eq_v_v_vb(a, b, i, p, o)       from_bool128(v_i16_cmp_eq_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_u16_cmp_eq_v_v_vb(a, b, i, p, o)       from_bool128(v_u16_cmp_eq_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_i8_cmp_eq_v_v_vb(a, b, i, p, o)        v_i8_cmp_eq_vb(a, b, 0, i, p, o)
#define bv_u8_cmp_eq_v_v_vb(a, b, i, p, o)        v_u8_cmp_eq_vb(a, b, 0, i, p, o)

#define bv_f32_cmp_eq_v_s_vb(a, b, i, p, o)       bv_f32_cmp_eq_v_v_vb(a, b, i, p, o)
#define bv_bf16_cmp_eq_v_s_vb(a, b, i, p, o)      bv_bf16_cmp_eq_v_v_vb(a, b, i, p, o)
#define bv_i32_cmp_eq_v_s_vb(a, b, i, p, o)       bv_i32_cmp_eq_v_v_vb(a, b, i, p, o)
#define bv_u32_cmp_eq_v_s_vb(a, b, i, p, o)       bv_u32_cmp_eq_v_v_vb(a, b, i, p, o)
#define bv_i16_cmp_eq_v_s_vb(a, b, i, p, o)       bv_i16_cmp_eq_v_v_vb(a, b, i, p, o)
#define bv_u16_cmp_eq_v_s_vb(a, b, i, p, o)       bv_u16_cmp_eq_v_v_vb(a, b, i, p, o)
#define bv_i8_cmp_eq_v_s_vb(a, b, i, p, o)        bv_i8_cmp_eq_v_v_vb(a, b, i, p, o)
#define bv_u8_cmp_eq_v_s_vb(a, b, i, p, o)        bv_u8_cmp_eq_v_v_vb(a, b, i, p, o)

#define bv_f32_cmp_eq_v_v_b(a, b, i, p, o)        from_bool64(v_f32_cmp_eq_b(a, b, 0, to_bool64(i), p, o))
#define bv_bf16_cmp_eq_v_v_b(a, b, i, p, o)       from_bool128(v_bf16_cmp_eq_b(a, b, 0, to_bool128(i), p, o))
#define bv_i32_cmp_eq_v_v_b(a, b, i, p, o)        from_bool64(v_i32_cmp_eq_b(a, b, 0, to_bool64(i), p, o))
#define bv_u32_cmp_eq_v_v_b(a, b, i, p, o)        from_bool64(v_u32_cmp_eq_b(a, b, 0, to_bool64(i), p, o))
#define bv_i16_cmp_eq_v_v_b(a, b, i, p, o)        from_bool128(v_i16_cmp_eq_b(a, b, 0, to_bool128(i), p, o))
#define bv_u16_cmp_eq_v_v_b(a, b, i, p, o)        from_bool128(v_u16_cmp_eq_b(a, b, 0, to_bool128(i), p, o))
#define bv_i8_cmp_eq_v_v_b(a, b, i, p, o)         v_i8_cmp_eq_b(a, b, 0, i, p, o)
#define bv_u8_cmp_eq_v_v_b(a, b, i, p, o)         v_u8_cmp_eq_b(a, b, 0, i, p, o)

#define bv_f32_cmp_eq_v_s_b(a, b, i, p, o)        bv_f32_cmp_eq_v_v_b(a, b, i, p, o)
#define bv_bf16_cmp_eq_v_s_b(a, b, i, p, o)       bv_bf16_cmp_eq_v_v_b(a, b, i, p, o)
#define bv_i32_cmp_eq_v_s_b(a, b, i, p, o)        bv_i32_cmp_eq_v_v_b(a, b, i, p, o)
#define bv_u32_cmp_eq_v_s_b(a, b, i, p, o)        bv_u32_cmp_eq_v_v_b(a, b, i, p, o)
#define bv_i16_cmp_eq_v_s_b(a, b, i, p, o)        bv_i16_cmp_eq_v_v_b(a, b, i, p, o)
#define bv_u16_cmp_eq_v_s_b(a, b, i, p, o)        bv_u16_cmp_eq_v_v_b(a, b, i, p, o)
#define bv_i8_cmp_eq_v_s_b(a, b, i, p, o)         bv_i8_cmp_eq_v_v_b(a, b, i, p, o)
#define bv_u8_cmp_eq_v_s_b(a, b, i, p, o)         bv_u8_cmp_eq_v_v_b(a, b, i, p, o)

#define bv_f32_cmp_eq_v_v(a, b)                   bv_f32_cmp_eq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_bf16_cmp_eq_v_v(a, b)                  bv_bf16_cmp_eq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i32_cmp_eq_v_v(a, b)                   bv_i32_cmp_eq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u32_cmp_eq_v_v(a, b)                   bv_u32_cmp_eq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i16_cmp_eq_v_v(a, b)                   bv_i16_cmp_eq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u16_cmp_eq_v_v(a, b)                   bv_u16_cmp_eq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i8_cmp_eq_v_v(a, b)                    bv_i8_cmp_eq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u8_cmp_eq_v_v(a, b)                    bv_u8_cmp_eq_v_v_b(a, b, (bool256){0}, 1, 0)

#define bv_f32_cmp_eq_v_s(a, b)                   bv_f32_cmp_eq_v_v(a, b)
#define bv_i32_cmp_eq_v_s(a, b)                   bv_i32_cmp_eq_v_v(a, b)
#define bv_u32_cmp_eq_v_s(a, b)                   bv_u32_cmp_eq_v_v(a, b)
#define bv_i16_cmp_eq_v_s(a, b)                   bv_i16_cmp_eq_v_v(a, b)
#define bv_u16_cmp_eq_v_s(a, b)                   bv_u16_cmp_eq_v_v(a, b)
#define bv_i8_cmp_eq_v_s(a, b)                    bv_i8_cmp_eq_v_v(a, b)
#define bv_u8_cmp_eq_v_s(a, b)                    bv_u8_cmp_eq_v_v(a, b)
#define bv_bf16_cmp_eq_v_s(a, b)                  bv_bf16_cmp_eq_v_v(a, b)

#define b_f32_cmp_eq_zero_s_s_b(a, b, i, p, o)    s_f32_cmp_eq(a, b, SW_MASK_EQ_ZERO, i, p, o)
#define b_bf16_cmp_eq_zero_s_s_b(a, b, i, p, o)   s_bf16_cmp_eq(a, b, SW_MASK_EQ_ZERO, i, p, o)
#define b_i32_cmp_eq_zero_s_s_b(a, b, i, p, o)    s_i32_cmp_eq(a, b, SW_MASK_EQ_ZERO, i, p, o)
#define b_u32_cmp_eq_zero_s_s_b(a, b, i, p, o)    s_u32_cmp_eq(a, b, SW_MASK_EQ_ZERO, i, p, o)
#define b_i16_cmp_eq_zero_s_s_b(a, b, i, p, o)    s_i16_cmp_eq(a, b, SW_MASK_EQ_ZERO, i, p, o)
#define b_u16_cmp_eq_zero_s_s_b(a, b, i, p, o)    s_u16_cmp_eq(a, b, SW_MASK_EQ_ZERO, i, p, o)
#define b_i8_cmp_eq_zero_s_s_b(a, b, i, p, o)     s_i8_cmp_eq(a, b, SW_MASK_EQ_ZERO, i, p, o)
#define b_u8_cmp_eq_zero_s_s_b(a, b, i, p, o)     s_u8_cmp_eq(a, b, SW_MASK_EQ_ZERO, i, p, o)

#define b_f32_cmp_eq_zero_s_s(a, b)               b_f32_cmp_eq_zero_s_s_b(a, b, 0, 1, 0)
#define b_bf16_cmp_eq_zero_s_s(a, b)              b_bf16_cmp_eq_zero_s_s_b(a, b, 0, 1, 0)
#define b_i32_cmp_eq_zero_s_s(a, b)               b_i32_cmp_eq_zero_s_s_b(a, b, 0, 1, 0)
#define b_u32_cmp_eq_zero_s_s(a, b)               b_u32_cmp_eq_zero_s_s_b(a, b, 0, 1, 0)
#define b_i16_cmp_eq_zero_s_s(a, b)               b_i16_cmp_eq_zero_s_s_b(a, b, 0, 1, 0)
#define b_u16_cmp_eq_zero_s_s(a, b)               b_u16_cmp_eq_zero_s_s_b(a, b, 0, 1, 0)
#define b_i8_cmp_eq_zero_s_s(a, b)                b_i8_cmp_eq_zero_s_s_b(a, b, 0, 1, 0)
#define b_u8_cmp_eq_zero_s_s(a, b)                b_u8_cmp_eq_zero_s_s_b(a, b, 0, 1, 0)

#define bv_f32_cmp_eq_zero_v_v_vb(a, b, i, p, o)  from_bool64(v_f32_cmp_eq_vb(a, b, SW_MASK_EQ_ZERO, to_bool64(i), to_bool64(p), o))
#define bv_bf16_cmp_eq_zero_v_v_vb(a, b, i, p, o) from_bool128(v_bf16_cmp_eq_vb(a, b, SW_MASK_EQ_ZERO, to_bool128(i), to_bool128(p), o))
#define bv_i32_cmp_eq_zero_v_v_vb(a, b, i, p, o)  from_bool64(v_i32_cmp_eq_vb(a, b, SW_MASK_EQ_ZERO, to_bool64(i), to_bool64(p), o))
#define bv_u32_cmp_eq_zero_v_v_vb(a, b, i, p, o)  from_bool64(v_u32_cmp_eq_vb(a, b, SW_MASK_EQ_ZERO, to_bool64(i), to_bool64(p), o))
#define bv_i16_cmp_eq_zero_v_v_vb(a, b, i, p, o)  from_bool128(v_i16_cmp_eq_vb(a, b, SW_MASK_EQ_ZERO, to_bool128(i), to_bool128(p), o))
#define bv_u16_cmp_eq_zero_v_v_vb(a, b, i, p, o)  from_bool128(v_u16_cmp_eq_vb(a, b, SW_MASK_EQ_ZERO, to_bool128(i), to_bool128(p), o))
#define bv_i8_cmp_eq_zero_v_v_vb(a, b, i, p, o)   v_i8_cmp_eq_vb(a, b, SW_MASK_EQ_ZERO, i, p, o)
#define bv_u8_cmp_eq_zero_v_v_vb(a, b, i, p, o)   v_u8_cmp_eq_vb(a, b, SW_MASK_EQ_ZERO, i, p, o)

#define bv_f32_cmp_eq_zero_v_s_vb(a, b, i, p, o)  bv_f32_cmp_eq_zero_v_v_vb(a, b, i, p, o)
#define bv_bf16_cmp_eq_zero_v_s_vb(a, b, i, p, o) bv_bf16_cmp_eq_zero_v_v_vb(a, b, i, p, o)
#define bv_i32_cmp_eq_zero_v_s_vb(a, b, i, p, o)  bv_i32_cmp_eq_zero_v_v_vb(a, b, i, p, o)
#define bv_u32_cmp_eq_zero_v_s_vb(a, b, i, p, o)  bv_u32_cmp_eq_zero_v_v_vb(a, b, i, p, o)
#define bv_i16_cmp_eq_zero_v_s_vb(a, b, i, p, o)  bv_i16_cmp_eq_zero_v_v_vb(a, b, i, p, o)
#define bv_u16_cmp_eq_zero_v_s_vb(a, b, i, p, o)  bv_u16_cmp_eq_zero_v_v_vb(a, b, i, p, o)
#define bv_i8_cmp_eq_zero_v_s_vb(a, b, i, p, o)   bv_i8_cmp_eq_zero_v_v_vb(a, b, i, p, o)
#define bv_u8_cmp_eq_zero_v_s_vb(a, b, i, p, o)   bv_u8_cmp_eq_zero_v_v_vb(a, b, i, p, o)

#define bv_f32_cmp_eq_zero_v_v_b(a, b, i, p, o)   from_bool64(v_f32_cmp_eq_b(a, b, SW_MASK_EQ_ZERO, to_bool64(i), p, o))
#define bv_bf16_cmp_eq_zero_v_v_b(a, b, i, p, o)  from_bool128(v_bf16_cmp_eq_b(a, b, SW_MASK_EQ_ZERO, to_bool128(i), p, o))
#define bv_i32_cmp_eq_zero_v_v_b(a, b, i, p, o)   from_bool64(v_i32_cmp_eq_b(a, b, SW_MASK_EQ_ZERO, to_bool64(i), p, o))
#define bv_u32_cmp_eq_zero_v_v_b(a, b, i, p, o)   from_bool64(v_u32_cmp_eq_b(a, b, SW_MASK_EQ_ZERO, to_bool64(i), p, o))
#define bv_i16_cmp_eq_zero_v_v_b(a, b, i, p, o)   from_bool128(v_i16_cmp_eq_b(a, b, SW_MASK_EQ_ZERO, to_bool128(i), p, o))
#define bv_u16_cmp_eq_zero_v_v_b(a, b, i, p, o)   from_bool128(v_u16_cmp_eq_b(a, b, SW_MASK_EQ_ZERO, to_bool128(i), p, o))
#define bv_i8_cmp_eq_zero_v_v_b(a, b, i, p, o)    v_i8_cmp_eq_b(a, b, SW_MASK_EQ_ZERO, i, p, o)
#define bv_u8_cmp_eq_zero_v_v_b(a, b, i, p, o)    v_u8_cmp_eq_b(a, b, SW_MASK_EQ_ZERO, i, p, o)

#define bv_f32_cmp_eq_zero_v_s_b(a, b, i, p, o)   bv_f32_cmp_eq_zero_v_v_b(a, b, i, p, o)
#define bv_bf16_cmp_eq_zero_v_s_b(a, b, i, p, o)  bv_bf16_cmp_eq_zero_v_v_b(a, b, i, p, o)
#define bv_i32_cmp_eq_zero_v_s_b(a, b, i, p, o)   bv_i32_cmp_eq_zero_v_v_b(a, b, i, p, o)
#define bv_u32_cmp_eq_zero_v_s_b(a, b, i, p, o)   bv_u32_cmp_eq_zero_v_v_b(a, b, i, p, o)
#define bv_i16_cmp_eq_zero_v_s_b(a, b, i, p, o)   bv_i16_cmp_eq_zero_v_v_b(a, b, i, p, o)
#define bv_u16_cmp_eq_zero_v_s_b(a, b, i, p, o)   bv_u16_cmp_eq_zero_v_v_b(a, b, i, p, o)
#define bv_i8_cmp_eq_zero_v_s_b(a, b, i, p, o)    bv_i8_cmp_eq_zero_v_v_b(a, b, i, p, o)
#define bv_u8_cmp_eq_zero_v_s_b(a, b, i, p, o)    bv_u8_cmp_eq_zero_v_v_b(a, b, i, p, o)

#define bv_f32_cmp_eq_zero_v_v(a, b)              bv_f32_cmp_eq_zero_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_bf16_cmp_eq_zero_v_v(a, b)             bv_bf16_cmp_eq_zero_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i32_cmp_eq_zero_v_v(a, b)              bv_i32_cmp_eq_zero_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u32_cmp_eq_zero_v_v(a, b)              bv_u32_cmp_eq_zero_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i16_cmp_eq_zero_v_v(a, b)              bv_i16_cmp_eq_zero_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u16_cmp_eq_zero_v_v(a, b)              bv_u16_cmp_eq_zero_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i8_cmp_eq_zero_v_v(a, b)               bv_i8_cmp_eq_zero_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u8_cmp_eq_zero_v_v(a, b)               bv_u8_cmp_eq_zero_v_v_b(a, b, (bool256){0}, 1, 0)

#define bv_f32_cmp_eq_zero_v_s(a, b)              bv_f32_cmp_eq_zero_v_v(a, b)
#define bv_bf16_cmp_eq_zero_v_s(a, b)             bv_bf16_cmp_eq_zero_v_v(a, b)
#define bv_i32_cmp_eq_zero_v_s(a, b)              bv_i32_cmp_eq_zero_v_v(a, b)
#define bv_u32_cmp_eq_zero_v_s(a, b)              bv_u32_cmp_eq_zero_v_v(a, b)
#define bv_i16_cmp_eq_zero_v_s(a, b)              bv_i16_cmp_eq_zero_v_v(a, b)
#define bv_u16_cmp_eq_zero_v_s(a, b)              bv_u16_cmp_eq_zero_v_v(a, b)
#define bv_i8_cmp_eq_zero_v_s(a, b)               bv_i8_cmp_eq_zero_v_v(a, b)
#define bv_u8_cmp_eq_zero_v_s(a, b)               bv_u8_cmp_eq_zero_v_v(a, b)


// CMP_NEQ

#define b_f32_cmp_neq_s_s_b(a, b, i, p, o)         s_f32_cmp_neq(a, b, 0, i, p, o)
#define b_bf16_cmp_neq_s_s_b(a, b, i, p, o)        s_bf16_cmp_neq(a, b, 0, i, p, o)
#define b_i32_cmp_neq_s_s_b(a, b, i, p, o)         s_i32_cmp_neq(a, b, 0, i, p, o)
#define b_u32_cmp_neq_s_s_b(a, b, i, p, o)         s_u32_cmp_neq(a, b, 0, i, p, o)
#define b_i16_cmp_neq_s_s_b(a, b, i, p, o)         s_i16_cmp_neq(a, b, 0, i, p, o)
#define b_u16_cmp_neq_s_s_b(a, b, i, p, o)         s_u16_cmp_neq(a, b, 0, i, p, o)
#define b_i8_cmp_neq_s_s_b(a, b, i, p, o)          s_i8_cmp_neq(a, b, 0, i, p, o)
#define b_u8_cmp_neq_s_s_b(a, b, i, p, o)          s_u8_cmp_neq(a, b, 0, i, p, o)

#define b_f32_cmp_neq_s_s(a, b)                    b_f32_cmp_neq_s_s_b(a, b, 0, 1, 0)
#define b_bf16_cmp_neq_s_s(a, b)                   b_bf16_cmp_neq_s_s_b(a, b, 0, 1, 0)
#define b_i32_cmp_neq_s_s(a, b)                    b_i32_cmp_neq_s_s_b(a, b, 0, 1, 0)
#define b_u32_cmp_neq_s_s(a, b)                    b_u32_cmp_neq_s_s_b(a, b, 0, 1, 0)
#define b_i16_cmp_neq_s_s(a, b)                    b_i16_cmp_neq_s_s_b(a, b, 0, 1, 0)
#define b_u16_cmp_neq_s_s(a, b)                    b_u16_cmp_neq_s_s_b(a, b, 0, 1, 0)
#define b_i8_cmp_neq_s_s(a, b)                     b_i8_cmp_neq_s_s_b(a, b, 0, 1, 0)
#define b_u8_cmp_neq_s_s(a, b)                     b_u8_cmp_neq_s_s_b(a, b, 0, 1, 0)

#define bv_f32_cmp_neq_v_v_vb(a, b, i, p, o)       from_bool64(v_f32_cmp_neq_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_bf16_cmp_neq_v_v_vb(a, b, i, p, o)      from_bool128(v_bf16_cmp_neq_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_i32_cmp_neq_v_v_vb(a, b, i, p, o)       from_bool64(v_i32_cmp_neq_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_u32_cmp_neq_v_v_vb(a, b, i, p, o)       from_bool64(v_u32_cmp_neq_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_i16_cmp_neq_v_v_vb(a, b, i, p, o)       from_bool128(v_i16_cmp_neq_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_u16_cmp_neq_v_v_vb(a, b, i, p, o)       from_bool128(v_u16_cmp_neq_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_i8_cmp_neq_v_v_vb(a, b, i, p, o)        v_i8_cmp_neq_vb(a, b, 0, i, p, o)
#define bv_u8_cmp_neq_v_v_vb(a, b, i, p, o)        v_u8_cmp_neq_vb(a, b, 0, i, p, o)

#define bv_f32_cmp_neq_v_s_vb(a, b, i, p, o)       bv_f32_cmp_neq_v_v_vb(a, b, i, p, o)
#define bv_bf16_cmp_neq_v_s_vb(a, b, i, p, o)      bv_bf16_cmp_neq_v_v_vb(a, b, i, p, o)
#define bv_i32_cmp_neq_v_s_vb(a, b, i, p, o)       bv_i32_cmp_neq_v_v_vb(a, b, i, p, o)
#define bv_u32_cmp_neq_v_s_vb(a, b, i, p, o)       bv_u32_cmp_neq_v_v_vb(a, b, i, p, o)
#define bv_i16_cmp_neq_v_s_vb(a, b, i, p, o)       bv_i16_cmp_neq_v_v_vb(a, b, i, p, o)
#define bv_u16_cmp_neq_v_s_vb(a, b, i, p, o)       bv_u16_cmp_neq_v_v_vb(a, b, i, p, o)
#define bv_i8_cmp_neq_v_s_vb(a, b, i, p, o)        bv_i8_cmp_neq_v_v_vb(a, b, i, p, o)
#define bv_u8_cmp_neq_v_s_vb(a, b, i, p, o)        bv_u8_cmp_neq_v_v_vb(a, b, i, p, o)

#define bv_f32_cmp_neq_v_v_b(a, b, i, p, o)        from_bool64(v_f32_cmp_neq_b(a, b, 0, to_bool64(i), p, o))
#define bv_bf16_cmp_neq_v_v_b(a, b, i, p, o)       from_bool128(v_bf16_cmp_neq_b(a, b, 0, to_bool128(i), p, o))
#define bv_i32_cmp_neq_v_v_b(a, b, i, p, o)        from_bool64(v_i32_cmp_neq_b(a, b, 0, to_bool64(i), p, o))
#define bv_u32_cmp_neq_v_v_b(a, b, i, p, o)        from_bool64(v_u32_cmp_neq_b(a, b, 0, to_bool64(i), p, o))
#define bv_i16_cmp_neq_v_v_b(a, b, i, p, o)        from_bool128(v_i16_cmp_neq_b(a, b, 0, to_bool128(i), p, o))
#define bv_u16_cmp_neq_v_v_b(a, b, i, p, o)        from_bool128(v_u16_cmp_neq_b(a, b, 0, to_bool128(i), p, o))
#define bv_i8_cmp_neq_v_v_b(a, b, i, p, o)         v_i8_cmp_neq_b(a, b, 0, i, p, o)
#define bv_u8_cmp_neq_v_v_b(a, b, i, p, o)         v_u8_cmp_neq_b(a, b, 0, i, p, o)

#define bv_f32_cmp_neq_v_s_b(a, b, i, p, o)        bv_f32_cmp_neq_v_v_b(a, b, i, p, o)
#define bv_bf16_cmp_neq_v_s_b(a, b, i, p, o)       bv_bf16_cmp_neq_v_v_b(a, b, i, p, o)
#define bv_i32_cmp_neq_v_s_b(a, b, i, p, o)        bv_i32_cmp_neq_v_v_b(a, b, i, p, o)
#define bv_u32_cmp_neq_v_s_b(a, b, i, p, o)        bv_u32_cmp_neq_v_v_b(a, b, i, p, o)
#define bv_i16_cmp_neq_v_s_b(a, b, i, p, o)        bv_i16_cmp_neq_v_v_b(a, b, i, p, o)
#define bv_u16_cmp_neq_v_s_b(a, b, i, p, o)        bv_u16_cmp_neq_v_v_b(a, b, i, p, o)
#define bv_i8_cmp_neq_v_s_b(a, b, i, p, o)         bv_i8_cmp_neq_v_v_b(a, b, i, p, o)
#define bv_u8_cmp_neq_v_s_b(a, b, i, p, o)         bv_u8_cmp_neq_v_v_b(a, b, i, p, o)

#define bv_f32_cmp_neq_v_v(a, b)                   bv_f32_cmp_neq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_bf16_cmp_neq_v_v(a, b)                  bv_bf16_cmp_neq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i32_cmp_neq_v_v(a, b)                   bv_i32_cmp_neq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u32_cmp_neq_v_v(a, b)                   bv_u32_cmp_neq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i16_cmp_neq_v_v(a, b)                   bv_i16_cmp_neq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u16_cmp_neq_v_v(a, b)                   bv_u16_cmp_neq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i8_cmp_neq_v_v(a, b)                    bv_i8_cmp_neq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u8_cmp_neq_v_v(a, b)                    bv_u8_cmp_neq_v_v_b(a, b, (bool256){0}, 1, 0)

#define bv_f32_cmp_neq_v_s(a, b)                   bv_f32_cmp_neq_v_v(a, b)
#define bv_i32_cmp_neq_v_s(a, b)                   bv_i32_cmp_neq_v_v(a, b)
#define bv_u32_cmp_neq_v_s(a, b)                   bv_u32_cmp_neq_v_v(a, b)
#define bv_i16_cmp_neq_v_s(a, b)                   bv_i16_cmp_neq_v_v(a, b)
#define bv_u16_cmp_neq_v_s(a, b)                   bv_u16_cmp_neq_v_v(a, b)
#define bv_i8_cmp_neq_v_s(a, b)                    bv_i8_cmp_neq_v_v(a, b)
#define bv_u8_cmp_neq_v_s(a, b)                    bv_u8_cmp_neq_v_v(a, b)
#define bv_bf16_cmp_neq_v_s(a, b)                  bv_bf16_cmp_neq_v_v(a, b)


// CMP_LESS

#define b_f32_cmp_less_s_s_b(a, b, i, p, o)         s_f32_cmp_less(a, b, 0, i, p, o)
#define b_bf16_cmp_less_s_s_b(a, b, i, p, o)        s_bf16_cmp_less(a, b, 0, i, p, o)
#define b_i32_cmp_less_s_s_b(a, b, i, p, o)         s_i32_cmp_less(a, b, 0, i, p, o)
#define b_u32_cmp_less_s_s_b(a, b, i, p, o)         s_u32_cmp_less(a, b, 0, i, p, o)
#define b_i16_cmp_less_s_s_b(a, b, i, p, o)         s_i16_cmp_less(a, b, 0, i, p, o)
#define b_u16_cmp_less_s_s_b(a, b, i, p, o)         s_u16_cmp_less(a, b, 0, i, p, o)
#define b_i8_cmp_less_s_s_b(a, b, i, p, o)          s_i8_cmp_less(a, b, 0, i, p, o)
#define b_u8_cmp_less_s_s_b(a, b, i, p, o)          s_u8_cmp_less(a, b, 0, i, p, o)

#define b_f32_cmp_less_s_s(a, b)                    b_f32_cmp_less_s_s_b(a, b, 0, 1, 0)
#define b_bf16_cmp_less_s_s(a, b)                   b_bf16_cmp_less_s_s_b(a, b, 0, 1, 0)
#define b_i32_cmp_less_s_s(a, b)                    b_i32_cmp_less_s_s_b(a, b, 0, 1, 0)
#define b_u32_cmp_less_s_s(a, b)                    b_u32_cmp_less_s_s_b(a, b, 0, 1, 0)
#define b_i16_cmp_less_s_s(a, b)                    b_i16_cmp_less_s_s_b(a, b, 0, 1, 0)
#define b_u16_cmp_less_s_s(a, b)                    b_u16_cmp_less_s_s_b(a, b, 0, 1, 0)
#define b_i8_cmp_less_s_s(a, b)                     b_i8_cmp_less_s_s_b(a, b, 0, 1, 0)
#define b_u8_cmp_less_s_s(a, b)                     b_u8_cmp_less_s_s_b(a, b, 0, 1, 0)

#define bv_f32_cmp_less_v_v_vb(a, b, i, p, o)       from_bool64(v_f32_cmp_less_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_bf16_cmp_less_v_v_vb(a, b, i, p, o)      from_bool128(v_bf16_cmp_less_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_i32_cmp_less_v_v_vb(a, b, i, p, o)       from_bool64(v_i32_cmp_less_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_u32_cmp_less_v_v_vb(a, b, i, p, o)       from_bool64(v_u32_cmp_less_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_i16_cmp_less_v_v_vb(a, b, i, p, o)       from_bool128(v_i16_cmp_less_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_u16_cmp_less_v_v_vb(a, b, i, p, o)       from_bool128(v_u16_cmp_less_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_i8_cmp_less_v_v_vb(a, b, i, p, o)        v_i8_cmp_less_vb(a, b, 0, i, p, o)
#define bv_u8_cmp_less_v_v_vb(a, b, i, p, o)        v_u8_cmp_less_vb(a, b, 0, i, p, o)

#define bv_f32_cmp_less_v_s_vb(a, b, i, p, o)       bv_f32_cmp_less_v_v_vb(a, b, i, p, o)
#define bv_bf16_cmp_less_v_s_vb(a, b, i, p, o)      bv_bf16_cmp_less_v_v_vb(a, b, i, p, o)
#define bv_i32_cmp_less_v_s_vb(a, b, i, p, o)       bv_i32_cmp_less_v_v_vb(a, b, i, p, o)
#define bv_u32_cmp_less_v_s_vb(a, b, i, p, o)       bv_u32_cmp_less_v_v_vb(a, b, i, p, o)
#define bv_i16_cmp_less_v_s_vb(a, b, i, p, o)       bv_i16_cmp_less_v_v_vb(a, b, i, p, o)
#define bv_u16_cmp_less_v_s_vb(a, b, i, p, o)       bv_u16_cmp_less_v_v_vb(a, b, i, p, o)
#define bv_i8_cmp_less_v_s_vb(a, b, i, p, o)        bv_i8_cmp_less_v_v_vb(a, b, i, p, o)
#define bv_u8_cmp_less_v_s_vb(a, b, i, p, o)        bv_u8_cmp_less_v_v_vb(a, b, i, p, o)

#define bv_f32_cmp_less_v_v_b(a, b, i, p, o)        from_bool64(v_f32_cmp_less_b(a, b, 0, to_bool64(i), p, o))
#define bv_bf16_cmp_less_v_v_b(a, b, i, p, o)       from_bool128(v_bf16_cmp_less_b(a, b, 0, to_bool128(i), p, o))
#define bv_i32_cmp_less_v_v_b(a, b, i, p, o)        from_bool64(v_i32_cmp_less_b(a, b, 0, to_bool64(i), p, o))
#define bv_u32_cmp_less_v_v_b(a, b, i, p, o)        from_bool64(v_u32_cmp_less_b(a, b, 0, to_bool64(i), p, o))
#define bv_i16_cmp_less_v_v_b(a, b, i, p, o)        from_bool128(v_i16_cmp_less_b(a, b, 0, to_bool128(i), p, o))
#define bv_u16_cmp_less_v_v_b(a, b, i, p, o)        from_bool128(v_u16_cmp_less_b(a, b, 0, to_bool128(i), p, o))
#define bv_i8_cmp_less_v_v_b(a, b, i, p, o)         v_i8_cmp_less_b(a, b, 0, i, p, o)
#define bv_u8_cmp_less_v_v_b(a, b, i, p, o)         v_u8_cmp_less_b(a, b, 0, i, p, o)

#define bv_f32_cmp_less_v_s_b(a, b, i, p, o)        bv_f32_cmp_less_v_v_b(a, b, i, p, o)
#define bv_bf16_cmp_less_v_s_b(a, b, i, p, o)       bv_bf16_cmp_less_v_v_b(a, b, i, p, o)
#define bv_i32_cmp_less_v_s_b(a, b, i, p, o)        bv_i32_cmp_less_v_v_b(a, b, i, p, o)
#define bv_u32_cmp_less_v_s_b(a, b, i, p, o)        bv_u32_cmp_less_v_v_b(a, b, i, p, o)
#define bv_i16_cmp_less_v_s_b(a, b, i, p, o)        bv_i16_cmp_less_v_v_b(a, b, i, p, o)
#define bv_u16_cmp_less_v_s_b(a, b, i, p, o)        bv_u16_cmp_less_v_v_b(a, b, i, p, o)
#define bv_i8_cmp_less_v_s_b(a, b, i, p, o)         bv_i8_cmp_less_v_v_b(a, b, i, p, o)
#define bv_u8_cmp_less_v_s_b(a, b, i, p, o)         bv_u8_cmp_less_v_v_b(a, b, i, p, o)

#define bv_f32_cmp_less_v_v(a, b)                   bv_f32_cmp_less_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_bf16_cmp_less_v_v(a, b)                  bv_bf16_cmp_less_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i32_cmp_less_v_v(a, b)                   bv_i32_cmp_less_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u32_cmp_less_v_v(a, b)                   bv_u32_cmp_less_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i16_cmp_less_v_v(a, b)                   bv_i16_cmp_less_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u16_cmp_less_v_v(a, b)                   bv_u16_cmp_less_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i8_cmp_less_v_v(a, b)                    bv_i8_cmp_less_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u8_cmp_less_v_v(a, b)                    bv_u8_cmp_less_v_v_b(a, b, (bool256){0}, 1, 0)

#define bv_f32_cmp_less_v_s(a, b)                   bv_f32_cmp_less_v_v(a, b)
#define bv_i32_cmp_less_v_s(a, b)                   bv_i32_cmp_less_v_v(a, b)
#define bv_u32_cmp_less_v_s(a, b)                   bv_u32_cmp_less_v_v(a, b)
#define bv_i16_cmp_less_v_s(a, b)                   bv_i16_cmp_less_v_v(a, b)
#define bv_u16_cmp_less_v_s(a, b)                   bv_u16_cmp_less_v_v(a, b)
#define bv_i8_cmp_less_v_s(a, b)                    bv_i8_cmp_less_v_v(a, b)
#define bv_u8_cmp_less_v_s(a, b)                    bv_u8_cmp_less_v_v(a, b)
#define bv_bf16_cmp_less_v_s(a, b)                  bv_bf16_cmp_less_v_v(a, b)


// CMP_LEQ

#define b_f32_cmp_leq_s_s_b(a, b, i, p, o)         s_f32_cmp_leq(a, b, 0, i, p, o)
#define b_bf16_cmp_leq_s_s_b(a, b, i, p, o)        s_bf16_cmp_leq(a, b, 0, i, p, o)
#define b_i32_cmp_leq_s_s_b(a, b, i, p, o)         s_i32_cmp_leq(a, b, 0, i, p, o)
#define b_u32_cmp_leq_s_s_b(a, b, i, p, o)         s_u32_cmp_leq(a, b, 0, i, p, o)
#define b_i16_cmp_leq_s_s_b(a, b, i, p, o)         s_i16_cmp_leq(a, b, 0, i, p, o)
#define b_u16_cmp_leq_s_s_b(a, b, i, p, o)         s_u16_cmp_leq(a, b, 0, i, p, o)
#define b_i8_cmp_leq_s_s_b(a, b, i, p, o)          s_i8_cmp_leq(a, b, 0, i, p, o)
#define b_u8_cmp_leq_s_s_b(a, b, i, p, o)          s_u8_cmp_leq(a, b, 0, i, p, o)

#define b_f32_cmp_leq_s_s(a, b)                    b_f32_cmp_leq_s_s_b(a, b, 0, 1, 0)
#define b_bf16_cmp_leq_s_s(a, b)                   b_bf16_cmp_leq_s_s_b(a, b, 0, 1, 0)
#define b_i32_cmp_leq_s_s(a, b)                    b_i32_cmp_leq_s_s_b(a, b, 0, 1, 0)
#define b_u32_cmp_leq_s_s(a, b)                    b_u32_cmp_leq_s_s_b(a, b, 0, 1, 0)
#define b_i16_cmp_leq_s_s(a, b)                    b_i16_cmp_leq_s_s_b(a, b, 0, 1, 0)
#define b_u16_cmp_leq_s_s(a, b)                    b_u16_cmp_leq_s_s_b(a, b, 0, 1, 0)
#define b_i8_cmp_leq_s_s(a, b)                     b_i8_cmp_leq_s_s_b(a, b, 0, 1, 0)
#define b_u8_cmp_leq_s_s(a, b)                     b_u8_cmp_leq_s_s_b(a, b, 0, 1, 0)

#define bv_f32_cmp_leq_v_v_vb(a, b, i, p, o)       from_bool64(v_f32_cmp_leq_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_bf16_cmp_leq_v_v_vb(a, b, i, p, o)      from_bool128(v_bf16_cmp_leq_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_i32_cmp_leq_v_v_vb(a, b, i, p, o)       from_bool64(v_i32_cmp_leq_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_u32_cmp_leq_v_v_vb(a, b, i, p, o)       from_bool64(v_u32_cmp_leq_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_i16_cmp_leq_v_v_vb(a, b, i, p, o)       from_bool128(v_i16_cmp_leq_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_u16_cmp_leq_v_v_vb(a, b, i, p, o)       from_bool128(v_u16_cmp_leq_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_i8_cmp_leq_v_v_vb(a, b, i, p, o)        v_i8_cmp_leq_vb(a, b, 0, i, p, o)
#define bv_u8_cmp_leq_v_v_vb(a, b, i, p, o)        v_u8_cmp_leq_vb(a, b, 0, i, p, o)

#define bv_f32_cmp_leq_v_s_vb(a, b, i, p, o)       bv_f32_cmp_leq_v_v_vb(a, b, i, p, o)
#define bv_bf16_cmp_leq_v_s_vb(a, b, i, p, o)      bv_bf16_cmp_leq_v_v_vb(a, b, i, p, o)
#define bv_i32_cmp_leq_v_s_vb(a, b, i, p, o)       bv_i32_cmp_leq_v_v_vb(a, b, i, p, o)
#define bv_u32_cmp_leq_v_s_vb(a, b, i, p, o)       bv_u32_cmp_leq_v_v_vb(a, b, i, p, o)
#define bv_i16_cmp_leq_v_s_vb(a, b, i, p, o)       bv_i16_cmp_leq_v_v_vb(a, b, i, p, o)
#define bv_u16_cmp_leq_v_s_vb(a, b, i, p, o)       bv_u16_cmp_leq_v_v_vb(a, b, i, p, o)
#define bv_i8_cmp_leq_v_s_vb(a, b, i, p, o)        bv_i8_cmp_leq_v_v_vb(a, b, i, p, o)
#define bv_u8_cmp_leq_v_s_vb(a, b, i, p, o)        bv_u8_cmp_leq_v_v_vb(a, b, i, p, o)

#define bv_f32_cmp_leq_v_v_b(a, b, i, p, o)        from_bool64(v_f32_cmp_leq_b(a, b, 0, to_bool64(i), p, o))
#define bv_bf16_cmp_leq_v_v_b(a, b, i, p, o)       from_bool128(v_bf16_cmp_leq_b(a, b, 0, to_bool128(i), p, o))
#define bv_i32_cmp_leq_v_v_b(a, b, i, p, o)        from_bool64(v_i32_cmp_leq_b(a, b, 0, to_bool64(i), p, o))
#define bv_u32_cmp_leq_v_v_b(a, b, i, p, o)        from_bool64(v_u32_cmp_leq_b(a, b, 0, to_bool64(i), p, o))
#define bv_i16_cmp_leq_v_v_b(a, b, i, p, o)        from_bool128(v_i16_cmp_leq_b(a, b, 0, to_bool128(i), p, o))
#define bv_u16_cmp_leq_v_v_b(a, b, i, p, o)        from_bool128(v_u16_cmp_leq_b(a, b, 0, to_bool128(i), p, o))
#define bv_i8_cmp_leq_v_v_b(a, b, i, p, o)         v_i8_cmp_leq_b(a, b, 0, i, p, o)
#define bv_u8_cmp_leq_v_v_b(a, b, i, p, o)         v_u8_cmp_leq_b(a, b, 0, i, p, o)

#define bv_f32_cmp_leq_v_s_b(a, b, i, p, o)        bv_f32_cmp_leq_v_v_b(a, b, i, p, o)
#define bv_bf16_cmp_leq_v_s_b(a, b, i, p, o)       bv_bf16_cmp_leq_v_v_b(a, b, i, p, o)
#define bv_i32_cmp_leq_v_s_b(a, b, i, p, o)        bv_i32_cmp_leq_v_v_b(a, b, i, p, o)
#define bv_u32_cmp_leq_v_s_b(a, b, i, p, o)        bv_u32_cmp_leq_v_v_b(a, b, i, p, o)
#define bv_i16_cmp_leq_v_s_b(a, b, i, p, o)        bv_i16_cmp_leq_v_v_b(a, b, i, p, o)
#define bv_u16_cmp_leq_v_s_b(a, b, i, p, o)        bv_u16_cmp_leq_v_v_b(a, b, i, p, o)
#define bv_i8_cmp_leq_v_s_b(a, b, i, p, o)         bv_i8_cmp_leq_v_v_b(a, b, i, p, o)
#define bv_u8_cmp_leq_v_s_b(a, b, i, p, o)         bv_u8_cmp_leq_v_v_b(a, b, i, p, o)

#define bv_f32_cmp_leq_v_v(a, b)                   bv_f32_cmp_leq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_bf16_cmp_leq_v_v(a, b)                  bv_bf16_cmp_leq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i32_cmp_leq_v_v(a, b)                   bv_i32_cmp_leq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u32_cmp_leq_v_v(a, b)                   bv_u32_cmp_leq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i16_cmp_leq_v_v(a, b)                   bv_i16_cmp_leq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u16_cmp_leq_v_v(a, b)                   bv_u16_cmp_leq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i8_cmp_leq_v_v(a, b)                    bv_i8_cmp_leq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u8_cmp_leq_v_v(a, b)                    bv_u8_cmp_leq_v_v_b(a, b, (bool256){0}, 1, 0)

#define bv_f32_cmp_leq_v_s(a, b)                   bv_f32_cmp_leq_v_v(a, b)
#define bv_i32_cmp_leq_v_s(a, b)                   bv_i32_cmp_leq_v_v(a, b)
#define bv_u32_cmp_leq_v_s(a, b)                   bv_u32_cmp_leq_v_v(a, b)
#define bv_i16_cmp_leq_v_s(a, b)                   bv_i16_cmp_leq_v_v(a, b)
#define bv_u16_cmp_leq_v_s(a, b)                   bv_u16_cmp_leq_v_v(a, b)
#define bv_i8_cmp_leq_v_s(a, b)                    bv_i8_cmp_leq_v_v(a, b)
#define bv_u8_cmp_leq_v_s(a, b)                    bv_u8_cmp_leq_v_v(a, b)
#define bv_bf16_cmp_leq_v_s(a, b)                  bv_bf16_cmp_leq_v_v(a, b)


// CMP_GRT

#define b_f32_cmp_grt_s_s_b(a, b, i, p, o)         s_f32_cmp_grt(a, b, 0, i, p, o)
#define b_bf16_cmp_grt_s_s_b(a, b, i, p, o)        s_bf16_cmp_grt(a, b, 0, i, p, o)
#define b_i32_cmp_grt_s_s_b(a, b, i, p, o)         s_i32_cmp_grt(a, b, 0, i, p, o)
#define b_u32_cmp_grt_s_s_b(a, b, i, p, o)         s_u32_cmp_grt(a, b, 0, i, p, o)
#define b_i16_cmp_grt_s_s_b(a, b, i, p, o)         s_i16_cmp_grt(a, b, 0, i, p, o)
#define b_u16_cmp_grt_s_s_b(a, b, i, p, o)         s_u16_cmp_grt(a, b, 0, i, p, o)
#define b_i8_cmp_grt_s_s_b(a, b, i, p, o)          s_i8_cmp_grt(a, b, 0, i, p, o)
#define b_u8_cmp_grt_s_s_b(a, b, i, p, o)          s_u8_cmp_grt(a, b, 0, i, p, o)

#define b_f32_cmp_grt_s_s(a, b)                    b_f32_cmp_grt_s_s_b(a, b, 0, 1, 0)
#define b_bf16_cmp_grt_s_s(a, b)                   b_bf16_cmp_grt_s_s_b(a, b, 0, 1, 0)
#define b_i32_cmp_grt_s_s(a, b)                    b_i32_cmp_grt_s_s_b(a, b, 0, 1, 0)
#define b_u32_cmp_grt_s_s(a, b)                    b_u32_cmp_grt_s_s_b(a, b, 0, 1, 0)
#define b_i16_cmp_grt_s_s(a, b)                    b_i16_cmp_grt_s_s_b(a, b, 0, 1, 0)
#define b_u16_cmp_grt_s_s(a, b)                    b_u16_cmp_grt_s_s_b(a, b, 0, 1, 0)
#define b_i8_cmp_grt_s_s(a, b)                     b_i8_cmp_grt_s_s_b(a, b, 0, 1, 0)
#define b_u8_cmp_grt_s_s(a, b)                     b_u8_cmp_grt_s_s_b(a, b, 0, 1, 0)

#define bv_f32_cmp_grt_v_v_vb(a, b, i, p, o)       from_bool64(v_f32_cmp_grt_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_bf16_cmp_grt_v_v_vb(a, b, i, p, o)      from_bool128(v_bf16_cmp_grt_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_i32_cmp_grt_v_v_vb(a, b, i, p, o)       from_bool64(v_i32_cmp_grt_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_u32_cmp_grt_v_v_vb(a, b, i, p, o)       from_bool64(v_u32_cmp_grt_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_i16_cmp_grt_v_v_vb(a, b, i, p, o)       from_bool128(v_i16_cmp_grt_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_u16_cmp_grt_v_v_vb(a, b, i, p, o)       from_bool128(v_u16_cmp_grt_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_i8_cmp_grt_v_v_vb(a, b, i, p, o)        v_i8_cmp_grt_vb(a, b, 0, i, p, o)
#define bv_u8_cmp_grt_v_v_vb(a, b, i, p, o)        v_u8_cmp_grt_vb(a, b, 0, i, p, o)

#define bv_f32_cmp_grt_v_s_vb(a, b, i, p, o)       bv_f32_cmp_grt_v_v_vb(a, b, i, p, o)
#define bv_bf16_cmp_grt_v_s_vb(a, b, i, p, o)      bv_bf16_cmp_grt_v_v_vb(a, b, i, p, o)
#define bv_i32_cmp_grt_v_s_vb(a, b, i, p, o)       bv_i32_cmp_grt_v_v_vb(a, b, i, p, o)
#define bv_u32_cmp_grt_v_s_vb(a, b, i, p, o)       bv_u32_cmp_grt_v_v_vb(a, b, i, p, o)
#define bv_i16_cmp_grt_v_s_vb(a, b, i, p, o)       bv_i16_cmp_grt_v_v_vb(a, b, i, p, o)
#define bv_u16_cmp_grt_v_s_vb(a, b, i, p, o)       bv_u16_cmp_grt_v_v_vb(a, b, i, p, o)
#define bv_i8_cmp_grt_v_s_vb(a, b, i, p, o)        bv_i8_cmp_grt_v_v_vb(a, b, i, p, o)
#define bv_u8_cmp_grt_v_s_vb(a, b, i, p, o)        bv_u8_cmp_grt_v_v_vb(a, b, i, p, o)

#define bv_f32_cmp_grt_v_v_b(a, b, i, p, o)        from_bool64(v_f32_cmp_grt_b(a, b, 0, to_bool64(i), p, o))
#define bv_bf16_cmp_grt_v_v_b(a, b, i, p, o)       from_bool128(v_bf16_cmp_grt_b(a, b, 0, to_bool128(i), p, o))
#define bv_i32_cmp_grt_v_v_b(a, b, i, p, o)        from_bool64(v_i32_cmp_grt_b(a, b, 0, to_bool64(i), p, o))
#define bv_u32_cmp_grt_v_v_b(a, b, i, p, o)        from_bool64(v_u32_cmp_grt_b(a, b, 0, to_bool64(i), p, o))
#define bv_i16_cmp_grt_v_v_b(a, b, i, p, o)        from_bool128(v_i16_cmp_grt_b(a, b, 0, to_bool128(i), p, o))
#define bv_u16_cmp_grt_v_v_b(a, b, i, p, o)        from_bool128(v_u16_cmp_grt_b(a, b, 0, to_bool128(i), p, o))
#define bv_i8_cmp_grt_v_v_b(a, b, i, p, o)         v_i8_cmp_grt_b(a, b, 0, i, p, o)
#define bv_u8_cmp_grt_v_v_b(a, b, i, p, o)         v_u8_cmp_grt_b(a, b, 0, i, p, o)

#define bv_f32_cmp_grt_v_s_b(a, b, i, p, o)        bv_f32_cmp_grt_v_v_b(a, b, i, p, o)
#define bv_bf16_cmp_grt_v_s_b(a, b, i, p, o)       bv_bf16_cmp_grt_v_v_b(a, b, i, p, o)
#define bv_i32_cmp_grt_v_s_b(a, b, i, p, o)        bv_i32_cmp_grt_v_v_b(a, b, i, p, o)
#define bv_u32_cmp_grt_v_s_b(a, b, i, p, o)        bv_u32_cmp_grt_v_v_b(a, b, i, p, o)
#define bv_i16_cmp_grt_v_s_b(a, b, i, p, o)        bv_i16_cmp_grt_v_v_b(a, b, i, p, o)
#define bv_u16_cmp_grt_v_s_b(a, b, i, p, o)        bv_u16_cmp_grt_v_v_b(a, b, i, p, o)
#define bv_i8_cmp_grt_v_s_b(a, b, i, p, o)         bv_i8_cmp_grt_v_v_b(a, b, i, p, o)
#define bv_u8_cmp_grt_v_s_b(a, b, i, p, o)         bv_u8_cmp_grt_v_v_b(a, b, i, p, o)

#define bv_f32_cmp_grt_v_v(a, b)                   bv_f32_cmp_grt_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_bf16_cmp_grt_v_v(a, b)                  bv_bf16_cmp_grt_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i32_cmp_grt_v_v(a, b)                   bv_i32_cmp_grt_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u32_cmp_grt_v_v(a, b)                   bv_u32_cmp_grt_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i16_cmp_grt_v_v(a, b)                   bv_i16_cmp_grt_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u16_cmp_grt_v_v(a, b)                   bv_u16_cmp_grt_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i8_cmp_grt_v_v(a, b)                    bv_i8_cmp_grt_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u8_cmp_grt_v_v(a, b)                    bv_u8_cmp_grt_v_v_b(a, b, (bool256){0}, 1, 0)

#define bv_f32_cmp_grt_v_s(a, b)                   bv_f32_cmp_grt_v_v(a, b)
#define bv_i32_cmp_grt_v_s(a, b)                   bv_i32_cmp_grt_v_v(a, b)
#define bv_u32_cmp_grt_v_s(a, b)                   bv_u32_cmp_grt_v_v(a, b)
#define bv_i16_cmp_grt_v_s(a, b)                   bv_i16_cmp_grt_v_v(a, b)
#define bv_u16_cmp_grt_v_s(a, b)                   bv_u16_cmp_grt_v_v(a, b)
#define bv_i8_cmp_grt_v_s(a, b)                    bv_i8_cmp_grt_v_v(a, b)
#define bv_u8_cmp_grt_v_s(a, b)                    bv_u8_cmp_grt_v_v(a, b)
#define bv_bf16_cmp_grt_v_s(a, b)                  bv_bf16_cmp_grt_v_v(a, b)


// CMP_GEQ

#define b_f32_cmp_geq_s_s_b(a, b, i, p, o)         s_f32_cmp_geq(a, b, 0, i, p, o)
#define b_bf16_cmp_geq_s_s_b(a, b, i, p, o)        s_bf16_cmp_geq(a, b, 0, i, p, o)
#define b_i32_cmp_geq_s_s_b(a, b, i, p, o)         s_i32_cmp_geq(a, b, 0, i, p, o)
#define b_u32_cmp_geq_s_s_b(a, b, i, p, o)         s_u32_cmp_geq(a, b, 0, i, p, o)
#define b_i16_cmp_geq_s_s_b(a, b, i, p, o)         s_i16_cmp_geq(a, b, 0, i, p, o)
#define b_u16_cmp_geq_s_s_b(a, b, i, p, o)         s_u16_cmp_geq(a, b, 0, i, p, o)
#define b_i8_cmp_geq_s_s_b(a, b, i, p, o)          s_i8_cmp_geq(a, b, 0, i, p, o)
#define b_u8_cmp_geq_s_s_b(a, b, i, p, o)          s_u8_cmp_geq(a, b, 0, i, p, o)

#define b_f32_cmp_geq_s_s(a, b)                    b_f32_cmp_geq_s_s_b(a, b, 0, 1, 0)
#define b_bf16_cmp_geq_s_s(a, b)                   b_bf16_cmp_geq_s_s_b(a, b, 0, 1, 0)
#define b_i32_cmp_geq_s_s(a, b)                    b_i32_cmp_geq_s_s_b(a, b, 0, 1, 0)
#define b_u32_cmp_geq_s_s(a, b)                    b_u32_cmp_geq_s_s_b(a, b, 0, 1, 0)
#define b_i16_cmp_geq_s_s(a, b)                    b_i16_cmp_geq_s_s_b(a, b, 0, 1, 0)
#define b_u16_cmp_geq_s_s(a, b)                    b_u16_cmp_geq_s_s_b(a, b, 0, 1, 0)
#define b_i8_cmp_geq_s_s(a, b)                     b_i8_cmp_geq_s_s_b(a, b, 0, 1, 0)
#define b_u8_cmp_geq_s_s(a, b)                     b_u8_cmp_geq_s_s_b(a, b, 0, 1, 0)

#define bv_f32_cmp_geq_v_v_vb(a, b, i, p, o)       from_bool64(v_f32_cmp_geq_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_bf16_cmp_geq_v_v_vb(a, b, i, p, o)      from_bool128(v_bf16_cmp_geq_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_i32_cmp_geq_v_v_vb(a, b, i, p, o)       from_bool64(v_i32_cmp_geq_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_u32_cmp_geq_v_v_vb(a, b, i, p, o)       from_bool64(v_u32_cmp_geq_vb(a, b, 0, to_bool64(i), to_bool64(p), o))
#define bv_i16_cmp_geq_v_v_vb(a, b, i, p, o)       from_bool128(v_i16_cmp_geq_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_u16_cmp_geq_v_v_vb(a, b, i, p, o)       from_bool128(v_u16_cmp_geq_vb(a, b, 0, to_bool128(i), to_bool128(p), o))
#define bv_i8_cmp_geq_v_v_vb(a, b, i, p, o)        v_i8_cmp_geq_vb(a, b, 0, i, p, o)
#define bv_u8_cmp_geq_v_v_vb(a, b, i, p, o)        v_u8_cmp_geq_vb(a, b, 0, i, p, o)

#define bv_f32_cmp_geq_v_s_vb(a, b, i, p, o)       bv_f32_cmp_geq_v_v_vb(a, b, i, p, o)
#define bv_bf16_cmp_geq_v_s_vb(a, b, i, p, o)      bv_bf16_cmp_geq_v_v_vb(a, b, i, p, o)
#define bv_i32_cmp_geq_v_s_vb(a, b, i, p, o)       bv_i32_cmp_geq_v_v_vb(a, b, i, p, o)
#define bv_u32_cmp_geq_v_s_vb(a, b, i, p, o)       bv_u32_cmp_geq_v_v_vb(a, b, i, p, o)
#define bv_i16_cmp_geq_v_s_vb(a, b, i, p, o)       bv_i16_cmp_geq_v_v_vb(a, b, i, p, o)
#define bv_u16_cmp_geq_v_s_vb(a, b, i, p, o)       bv_u16_cmp_geq_v_v_vb(a, b, i, p, o)
#define bv_i8_cmp_geq_v_s_vb(a, b, i, p, o)        bv_i8_cmp_geq_v_v_vb(a, b, i, p, o)
#define bv_u8_cmp_geq_v_s_vb(a, b, i, p, o)        bv_u8_cmp_geq_v_v_vb(a, b, i, p, o)

#define bv_f32_cmp_geq_v_v_b(a, b, i, p, o)        from_bool64(v_f32_cmp_geq_b(a, b, 0, to_bool64(i), p, o))
#define bv_bf16_cmp_geq_v_v_b(a, b, i, p, o)       from_bool128(v_bf16_cmp_geq_b(a, b, 0, to_bool128(i), p, o))
#define bv_i32_cmp_geq_v_v_b(a, b, i, p, o)        from_bool64(v_i32_cmp_geq_b(a, b, 0, to_bool64(i), p, o))
#define bv_u32_cmp_geq_v_v_b(a, b, i, p, o)        from_bool64(v_u32_cmp_geq_b(a, b, 0, to_bool64(i), p, o))
#define bv_i16_cmp_geq_v_v_b(a, b, i, p, o)        from_bool128(v_i16_cmp_geq_b(a, b, 0, to_bool128(i), p, o))
#define bv_u16_cmp_geq_v_v_b(a, b, i, p, o)        from_bool128(v_u16_cmp_geq_b(a, b, 0, to_bool128(i), p, o))
#define bv_i8_cmp_geq_v_v_b(a, b, i, p, o)         v_i8_cmp_geq_b(a, b, 0, i, p, o)
#define bv_u8_cmp_geq_v_v_b(a, b, i, p, o)         v_u8_cmp_geq_b(a, b, 0, i, p, o)

#define bv_f32_cmp_geq_v_s_b(a, b, i, p, o)        bv_f32_cmp_geq_v_v_b(a, b, i, p, o)
#define bv_bf16_cmp_geq_v_s_b(a, b, i, p, o)       bv_bf16_cmp_geq_v_v_b(a, b, i, p, o)
#define bv_i32_cmp_geq_v_s_b(a, b, i, p, o)        bv_i32_cmp_geq_v_v_b(a, b, i, p, o)
#define bv_u32_cmp_geq_v_s_b(a, b, i, p, o)        bv_u32_cmp_geq_v_v_b(a, b, i, p, o)
#define bv_i16_cmp_geq_v_s_b(a, b, i, p, o)        bv_i16_cmp_geq_v_v_b(a, b, i, p, o)
#define bv_u16_cmp_geq_v_s_b(a, b, i, p, o)        bv_u16_cmp_geq_v_v_b(a, b, i, p, o)
#define bv_i8_cmp_geq_v_s_b(a, b, i, p, o)         bv_i8_cmp_geq_v_v_b(a, b, i, p, o)
#define bv_u8_cmp_geq_v_s_b(a, b, i, p, o)         bv_u8_cmp_geq_v_v_b(a, b, i, p, o)

#define bv_f32_cmp_geq_v_v(a, b)                   bv_f32_cmp_geq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_bf16_cmp_geq_v_v(a, b)                  bv_bf16_cmp_geq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i32_cmp_geq_v_v(a, b)                   bv_i32_cmp_geq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u32_cmp_geq_v_v(a, b)                   bv_u32_cmp_geq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i16_cmp_geq_v_v(a, b)                   bv_i16_cmp_geq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u16_cmp_geq_v_v(a, b)                   bv_u16_cmp_geq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_i8_cmp_geq_v_v(a, b)                    bv_i8_cmp_geq_v_v_b(a, b, (bool256){0}, 1, 0)
#define bv_u8_cmp_geq_v_v(a, b)                    bv_u8_cmp_geq_v_v_b(a, b, (bool256){0}, 1, 0)

#define bv_f32_cmp_geq_v_s(a, b)                   bv_f32_cmp_geq_v_v(a, b)
#define bv_i32_cmp_geq_v_s(a, b)                   bv_i32_cmp_geq_v_v(a, b)
#define bv_u32_cmp_geq_v_s(a, b)                   bv_u32_cmp_geq_v_v(a, b)
#define bv_i16_cmp_geq_v_s(a, b)                   bv_i16_cmp_geq_v_v(a, b)
#define bv_u16_cmp_geq_v_s(a, b)                   bv_u16_cmp_geq_v_v(a, b)
#define bv_i8_cmp_geq_v_s(a, b)                    bv_i8_cmp_geq_v_v(a, b)
#define bv_u8_cmp_geq_v_s(a, b)                    bv_u8_cmp_geq_v_v(a, b)
#define bv_bf16_cmp_geq_v_s(a, b)                  bv_bf16_cmp_geq_v_v(a, b)

// CONVERT_INT32
#define s_convert_int32_to_i16_s_s_b(a, b, i, rm, p, o)  s_convert_int32_to_i16(a, b, (rm) << 16, i, p, o)
#define s_convert_int32_to_i16_s_s(a, b, rm)             s_convert_int32_to_i16_s_s_b(a, b, 0, rm, 1, 0)

#define s_convert_int32_to_i8_s_s_b(a, b, i, rm, p, o)   s_convert_int32_to_i8(a, b, (rm) << 16, i, p, o)
#define s_convert_int32_to_i8_s_s(a, b, rm)              s_convert_int32_to_i8_s_s_b(a, b, 0, rm, 1, 0)

#if defined(__goya__) || defined(__gaudi__)
#define v_convert_int32_to_i16_v_v_b(a, b, i, rm, ls, p, o)      v_convert_int32_to_i16_b(a, b, ls, ((rm) << 16), i, p, o)
#define v_convert_int32_to_i16_v_v_vb(a, b, i, rm, ls, p, o)     v_convert_int32_to_i16_vb(a, b, ls, ((rm) << 16), i, to_bool64(p), o)
#endif
#define v_convert_int32_to_i16_v_v(a, b, i, rm, ls)              v_convert_int32_to_i16_v_v_b(a, b, i, rm, ls, 1, 0)
#define v_convert_int32_to_i16_v_s_b(a, b, i, rm, ls, p, o)      v_convert_int32_to_i16_v_v_b(a, b, i, rm, ls, p, o)
#define v_convert_int32_to_i16_v_s_vb(a, b, i, rm, ls, p, o)     v_convert_int32_to_i16_v_v_vb(a, b, i, rm, ls, p, o)
#define v_convert_int32_to_i16_v_s(a, b, i, rm, ls)              v_convert_int32_to_i16_v_v(a, b, i, rm, ls)

#if defined(__goya__) || defined(__gaudi__)
#define v_convert_int32_to_i8_v_v_b(a, b, i, rm, ls, p, o)       v_convert_int32_to_i8_b(a, b, ls, ((rm) << 16), i, p, o)
#define v_convert_int32_to_i8_v_v_vb(a, b, i, rm, ls, p, o)      v_convert_int32_to_i8_vb(a, b, ls, ((rm) << 16), i, to_bool64(p), o)
#endif
#define v_convert_int32_to_i8_v_v(a, b, i, rm, ls)               v_convert_int32_to_i8_v_v_b(a, b, i, rm, ls, 1, 0)
#define v_convert_int32_to_i8_v_s_b(a, b, i, rm, ls, p, o)       v_convert_int32_to_i8_v_v_b(a, b, i, rm, ls, p, o)
#define v_convert_int32_to_i8_v_s_vb(a, b, i, rm, ls, p, o)      v_convert_int32_to_i8_v_v_vb(a, b, i, rm, ls, p, o)
#define v_convert_int32_to_i8_v_s(a, b, i, rm, ls)               v_convert_int32_to_i8_v_v(a, b, i, rm, ls)

// CONVERT_UINT32
#define s_convert_uint32_to_u16_s_s_b(a, b, i, rm, p, o)         s_convert_uint32_to_u16(a, b, (rm) << 16, i, p, o)
#define s_convert_uint32_to_u16_s_s(a, b, rm)                    s_convert_uint32_to_u16_s_s_b(a, b, 0, rm, 1, 0)

#define s_convert_uint32_to_u8_s_s_b(a, b, i, rm, p, o)          s_convert_uint32_to_u8(a, b, (rm) << 16, i, p, o)
#define s_convert_uint32_to_u8_s_s(a, b, rm)                     s_convert_uint32_to_u8_s_s_b(a, b, 0, rm, 1, 0)

#if defined(__goya__) || defined(__gaudi__)
#define v_convert_uint32_to_u16_v_v_b(a, b, i, rm, ls, p, o)     v_convert_uint32_to_u16_b(a, b, ls, ((rm) << 16), i, p, o)
#define v_convert_uint32_to_u16_v_v_vb(a, b, i, rm, ls, p, o)    v_convert_uint32_to_u16_vb(a, b, ls, ((rm) << 16), i, to_bool64(p), o)
#endif
#define v_convert_uint32_to_u16_v_v(a, b, i, rm, ls)             v_convert_uint32_to_u16_v_v_b(a, b, i, rm, ls, 1, 0)
#define v_convert_uint32_to_u16_v_s_b(a, b, i, rm, ls, p, o)     v_convert_uint32_to_u16_v_v_b(a, b, i, rm, ls, p, o)
#define v_convert_uint32_to_u16_v_s_vb(a, b, i, rm, ls, p, o)    v_convert_uint32_to_u16_v_v_vb(a, b, i, rm, ls, p, o)
#define v_convert_uint32_to_u16_v_s(a, b, i, rm, ls)             v_convert_uint32_to_u16_v_v(a, b, i, rm, ls)

#if defined(__goya__) || defined(__gaudi__)
#define v_convert_uint32_to_u8_v_v_b(a, b, i, rm, ls, p, o)      v_convert_uint32_to_u8_b(a, b, ls, ((rm) << 16), i, p, o)
#define v_convert_uint32_to_u8_v_v_vb(a, b, i, rm, ls, p, o)     v_convert_uint32_to_u8_vb(a, b, ls, ((rm) << 16), i, to_bool64(p), o)
#endif
#define v_convert_uint32_to_u8_v_v(a, b, i, rm, ls)              v_convert_uint32_to_u8_v_v_b(a, b, i, rm, ls, 1, 0)
#define v_convert_uint32_to_u8_v_s_b(a, b, i, rm, ls, p, o)      v_convert_uint32_to_u8_v_v_b(a, b, i, rm, ls, p, o)
#define v_convert_uint32_to_u8_v_s_vb(a, b, i, rm, ls, p, o)     v_convert_uint32_to_u8_v_v_vb(a, b, i, rm, ls, p, o)
#define v_convert_uint32_to_u8_v_s(a, b, i, rm, ls)              v_convert_uint32_to_u8_v_v(a, b, i, rm, ls)

// CONVERT_INT16
#define s_convert_int16_s_s_b(a, b, i, rm, p, o)                 s_convert_int16_to_i8(a, b, (rm) << 16, i, p, o)
#define s_convert_int16_s_s(a, b, rm)                            s_convert_int16_s_s_b(a, b, 0, rm, 1, 0)

#if defined(__goya__) || defined(__gaudi__)
#define v_convert_int16_v_v_b(a, b, i, rm, ls, p, o)             v_convert_int16_to_i8_b(a, b, ls, ((rm) << 16), i, p, o)
#define v_convert_int16_v_v_vb(a, b, i, rm, ls, p, o)            v_convert_int16_to_i8_vb(a, b, ls, ((rm) << 16), i, to_bool128(p), o)
#endif
#define v_convert_int16_v_v(a, b, i, rm, ls)                     v_convert_int16_v_v_b(a, b, i, rm, ls, 1, 0)
#define v_convert_int16_v_s_b(a, b, i, rm, ls, p, o)             v_convert_int16_v_v_b(a, b, i, rm, ls, p, o)
#define v_convert_int16_v_s_vb(a, b, i, rm, ls, p, o)            v_convert_int16_v_v_vb(a, b, i, rm, ls, p, o)
#define v_convert_int16_v_s(a, b, i, rm, ls)                     v_convert_int16_v_v(a, b, i, rm, ls)

// CONVERT_UINT16
#define s_convert_uint16_s_s_b(a, b, i, rm, p, o)                s_convert_uint16_to_u8(a, b, (rm) << 16, i, p, o)
#define s_convert_uint16_s_s(a, b, rm)                           s_convert_uint16_s_s_b(a, b, 0, rm, 1, 0)

#if defined(__goya__) || defined(__gaudi__)
#define v_convert_uint16_v_v_b(a, b, i, rm, ls, p, o)            v_convert_uint16_to_u8_b(a, b, ls, ((rm) << 16), i, p, o)
#define v_convert_uint16_v_v_vb(a, b, i, rm, ls, p, o)           v_convert_uint16_to_u8_vb(a, b, ls, ((rm) << 16), i, to_bool128(p), o)
#endif
#define v_convert_uint16_v_v(a, b, i, rm, ls)                    v_convert_uint16_v_v_b(a, b, i, rm, ls, 1, 0)
#define v_convert_uint16_v_s_b(a, b, i, rm, ls, p, o)            v_convert_uint16_v_v_b(a, b, i, rm, ls, p, o)
#define v_convert_uint16_v_s_vb(a, b, i, rm, ls, p, o)           v_convert_uint16_v_v_vb(a, b, i, rm, ls, p, o)
#define v_convert_uint16_v_s(a, b, i, rm, ls)                    v_convert_uint16_v_v(a, b, i, rm, ls)


// POPCNT
#define s_f32_popcnt_s_b(x, i, s, p, o)     s_f32_popcnt(x, s, i, p, o)
#define s_bf16_popcnt_s_b(x, i, s, p, o)    s_bf16_popcnt(x, s, i, p, o)
#define s_i32_popcnt_s_b(x, i, s, p, o)     s_i32_popcnt(x, s, i, p, o)
#define s_u32_popcnt_s_b(x, i, s, p, o)     s_u32_popcnt(x, s, i, p, o)
#define s_i16_popcnt_s_b(x, i, s, p, o)     s_i16_popcnt(x, s, i, p, o)
#define s_u16_popcnt_s_b(x, i, s, p, o)     s_u16_popcnt(x, s, i, p, o)
#define s_i8_popcnt_s_b(x, i, s, p, o)      s_i8_popcnt(x, s, i, p, o)
#define s_u8_popcnt_s_b(x, i, s, p, o)      s_u8_popcnt(x, s, i, p, o)

#define s_f32_popcnt_s(x, s)                s_f32_popcnt_s_b(x, 0, s, 1, 0)
#define s_bf16_popcnt_s(x, s)               s_bf16_popcnt_s_b(x, 0, s, 1, 0)
#define s_i32_popcnt_s(x, s)                s_i32_popcnt_s_b(x, 0, s, 1, 0)
#define s_u32_popcnt_s(x, s)                s_u32_popcnt_s_b(x, 0, s, 1, 0)
#define s_i16_popcnt_s(x, s)                s_i16_popcnt_s_b(x, 0, s, 1, 0)
#define s_u16_popcnt_s(x, s)                s_u16_popcnt_s_b(x, 0, s, 1, 0)
#define s_i8_popcnt_s(x, s)                 s_i8_popcnt_s_b(x, 0, s, 1, 0)
#define s_u8_popcnt_s(x, s)                 s_u8_popcnt_s_b(x, 0, s, 1, 0)

#define v_f32_popcnt_v_vb(x, i, s, p, o)    v_f32_popcnt_vb(x, s, i, to_bool64(p), o)
#define v_bf16_popcnt_v_vb(x, i, s, p, o)   v_bf16_popcnt_vb(x, s, i, to_bool128(p), o)
#define v_i32_popcnt_v_vb(x, i, s, p, o)    v_i32_popcnt_vb(x, s, i, to_bool64(p), o)
#define v_u32_popcnt_v_vb(x, i, s, p, o)    v_u32_popcnt_vb(x, s, i, to_bool64(p), o)
#define v_i16_popcnt_v_vb(x, i, s, p, o)    v_i16_popcnt_vb(x, s, i, to_bool128(p), o)
#define v_u16_popcnt_v_vb(x, i, s, p, o)    v_u16_popcnt_vb(x, s, i, to_bool128(p), o)
#define v_i8_popcnt_v_vb(x, i, s, p, o)     v_i8_popcnt_vb(x, s, i, p, o)
#define v_u8_popcnt_v_vb(x, i, s, p, o)     v_u8_popcnt_vb(x, s, i, p, o)

#define v_f32_popcnt_v_b(x, i, s, p, o)     v_f32_popcnt_b(x, s, i, p, o)
#define v_bf16_popcnt_v_b(x, i, s, p, o)    v_bf16_popcnt_b(x, s, i, p, o)
#define v_i32_popcnt_v_b(x, i, s, p, o)     v_i32_popcnt_b(x, s, i, p, o)
#define v_u32_popcnt_v_b(x, i, s, p, o)     v_u32_popcnt_b(x, s, i, p, o)
#define v_i16_popcnt_v_b(x, i, s, p, o)     v_i16_popcnt_b(x, s, i, p, o)
#define v_u16_popcnt_v_b(x, i, s, p, o)     v_u16_popcnt_b(x, s, i, p, o)
#define v_i8_popcnt_v_b(x, i, s, p, o)      v_i8_popcnt_b(x, s, i, p, o)
#define v_u8_popcnt_v_b(x, i, s, p, o)      v_u8_popcnt_b(x, s, i, p, o)

#define v_f32_popcnt_v(x, s)                v_f32_popcnt_v_b(x, 0, s, 1, 0)
#define v_bf16_popcnt_v(x, s)               v_bf16_popcnt_v_b(x, 0, s, 1, 0)
#define v_i32_popcnt_v(x, s)                v_i32_popcnt_v_b(x, 0, s, 1, 0)
#define v_u32_popcnt_v(x, s)                v_u32_popcnt_v_b(x, 0, s, 1, 0)
#define v_i16_popcnt_v(x, s)                v_i16_popcnt_v_b(x, 0, s, 1, 0)
#define v_u16_popcnt_v(x, s)                v_u16_popcnt_v_b(x, 0, s, 1, 0)
#define v_i8_popcnt_v(x, s)                 v_i8_popcnt_v_b(x, 0, s, 1, 0)
#define v_u8_popcnt_v(x, s)                 v_u8_popcnt_v_b(x, 0, s, 1, 0)


// FIND_FIRST
#define s_f32_find_first_s_b(x, i, v, d, p, o)     s_f32_find_first(x, ((v) | ((d) << 1)), i, p, o)
#define s_bf16_find_first_s_b(x, i, v, d, p, o)    s_bf16_find_first(x, ((v) | ((d) << 1)), i, p, o)
#define s_i32_find_first_s_b(x, i, v, d, p, o)     s_i32_find_first(x, ((v) | ((d) << 1)), i, p, o)
#define s_u32_find_first_s_b(x, i, v, d, p, o)     s_u32_find_first(x, ((v) | ((d) << 1)), i, p, o)
#define s_i16_find_first_s_b(x, i, v, d, p, o)     s_i16_find_first(x, ((v) | ((d) << 1)), i, p, o)
#define s_u16_find_first_s_b(x, i, v, d, p, o)     s_u16_find_first(x, ((v) | ((d) << 1)), i, p, o)
#define s_i8_find_first_s_b(x, i, v, d, p, o)      s_i8_find_first(x, ((v) | ((d) << 1)), i, p, o)
#define s_u8_find_first_s_b(x, i, v, d, p, o)      s_u8_find_first(x, ((v) | ((d) << 1)), i, p, o)

#define s_f32_find_first_s(x, v, d)                s_f32_find_first_s_b(x, 0, v, d, 1, 0)
#define s_bf16_find_first_s(x, v, d)               s_bf16_find_first_s_b(x, 0, v, d, 1, 0)
#define s_i32_find_first_s(x, v, d)                s_i32_find_first_s_b(x, 0, v, d, 1, 0)
#define s_u32_find_first_s(x, v, d)                s_u32_find_first_s_b(x, 0, v, d, 1, 0)
#define s_i16_find_first_s(x, v, d)                s_i16_find_first_s_b(x, 0, v, d, 1, 0)
#define s_u16_find_first_s(x, v, d)                s_u16_find_first_s_b(x, 0, v, d, 1, 0)
#define s_i8_find_first_s(x, v, d)                 s_i8_find_first_s_b(x, 0, v, d, 1, 0)
#define s_u8_find_first_s(x, v, d)                 s_u8_find_first_s_b(x, 0, v, d, 1, 0)

#define v_f32_find_first_v_vb(x, i, v, d, p, o)    v_f32_find_first_vb(x, ((v) | ((d) << 1)), i, to_bool64(p), o)
#define v_bf16_find_first_v_vb(x, i, v, d, p, o)   v_bf16_find_first_vb(x, ((v) | ((d) << 1)), i, to_bool128(p), o)

#define v_i32_find_first_v_vb(x, i, v, d, p, o)    v_i32_find_first_vb(x, ((v) | ((d) << 1)), i, to_bool64(p), o)
#define v_u32_find_first_v_vb(x, i, v, d, p, o)    v_u32_find_first_vb(x, ((v) | ((d) << 1)), i, to_bool64(p), o)
#define v_i16_find_first_v_vb(x, i, v, d, p, o)    v_i16_find_first_vb(x, ((v) | ((d) << 1)), i, to_bool128(p), o)
#define v_u16_find_first_v_vb(x, i, v, d, p, o)    v_u16_find_first_vb(x, ((v) | ((d) << 1)), i, to_bool128(p), o)
#define v_i8_find_first_v_vb(x, i, v, d, p, o)     v_i8_find_first_vb(x, ((v) | ((d) << 1)), i, p, o)
#define v_u8_find_first_v_vb(x, i, v, d, p, o)     v_u8_find_first_vb(x, ((v) | ((d) << 1)), i, p, o)

#define v_f32_find_first_v_b(x, i, v, d, p, o)     v_f32_find_first_b(x, ((v) | ((d) << 1)), i, p, o)
#define v_bf16_find_first_v_b(x, i, v, d, p, o)    v_bf16_find_first_b(x, ((v) | ((d) << 1)), i, p, o)
#define v_i32_find_first_v_b(x, i, v, d, p, o)     v_i32_find_first_b(x, ((v) | ((d) << 1)), i, p, o)
#define v_u32_find_first_v_b(x, i, v, d, p, o)     v_u32_find_first_b(x, ((v) | ((d) << 1)), i, p, o)
#define v_i16_find_first_v_b(x, i, v, d, p, o)     v_i16_find_first_b(x, ((v) | ((d) << 1)), i, p, o)
#define v_u16_find_first_v_b(x, i, v, d, p, o)     v_u16_find_first_b(x, ((v) | ((d) << 1)), i, p, o)
#define v_i8_find_first_v_b(x, i, v, d, p, o)      v_i8_find_first_b(x, ((v) | ((d) << 1)), i, p, o)
#define v_u8_find_first_v_b(x, i, v, d, p, o)      v_u8_find_first_b(x, ((v) | ((d) << 1)), i, p, o)

#define v_f32_find_first_v(x, v, d)                v_f32_find_first_v_b(x, 0, v, d, 1, 0)
#define v_bf16_find_first_v(x, v, d)               v_bf16_find_first_v_b(x, 0, v, d, 1, 0)
#define v_i32_find_first_v(x, v, d)                v_i32_find_first_v_b(x, 0, v, d, 1, 0)
#define v_u32_find_first_v(x, v, d)                v_u32_find_first_v_b(x, 0, v, d, 1, 0)
#define v_i16_find_first_v(x, v, d)                v_i16_find_first_v_b(x, 0, v, d, 1, 0)
#define v_u16_find_first_v(x, v, d)                v_u16_find_first_v_b(x, 0, v, d, 1, 0)
#define v_i8_find_first_v(x, v, d)                 v_i8_find_first_v_b(x, 0, v, d, 1, 0)
#define v_u8_find_first_v(x, v, d)                 v_u8_find_first_v_b(x, 0, v, d, 1, 0)


// NEARBYINT
#define s_f32_nearbyint_s_b(a, i, s, p, o)        s_f32_nearbyint(a, (s) << 16, i, p, o)
#define s_bf16_nearbyint_s_b(a, i, s, p, o)       s_bf16_nearbyint(a, (s) << 16, i, p, o)
#define s_f32_nearbyint_s(a, s)                   s_f32_nearbyint_s_b(a, 0, s, 1, 0)
#define s_bf16_nearbyint_s(a, s)                  s_bf16_nearbyint_s_b(a, 0, s, 1, 0)

#define v_f32_nearbyint_v_b(a, i, s, p, o)        v_f32_nearbyint_b(a, (s) << 16, i, p, o)
#define v_bf16_nearbyint_v_b(a, i, s, p, o)       v_bf16_nearbyint_b(a, (s) << 16, i, p, o)
#define v_f32_nearbyint_v_vb(a, i, s, p, o)       v_f32_nearbyint_vb(a, (s) << 16, i, to_bool64(p), o)
#define v_bf16_nearbyint_v_vb(a, i, s, p, o)      v_bf16_nearbyint_vb(a, (s) << 16, i, to_bool128(p), o)
#define v_f32_nearbyint_v(a, s)                   v_f32_nearbyint_v_b(a, 0, s, 1, 0)
#define v_bf16_nearbyint_v(a, s)                  v_bf16_nearbyint_v_b(a, 0, s, 1, 0)


// EXTRACT_EXP

#define s_f32_extract_exp_s_b(a, i, s, p, o)      s_f32_extract_exp(a, s, i, p, o)
#define s_bf16_extract_exp_s_b(a, i, s, p, o)     s_bf16_extract_exp(a, s, i, p, o)
#define s_f32_extract_exp_s(a, s)                 s_f32_extract_exp_s_b(a, 0, s, 1, 0)
#define s_bf16_extract_exp_s(a, s)                s_bf16_extract_exp_s_b(a, 0, s, 1, 0)

#define v_f32_extract_exp_v_vb(a, i, s, p, o)     v_f32_extract_exp_vb(a, s, i, to_bool64(p), o)
#define v_bf16_extract_exp_v_vb(a, i, s, p, o)    v_bf16_extract_exp_vb(a, s, i, to_bool128(p), o)
#define v_f32_extract_exp_v_b(a, i, s, p, o)      v_f32_extract_exp_b(a, s, i, p, o)
#define v_bf16_extract_exp_v_b(a, i, s, p, o)     v_bf16_extract_exp_b(a, s, i, p, o)

#define v_f32_extract_exp_s_vb(a, i, s, p, o)     v_f32_extract_exp_v_vb(a, i, s, p, o)
#define v_bf16_extract_exp_s_vb(a, i, s, p, o)    v_bf16_extract_exp_v_vb(a, i, s, p, o)
#define v_f32_extract_exp_s_b(a, i, s, p, o)      v_f32_extract_exp_v_b(a, i, s, p, o)
#define v_bf16_extract_exp_s_b(a, i, s, p, o)     v_bf16_extract_exp_v_b(a, i, s, p, o)
#define v_f32_extract_exp_v(a, s)                 v_f32_extract_exp_v_b(a, 0, s, 1, 0)
#define v_bf16_extract_exp_v(a, s)                v_bf16_extract_exp_v_b(a, 0, s, 1, 0)
#define v_f32_extract_exp_s(a, s)                 v_f32_extract_exp_v(a, s)
#define v_bf16_extract_exp_s(a, s)                v_bf16_extract_exp_v(a, s)


// BREV
#define s_f32_brev_s_b(a, i, p, o)           s_f32_brev(a, 0, i, p, o)
#define s_bf16_brev_s_b(a, i, p, o)          s_bf16_brev(a, 0, i, p, o)
#define s_i32_brev_s_b(a, i, p, o)           s_i32_brev(a, 0, i, p, o)
#define s_u32_brev_s_b(a, i, p, o)           s_u32_brev(a, 0, i, p, o)
#define s_i16_brev_s_b(a, i, p, o)           s_i16_brev(a, 0, i, p, o)
#define s_u16_brev_s_b(a, i, p, o)           s_u16_brev(a, 0, i, p, o)
#define s_i8_brev_s_b(a, i, p, o)            s_i8_brev(a, 0, i, p, o)
#define s_u8_brev_s_b(a, i, p, o)            s_u8_brev(a, 0, i, p, o)

#define s_f32_brev_s(a)                      s_f32_brev_s_b(a, 0, 1, 0)
#define s_bf16_brev_s(a)                     s_bf16_brev_s_b(a, 0, 1, 0)
#define s_i32_brev_s(a)                      s_i32_brev_s_b(a, 0, 1, 0)
#define s_u32_brev_s(a)                      s_u32_brev_s_b(a, 0, 1, 0)
#define s_i16_brev_s(a)                      s_i16_brev_s_b(a, 0, 1, 0)
#define s_u16_brev_s(a)                      s_u16_brev_s_b(a, 0, 1, 0)
#define s_i8_brev_s(a)                       s_i8_brev_s_b(a, 0, 1, 0)
#define s_u8_brev_s(a)                       s_u8_brev_s_b(a, 0, 1, 0)

#define v_f32_brev_v_b(a, i, p, o)           v_f32_brev_b(a, 0, i, p, o)
#define v_f32_brev_v_vb(a, i, p, o)          v_f32_brev_vb(a, 0, i, to_bool64(p), o)
#define v_bf16_brev_v_b(a, i, p, o)          v_bf16_brev_b(a, 0, i, p, o)
#define v_bf16_brev_v_vb(a, i, p, o)         v_bf16_brev_vb(a, 0, i, to_bool128(p), o)
#define v_i32_brev_v_b(a, i, p, o)           v_i32_brev_b(a, 0, i, p, o)
#define v_i32_brev_v_vb(a, i, p, o)          v_i32_brev_vb(a, 0, i, to_bool64(p), o)
#define v_u32_brev_v_b(a, i, p, o)           v_u32_brev_b(a, 0, i, p, o)
#define v_u32_brev_v_vb(a, i, p, o)          v_u32_brev_vb(a, 0, i, to_bool64(p), o)
#define v_i16_brev_v_b(a, i, p, o)           v_i16_brev_b(a, 0, i, p, o)
#define v_i16_brev_v_vb(a, i, p, o)          v_i16_brev_vb(a, 0, i, to_bool128(p), o)
#define v_u16_brev_v_b(a, i, p, o)           v_u16_brev_b(a, 0, i, p, o)
#define v_u16_brev_v_vb(a, i, p, o)          v_u16_brev_vb(a, 0, i, to_bool128(p), o)
#define v_i8_brev_v_b(a, i, p, o)            v_i8_brev_b(a, 0, i, p, o)
#define v_i8_brev_v_vb(a, i, p, o)           v_i8_brev_vb(a, 0, i, p, o)
#define v_u8_brev_v_b(a, i, p, o)            v_u8_brev_b(a, 0, i, p, o)
#define v_u8_brev_v_vb(a, i, p, o)           v_u8_brev_vb(a, 0, i, p, o)

#define v_f32_brev_s_b                       v_f32_brev_v_b
#define v_bf16_brev_s_b                      v_bf16_brev_v_b
#define v_i32_brev_s_b                       v_i32_brev_v_b
#define v_u32_brev_s_b                       v_u32_brev_v_b
#define v_i16_brev_s_b                       v_i16_brev_v_b
#define v_u16_brev_s_b                       v_u16_brev_v_b
#define v_i8_brev_s_b                        v_i8_brev_v_b
#define v_u8_brev_s_b                        v_u8_brev_v_b

#define v_f32_brev_s_vb                      v_f32_brev_v_vb
#define v_bf16_brev_s_vb                     v_bf16_brev_v_vb
#define v_i32_brev_s_vb                      v_i32_brev_v_vb
#define v_u32_brev_s_vb                      v_u32_brev_v_vb
#define v_i16_brev_s_vb                      v_i16_brev_v_vb
#define v_u16_brev_s_vb                      v_u16_brev_v_vb
#define v_i8_brev_s_vb                       v_i8_brev_v_vb
#define v_u8_brev_s_vb                       v_u8_brev_v_vb

#define v_f32_brev_v(a)                      v_f32_brev_v_b(a, 0, 1, 0)
#define v_bf16_brev_v(a)                     v_bf16_brev_v_b(a, 0, 1, 0)
#define v_i32_brev_v(a)                      v_i32_brev_v_b(a, 0, 1, 0)
#define v_u32_brev_v(a)                      v_u32_brev_v_b(a, 0, 1, 0)
#define v_i16_brev_v(a)                      v_i16_brev_v_b(a, 0, 1, 0)
#define v_u16_brev_v(a)                      v_u16_brev_v_b(a, 0, 1, 0)
#define v_i8_brev_v(a)                       v_i8_brev_v_b(a, 0, 1, 0)
#define v_u8_brev_v(a)                       v_u8_brev_v_b(a, 0, 1, 0)

#define v_f32_brev_s                         v_f32_brev_v
#define v_bf16_brev_s                        v_bf16_brev_v
#define v_i32_brev_s                         v_i32_brev_v
#define v_u32_brev_s                         v_u32_brev_v
#define v_i16_brev_s                         v_i16_brev_v
#define v_u16_brev_s                         v_u16_brev_v
#define v_i8_brev_s                          v_i8_brev_v
#define v_u8_brev_s                          v_u8_brev_v


// FCLASS
#define s_f32_fclass_s_b(a, i, p, o)        s_f32_fclass(a, 0, i, p, o)
#define s_bf16_fclass_s_b(a, i, p, o)       s_bf16_fclass(a, 0, i, p, o)
#define v_f32_fclass_v_b(a, i, p, o)        v_f32_fclass_b(a, 0, i, p, o)
#define v_bf16_fclass_v_b(a, i, p, o)       v_bf16_fclass_b(a, 0, i, p, o)
#define v_f32_fclass_v_vb(a, i, p, o)       v_f32_fclass_vb(a, 0, i, to_bool64(p), o)
#define v_bf16_fclass_v_vb(a, i, p, o)      v_bf16_fclass_vb(a, 0, i, to_bool128(p), o)


#define v_f32_fclass_s_b                    v_f32_fclass_v_b
#define v_bf16_fclass_s_b                   v_bf16_fclass_v_b
                                       
#define v_f32_fclass_s_vb                   v_f32_fclass_v_vb
#define v_bf16_fclass_s_vb                  v_bf16_fclass_v_vb

#define s_f32_fclass_s(a)                   s_f32_fclass_s_b(a, 0, 1, 0)
#define s_bf16_fclass_s(a)                  s_bf16_fclass_s_b(a, 0, 1, 0)
#define v_f32_fclass_v(a)                   v_f32_fclass_v_b(a, 0, 1, 0)
#define v_bf16_fclass_v(a)                  v_bf16_fclass_v_b(a, 0, 1, 0)


#define v_f32_fclass_s                      v_f32_fclass_v
#define v_bf16_fclass_s                     v_bf16_fclass_v


// SHUFFLE
#define v_f32_shuffle_v_v_b(a, b, i, p, o)    v_f32_shuffle_b(a, b, 0, i, p, o)
#define v_f32_shuffle_v_v_vb(a, b, i, p, o)   v_f32_shuffle_vb(a, b, 0, i, to_bool64(p), o)
#define v_bf16_shuffle_v_v_b(a, b, i, p, o)   v_bf16_shuffle_b(a, b, 0, i, p, o)
#define v_bf16_shuffle_v_v_vb(a, b, i, p, o)  v_bf16_shuffle_vb(a, b, 0, i, to_bool128(p), o)
#define v_i32_shuffle_v_v_b(a, b, i, p, o)    v_i32_shuffle_b(a, b, 0, i, p, o)
#define v_i32_shuffle_v_v_vb(a, b, i, p, o)   v_i32_shuffle_vb(a, b, 0, i, to_bool64(p), o)
#define v_u32_shuffle_v_v_b(a, b, i, p, o)    v_u32_shuffle_b(a, b, 0, i, p, o)
#define v_u32_shuffle_v_v_vb(a, b, i, p, o)   v_u32_shuffle_vb(a, b, 0, i, to_bool64(p), o)
#define v_i16_shuffle_v_v_b(a, b, i, p, o)    v_i16_shuffle_b(a, b, 0, i, p, o)
#define v_i16_shuffle_v_v_vb(a, b, i, p, o)   v_i16_shuffle_vb(a, b, 0, i, to_bool128(p), o)
#define v_u16_shuffle_v_v_b(a, b, i, p, o)    v_u16_shuffle_b(a, b, 0, i, p, o)
#define v_u16_shuffle_v_v_vb(a, b, i, p, o)   v_u16_shuffle_vb(a, b, 0, i, to_bool128(p), o)
#define v_i8_shuffle_v_v_b(a, b, i, p, o)     v_i8_shuffle_b(a, b, 0, i, p, o)
#define v_i8_shuffle_v_v_vb(a, b, i, p, o)    v_i8_shuffle_vb(a, b, 0, i, p, o)
#define v_u8_shuffle_v_v_b(a, b, i, p, o)     v_u8_shuffle_b(a, b, 0, i, p, o)
#define v_u8_shuffle_v_v_vb(a, b, i, p, o)    v_u8_shuffle_vb(a, b, 0, i, p, o)

#define v_f32_shuffle_v_v(a, b)               v_f32_shuffle_v_v_b(a, b, a, 1, 0)
#define v_bf16_shuffle_v_v(a, b)              v_bf16_shuffle_v_v_b(a, b, a, 1, 0)
#define v_i32_shuffle_v_v(a, b)               v_i32_shuffle_v_v_b(a, b, a, 1, 0)
#define v_u32_shuffle_v_v(a, b)               v_u32_shuffle_v_v_b(a, b, a, 1, 0)
#define v_i16_shuffle_v_v(a, b)               v_i16_shuffle_v_v_b(a, b, a, 1, 0)
#define v_u16_shuffle_v_v(a, b)               v_u16_shuffle_v_v_b(a, b, a, 1, 0)
#define v_i8_shuffle_v_v(a, b)                v_i8_shuffle_v_v_b(a, b, a, 1, 0)
#define v_u8_shuffle_v_v(a, b)                v_u8_shuffle_v_v_b(a, b, a, 1, 0)


// PACK
#define v_bf16_pack_v_b(a, i, sg, es, p, o)   v_bf16_pack_b(a, ((sg) << 8) | ((es) << 9), i, p, o)
#define v_bf16_pack_v_vb(a, i, sg, es, p, o)  v_bf16_pack_vb(a, ((sg) << 8) | ((es) << 9), i, to_bool128(p), o)
#define v_i16_pack_v_b(a, i, sg, es, p, o)    v_i16_pack_b(a, ((sg) << 8) | ((es) << 9), i, p, o)
#define v_i16_pack_v_vb(a, i, sg, es, p, o)   v_i16_pack_vb(a, ((sg) << 8) | ((es) << 9), i, to_bool128(p), o)
#define v_u16_pack_v_b(a, i, sg, es, p, o)    v_u16_pack_b(a, ((sg) << 8) | ((es) << 9), i, p, o)
#define v_u16_pack_v_vb(a, i, sg, es, p, o)   v_u16_pack_vb(a, ((sg) << 8) | ((es) << 9), i, to_bool128(p), o)
#define v_i8_pack_v_b(a, i, sg, es, p, o)     v_i8_pack_b(a, ((sg) << 8) | ((es) << 9), i, p, o)
#define v_i8_pack_v_vb(a, i, sg, es, p, o)    v_i8_pack_vb(a, ((sg) << 8) | ((es) << 9), i, p, o)
#define v_u8_pack_v_b(a, i, sg, es, p, o)     v_u8_pack_b(a, ((sg) << 8) | ((es) << 9), i, p, o)
#define v_u8_pack_v_vb(a, i, sg, es, p, o)    v_u8_pack_vb(a, ((sg) << 8) | ((es) << 9), i, p, o)

#define v_bf16_pack_v(a, i, sg, es)           v_bf16_pack_v_b(a, i, sg, es, 1, 0)
#define v_i16_pack_v(a, i, sg, es)            v_i16_pack_v_b(a, i, sg, es, 1, 0)
#define v_u16_pack_v(a, i, sg, es)            v_u16_pack_v_b(a, i, sg, es, 1, 0)
#define v_i8_pack_v(a, i, sg, es)             v_i8_pack_v_b(a, i, sg, es, 1, 0)
#define v_u8_pack_v(a, i, sg, es)             v_u8_pack_v_b(a, i, sg, es, 1, 0)


// UNPACK
#define v_bf16_unpack_v_b(a, i, sg, es, gh, p, o)   v_bf16_unpack_b(a, ((sg) << 8) | ((es) << 9) | ((gh) << 10), i, p, o)
#define v_bf16_unpack_v_vb(a, i, sg, es, gh, p, o)  v_bf16_unpack_vb(a, ((sg) << 8) | ((es) << 9) | ((gh) << 10), i, to_bool128(p), o)
#define v_i16_unpack_v_b(a, i, sg, es, gh, p, o)    v_i16_unpack_b(a, ((sg) << 8) | ((es) << 9) | ((gh) << 10), i, p, o)
#define v_i16_unpack_v_vb(a, i, sg, es, gh, p, o)   v_i16_unpack_vb(a, ((sg) << 8) | ((es) << 9) | ((gh) << 10), i, to_bool128(p), o)
#define v_u16_unpack_v_b(a, i, sg, es, gh, p, o)    v_u16_unpack_b(a, ((sg) << 8) | ((es) << 9) | ((gh) << 10), i, p, o)
#define v_u16_unpack_v_vb(a, i, sg, es, gh, p, o)   v_u16_unpack_vb(a, ((sg) << 8) | ((es) << 9) | ((gh) << 10), i, to_bool128(p), o)
#define v_i8_unpack_v_b(a, i, sg, es, gh, p, o)     v_i8_unpack_b(a, ((sg) << 8) | ((es) << 9) | ((gh) << 10), i, p, o)
#define v_i8_unpack_v_vb(a, i, sg, es, gh, p, o)    v_i8_unpack_vb(a, ((sg) << 8) | ((es) << 9) | ((gh) << 10), i, p, o)
#define v_u8_unpack_v_b(a, i, sg, es, gh, p, o)     v_u8_unpack_b(a, ((sg) << 8) | ((es) << 9) | ((gh) << 10), i, p, o)
#define v_u8_unpack_v_vb(a, i, sg, es, gh, p, o)    v_u8_unpack_vb(a, ((sg) << 8) | ((es) << 9) | ((gh) << 10), i, p, o)

#define v_bf16_unpack_v(a, i, sg, es, gh)           v_bf16_unpack_v_b(a, i, sg, es, gh, 1, 0)
#define v_i16_unpack_v(a, i, sg, es, gh)            v_i16_unpack_v_b(a, i, sg, es, gh, 1, 0)
#define v_u16_unpack_v(a, i, sg, es, gh)            v_u16_unpack_v_b(a, i, sg, es, gh, 1, 0)
#define v_i8_unpack_v(a, i, sg, es, gh)             v_i8_unpack_v_b(a, i, sg, es, gh, 1, 0)
#define v_u8_unpack_v(a, i, sg, es, gh)             v_u8_unpack_v_b(a, i, sg, es, gh, 1, 0)


// GET_LUT_ENTRY_AND_INTERVAL_START
#define v_f32_get_lut_entry_and_interval_start_v_b(a, i, sh, v, p, o)     v_f32_get_lut_entry_and_interval_start_b(a, sh, (v) << 13, i, p, o)
#define v_f32_get_lut_entry_and_interval_start_v_vb(a, i, sh, v, p, o)    v_f32_get_lut_entry_and_interval_start_vb(a, sh, (v) << 13, i, to_bool64(p), o)
#define v_bf16_get_lut_entry_and_interval_start_v_b(a, i, sh, v, p, o)    v_bf16_get_lut_entry_and_interval_start_b(a, sh, (v) << 13, i, p, o)
#define v_bf16_get_lut_entry_and_interval_start_v_vb(a, i, sh, v, p, o)   v_bf16_get_lut_entry_and_interval_start_vb(a, sh, (v) << 13, i, to_bool128(p), o)

#define v_f32_get_lut_entry_and_interval_start_v(a, sh, v)     v_f32_get_lut_entry_and_interval_start_v_b(a, (uint64_float64_pair_t){0}, sh, v, 1, 0)
#define v_bf16_get_lut_entry_and_interval_start_v(a, sh, v)    v_bf16_get_lut_entry_and_interval_start_v_b(a, (ushort128_bfloat128_pair_t){0}, sh, v, 1, 0)


// FORM_FP_NUMMBER
#define v_f32_form_fp_num_v_v_v_b(a, b, c, i, s, p, o)        v_f32_form_fp_num_b(a, b, c, s, i, p, o)
#define v_f32_form_fp_num_v_v_v_vb(a, b, c, i, s, p, o)       v_f32_form_fp_num_vb(a, b, c, s, i, to_bool64(p), o)
#define v_bf16_form_fp_num_v_v_v_b(a, b, c, i, s, p, o)       v_bf16_form_fp_num_b(a, b, c, s, i, p, o)
#define v_bf16_form_fp_num_v_v_v_vb(a, b, c, i, s, p, o)      v_bf16_form_fp_num_vb(a, b, c, s, i, to_bool128(p), o)

#define v_f32_form_fp_num_i8_v_v_v_b(a, b, c, i, s, p, o)     v_f32_form_fp_num_ie_b(a, b, c, s, i, p, o)
#define v_f32_form_fp_num_i8_v_v_v_vb(a, b, c, i, s, p, o)    v_f32_form_fp_num_ie_vb(a, b, c, s, i, to_bool64(p), o)
#define v_bf16_form_fp_num_i8_v_v_v_b(a, b, c, i, s, p, o)    v_bf16_form_fp_num_ie_b(a, b, c, s, i, p, o)
#define v_bf16_form_fp_num_i8_v_v_v_vb(a, b, c, i, s, p, o)   v_bf16_form_fp_num_ie_vb(a, b, c, s, i, to_bool128(p), o)

#define v_f32_form_fp_num_s_v_v_b                             v_f32_form_fp_num_v_v_v_b
#define v_f32_form_fp_num_s_v_v_vb                            v_f32_form_fp_num_v_v_v_vb
#define v_bf16_form_fp_num_s_v_v_b                            v_bf16_form_fp_num_v_v_v_b
#define v_bf16_form_fp_num_s_v_v_vb                           v_bf16_form_fp_num_v_v_v_vb


#define v_f32_form_fp_num_i8_s_v_v_b                          v_f32_form_fp_num_i8_v_v_v_b
#define v_f32_form_fp_num_i8_s_v_v_vb                         v_f32_form_fp_num_i8_v_v_v_vb
#define v_bf16_form_fp_num_i8_s_v_v_b                         v_bf16_form_fp_num_i8_v_v_v_b
#define v_bf16_form_fp_num_i8_s_v_v_vb                        v_bf16_form_fp_num_i8_v_v_v_vb


#define v_f32_form_fp_num_v_v_v(a, b, c, s)                   v_f32_form_fp_num_v_v_v_b(a, b, c, 0, s, 1, 0)
#define v_bf16_form_fp_num_v_v_v(a, b, c, s)                  v_bf16_form_fp_num_v_v_v_b(a, b, c, 0, s, 1, 0)
#define v_f32_form_fp_num_i8_v_v_v(a, b, c, s)                v_f32_form_fp_num_i8_v_v_v_b(a, b, c, 0, s, 1, 0)
#define v_bf16_form_fp_num_i8_v_v_v(a, b, c, s)               v_bf16_form_fp_num_i8_v_v_v_b(a, b, c, 0, s, 1, 0)
#define v_f32_form_fp_num_s_v_v(a, b, c, s)                   v_f32_form_fp_num_s_v_v_b(a, b, c, 0, s, 1, 0)
#define v_bf16_form_fp_num_s_v_v(a, b, c, s)                  v_bf16_form_fp_num_s_v_v_b(a, b, c, 0, s, 1, 0)
#define v_f32_form_fp_num_i8_s_v_v(a, b, c, s)                v_f32_form_fp_num_i8_s_v_v_b(a, b, c, 0, s, 1, 0)
#define v_bf16_form_fp_num_i8_s_v_v(a, b, c, s)               v_bf16_form_fp_num_i8_s_v_v_b(a, b, c, 0, s, 1, 0)


#define s_i32_mov_irf_dim_i_b(s, i, d, p, o)      mov_irf_dim(s, d, 0, i, p, o)
#define s_i32_mov_irf_dim_i(s, d)                 s_i32_mov_irf_dim_i_b(s, 0, d, 1, 0)


#define s_f32_calc_fp_special_s_s_b(s1, s2, i, f, p, o)   s_f32_calc_fp_special(s1, s2, f, i, p, o)
#define s_f32_calc_fp_special_s_b(s, i, f, p, o)          s_f32_calc_fp_special_s_s_b(s, s, i, f, p, o)
#define s_f32_calc_fp_special_s(s, i, f)                  s_f32_calc_fp_special_s_b(s, i, f, 1, 0)
#define s_f32_calc_fp_special_s_s(s1, s2, i, f)           s_f32_calc_fp_special_s_s_b(s1, s2, i, f, 1, 0)
#define s_bf16_calc_fp_special_s_s_b(s1, s2, i, f, p, o)  s_bf16_calc_fp_special(s1, s2, f, i, p, o)
#define s_bf16_calc_fp_special_s_b(s, i, f, p, o)         s_bf16_calc_fp_special_s_s_b(s, s, i, f, p, o)
#define s_bf16_calc_fp_special_s(s, i, f)                 s_bf16_calc_fp_special_s_b(s, i, f, 1, 0)
#define s_bf16_calc_fp_special_s_s(s1, s2, i, f)          s_bf16_calc_fp_special_s_s_b(s1, s2, i, f, 1, 0)

#define v_f32_calc_fp_special_v_v_b(v1, v2, i, f, p, o)   v_f32_calc_fp_special_b(v1, v2, f, i, p, o)
#define v_f32_calc_fp_special_v_v_vb(v1, v2, i, f, p, o)  v_f32_calc_fp_special_vb(v1, v2, f, i, to_bool64(p), o)
#define v_f32_calc_fp_special_v_b(v, i, f, p, o)          v_f32_calc_fp_special_v_v_b(v, v, i, f, p, o)
#define v_f32_calc_fp_special_v_vb(v, i, f, p, o)         v_f32_calc_fp_special_v_v_vb(v, v, i, f, p, o)
#define v_f32_calc_fp_special_v_v(v1, v2, i, f)           v_f32_calc_fp_special_v_v_b(v1, v2, i, f, 1, 0)
#define v_f32_calc_fp_special_v(v, i, f)                  v_f32_calc_fp_special_v_b(v, i, f, 1, 0)
#define v_bf16_calc_fp_special_v_v_b(v1, v2, i, f, p, o)  v_bf16_calc_fp_special_b(v1, v2, f, i, p, o)
#define v_bf16_calc_fp_special_v_v_vb(v1, v2, i, f, p, o) v_bf16_calc_fp_special_vb(v1, v2, f, i, to_bool128(p), o)
#define v_bf16_calc_fp_special_v_b(v, i, f, p, o)         v_bf16_calc_fp_special_v_v_b(v, v, i, f, p, o)
#define v_bf16_calc_fp_special_v_vb(v, i, f, p, o)        v_bf16_calc_fp_special_v_v_vb(v, v, i, f, p, o)
#define v_bf16_calc_fp_special_v_v(v1, v2, i, f)          v_bf16_calc_fp_special_v_v_b(v1, v2, i, f, 1, 0)
#define v_bf16_calc_fp_special_v(v, i, f)                 v_bf16_calc_fp_special_v_b(v, i, f, 1, 0)



// ABS
#define s_f32_abs_s_b(a, i, p, o)         s_f32_abs(a, 0, i, p, o)
#define s_bf16_abs_s_b(a, i, p, o)        s_bf16_abs(a, 0, i, p, o)
#define s_i32_abs_s_b(a, i, p, o)         s_i32_abs(a, 0, i, p, o)
#define s_i16_abs_s_b(a, i, p, o)         s_i16_abs(a, 0, i, p, o)
#define s_i8_abs_s_b(a, i, p, o)          s_i8_abs(a, 0, i, p, o)

#define s_f32_abs_s                       s_f32_abs
#define s_bf16_abs_s                      s_bf16_abs
#define s_i32_abs_s                       s_i32_abs
#define s_i16_abs_s                       s_i16_abs
#define s_i8_abs_s                        s_i8_abs

#define i_i32_abs_i_b(a, i, m, p, o)      i_i32_abs(a, m, 0, i, p, o)
#define i_i32_abs_s_b                     i_i32_abs_i_b
#define i_i32_abs_i(a, i, m)              i_i32_abs_i_b(a, i, m, 1, 0)
#define i_i32_abs_s                       i_i32_abs_i

#define v_f32_abs_v_vb(a, i, p, o)        v_f32_abs_vb(a, 0, i, to_bool64(p), o)
#define v_f32_abs_v_b(a, i, p, o)         v_f32_abs_b(a, 0, i, p, o)
#define v_bf16_abs_v_vb(a, i, p, o)       v_bf16_abs_vb(a, 0, i, to_bool128(p), o)
#define v_bf16_abs_v_b(a, i, p, o)        v_bf16_abs_b(a, 0, i, p, o)
#define v_i32_abs_v_vb(a, i, p, o)        v_i32_abs_vb(a, 0, i, to_bool64(p), o)
#define v_i32_abs_v_b(a, i, p, o)         v_i32_abs_b(a, 0, i, p, o)
#define v_i16_abs_v_vb(a, i, p, o)        v_i16_abs_vb(a, 0, i, to_bool128(p), o)
#define v_i16_abs_v_b(a, i, p, o)         v_i16_abs_b(a, 0, i, p, o)
#define v_i8_abs_v_vb(a, i, p, o)         v_i8_abs_vb(a, 0, i, p, o)
#define v_i8_abs_v_b(a, i, p, o)          v_i8_abs_b(a, 0, i, p, o)

#define v_f32_abs_v                       v_f32_abs_b
#define v_bf16_abs_v                      v_bf16_abs_b
#define v_i32_abs_v                       v_i32_abs_b
#define v_i16_abs_v                       v_i16_abs_b
#define v_i8_abs_v                        v_i8_abs_b


// NOT
#define s_f32_not_s_b(a, i, p, o)         s_f32_not(a, 0, i, p, o)
#define s_bf16_not_s_b(a, i, p, o)        s_bf16_not(a, 0, i, p, o)
#define s_i32_not_s_b(a, i, p, o)         s_i32_not(a, 0, i, p, o)
#define s_u32_not_s_b(a, i, p, o)         s_u32_not(a, 0, i, p, o)
#define s_i16_not_s_b(a, i, p, o)         s_i16_not(a, 0, i, p, o)
#define s_u16_not_s_b(a, i, p, o)         s_u16_not(a, 0, i, p, o)
#define s_i8_not_s_b(a, i, p, o)          s_i8_not(a, 0, i, p, o)
#define s_u8_not_s_b(a, i, p, o)          s_u8_not(a, 0, i, p, o)
#define b_b_not_b_b(a, i, p, o)           s_i1_not(a, 0, i, p, o)

#define s_f32_not_s(a)                    s_f32_not_s_b(a, 0, 1, 0)
#define s_bf16_not_s(a)                   s_bf16_not_s_b(a, 0, 1, 0)
#define s_i32_not_s(a)                    s_i32_not_s_b(a, 0, 1, 0)
#define s_u32_not_s(a)                    s_u32_not_s_b(a, 0, 1, 0)
#define s_i16_not_s(a)                    s_i16_not_s_b(a, 0, 1, 0)
#define s_u16_not_s(a)                    s_u16_not_s_b(a, 0, 1, 0)
#define s_i8_not_s(a)                     s_i8_not_s_b(a, 0, 1, 0)
#define s_u8_not_s(a)                     s_u8_not_s_b(a, 0, 1, 0)
#define b_b_not_b(a)                      b_b_not_b_b(a, 0, 1, 0)

#define v_f32_not_v_vb(a, i, p, o)        v_f32_not_vb(a, 0, i, to_bool64(p), o)
#define v_f32_not_v_b(a, i, p, o)         v_f32_not_b(a, 0, i, p, o)
#define v_bf16_not_v_vb(a, i, p, o)       v_bf16_not_vb(a, 0, i, to_bool128(p), o)
#define v_bf16_not_v_b(a, i, p, o)        v_bf16_not_b(a, 0, i, p, o)
#define v_i32_not_v_vb(a, i, p, o)        v_i32_not_vb(a, 0, i, to_bool64(p), o)
#define v_i32_not_v_b(a, i, p, o)         v_i32_not_b(a, 0, i, p, o)
#define v_u32_not_v_vb(a, i, p, o)        v_u32_not_vb(a, 0, i, to_bool64(p), o)
#define v_u32_not_v_b(a, i, p, o)         v_u32_not_b(a, 0, i, p, o)
#define v_i16_not_v_vb(a, i, p, o)        v_i16_not_vb(a, 0, i, to_bool128(p), o)
#define v_i16_not_v_b(a, i, p, o)         v_i16_not_b(a, 0, i, p, o)
#define v_u16_not_v_vb(a, i, p, o)        v_u16_not_vb(a, 0, i, to_bool128(p), o)
#define v_u16_not_v_b(a, i, p, o)         v_u16_not_b(a, 0, i, p, o)
#define v_i8_not_v_vb(a, i, p, o)         v_i8_not_vb(a, 0, i, p, o)
#define v_i8_not_v_b(a, i, p, o)          v_i8_not_b(a, 0, i, p, o)
#define v_u8_not_v_vb(a, i, p, o)         v_u8_not_vb(a, 0, i, p, o)
#define v_u8_not_v_b(a, i, p, o)          v_u8_not_b(a, 0, i, p, o)
#define bv_b_not_bv_vb(a, i, p, o)        v_i1_not_vb(a, 0, i, p, o)
#define bv_b_not_bv_b(a, i, p, o)         v_i1_not_b(a, 0, i, p, o)

#define v_f32_not_v(a)                    v_f32_not_v_b(a, 0, 1, 0)
#define v_bf16_not_v(a)                   v_bf16_not_v_b(a, 0, 1, 0)
#define v_i32_not_v(a)                    v_i32_not_v_b(a, 0, 1, 0)
#define v_u32_not_v(a)                    v_u32_not_v_b(a, 0, 1, 0)
#define v_i16_not_v(a)                    v_i16_not_v_b(a, 0, 1, 0)
#define v_u16_not_v(a)                    v_u16_not_v_b(a, 0, 1, 0)
#define v_i8_not_v(a)                     v_i8_not_v_b(a, 0, 1, 0)
#define v_u8_not_v(a)                     v_u8_not_v_b(a, 0, 1, 0)
#define bv_b_not_bv(a)                    bv_b_not_bv_b(a, (bool256){0}, 1, 0)

#define i_i32_not_i_b(a, i, m, p, o)      i_i32_not(a, m, 0, i, p, o)
#define i_i32_not_s_b                     i_i32_not_i_b
#define i_i32_not_i(a, i, m)              i_i32_not_i_b(a, i, m, 1, 0)
#define i_i32_not_s                       i_i32_not_i


// SHR
#define s_f32_shr_s_s_b(a, b, i, p, o)    s_f32_shr(a, b, 0, i, p, o)
#define s_bf16_shr_s_s_b(a, b, i, p, o)   s_bf16_shr(a, b, 0, i, p, o)
#define s_i32_shr_s_s_b(a, b, i, p, o)    s_i32_shr(a, b, 0, i, p, o)
#define s_u32_shr_s_s_b(a, b, i, p, o)    s_u32_shr(a, b, 0, i, p, o)
#define s_i16_shr_s_s_b(a, b, i, p, o)    s_i16_shr(a, b, 0, i, p, o)
#define s_u16_shr_s_s_b(a, b, i, p, o)    s_u16_shr(a, b, 0, i, p, o)
#define s_i8_shr_s_s_b(a, b, i, p, o)     s_i8_shr(a, b, 0, i, p, o)
#define s_u8_shr_s_s_b(a, b, i, p, o)     s_u8_shr(a, b, 0, i, p, o)

#define s_f32_shr_s_s(a, b)               s_f32_shr_s_s_b(a, b, 0, 1, 0)
#define s_bf16_shr_s_s(a, b)              s_bf16_shr_s_s_b(a, b, 0, 1, 0)
#define s_i32_shr_s_s(a, b)               s_i32_shr_s_s_b(a, b, 0, 1, 0)
#define s_u32_shr_s_s(a, b)               s_u32_shr_s_s_b(a, b, 0, 1, 0)
#define s_i16_shr_s_s(a, b)               s_i16_shr_s_s_b(a, b, 0, 1, 0)
#define s_u16_shr_s_s(a, b)               s_u16_shr_s_s_b(a, b, 0, 1, 0)
#define s_i8_shr_s_s(a, b)                s_i8_shr_s_s_b(a, b, 0, 1, 0)
#define s_u8_shr_s_s(a, b)                s_u8_shr_s_s_b(a, b, 0, 1, 0)

#define i_i32_shr_i_i_b(a, b, i, m, p, o) i_i32_shr(a, b, m, 0, i, p, o)
#define i_i32_shr_i_s_b                   i_i32_shr_i_i_b
#define i_i32_shr_i_i(a, b, i, m)         i_i32_shr_i_i_b(a, b, i, m, 1, 0)
#define i_i32_shr_i_s                     i_i32_shr_i_i

#define v_f32_shr_v_v_vb(a, b, i, p, o)   v_f32_shr_vb(a, b, 0, i, to_bool64(p), o)
#define v_f32_shr_v_v_b(a, b, i, p, o)    v_f32_shr_b(a, b, 0, i, p, o)
#define v_bf16_shr_v_v_vb(a, b, i, p, o)  v_bf16_shr_vb(a, b, 0, i, to_bool128(p), o)
#define v_bf16_shr_v_v_b(a, b, i, p, o)   v_bf16_shr_b(a, b, 0, i, p, o)
#define v_i32_shr_v_v_vb(a, b, i, p, o)   v_i32_shr_vb(a, b, 0, i, to_bool64(p), o)
#define v_i32_shr_v_v_b(a, b, i, p, o)    v_i32_shr_b(a, b, 0, i, p, o)
#define v_u32_shr_v_v_vb(a, b, i, p, o)   v_u32_shr_vb(a, b, 0, i, to_bool64(p), o)
#define v_u32_shr_v_v_b(a, b, i, p, o)    v_u32_shr_b(a, b, 0, i, p, o)
#define v_i16_shr_v_v_vb(a, b, i, p, o)   v_i16_shr_vb(a, b, 0, i, to_bool128(p), o)
#define v_i16_shr_v_v_b(a, b, i, p, o)    v_i16_shr_b(a, b, 0, i, p, o)
#define v_u16_shr_v_v_vb(a, b, i, p, o)   v_u16_shr_vb(a, b, 0, i, to_bool128(p), o)
#define v_u16_shr_v_v_b(a, b, i, p, o)    v_u16_shr_b(a, b, 0, i, p, o)
#define v_i8_shr_v_v_vb(a, b, i, p, o)    v_i8_shr_vb(a, b, 0, i, p, o)
#define v_i8_shr_v_v_b(a, b, i, p, o)     v_i8_shr_b(a, b, 0, i, p, o)
#define v_u8_shr_v_v_vb(a, b, i, p, o)    v_u8_shr_vb(a, b, 0, i, p, o)
#define v_u8_shr_v_v_b(a, b, i, p, o)     v_u8_shr_b(a, b, 0, i, p, o)

#define v_f32_shr_v_s_vb                  v_f32_shr_v_v_vb
#define v_f32_shr_v_s_b                   v_f32_shr_v_v_b
#define v_bf16_shr_v_s_vb                 v_bf16_shr_v_v_vb
#define v_bf16_shr_v_s_b                  v_bf16_shr_v_v_b
#define v_i32_shr_v_s_vb                  v_i32_shr_v_v_vb
#define v_i32_shr_v_s_b                   v_i32_shr_v_v_b
#define v_u32_shr_v_s_vb                  v_u32_shr_v_v_vb
#define v_u32_shr_v_s_b                   v_u32_shr_v_v_b
#define v_i16_shr_v_s_vb                  v_i16_shr_v_v_vb
#define v_i16_shr_v_s_b                   v_i16_shr_v_v_b
#define v_u16_shr_v_s_vb                  v_u16_shr_v_v_vb
#define v_u16_shr_v_s_b                   v_u16_shr_v_v_b
#define v_i8_shr_v_s_vb                   v_i8_shr_v_v_vb
#define v_i8_shr_v_s_b                    v_i8_shr_v_v_b
#define v_u8_shr_v_s_vb                   v_u8_shr_v_v_vb
#define v_u8_shr_v_s_b                    v_u8_shr_v_v_b

#define v_f32_shr_v_v(a, b)               v_f32_shr_v_v_b(a, b, 0, 1, 0)
#define v_bf16_shr_v_v(a, b)              v_bf16_shr_v_v_b(a, b, 0, 1, 0)
#define v_i32_shr_v_v(a, b)               v_i32_shr_v_v_b(a, b, 0, 1, 0)
#define v_u32_shr_v_v(a, b)               v_u32_shr_v_v_b(a, b, 0, 1, 0)
#define v_i16_shr_v_v(a, b)               v_i16_shr_v_v_b(a, b, 0, 1, 0)
#define v_u16_shr_v_v(a, b)               v_u16_shr_v_v_b(a, b, 0, 1, 0)
#define v_i8_shr_v_v(a, b)                v_i8_shr_v_v_b(a, b, 0, 1, 0)
#define v_u8_shr_v_v(a, b)                v_u8_shr_v_v_b(a, b, 0, 1, 0)

#define v_f32_shr_v_s                     v_f32_shr_v_v
#define v_bf16_shr_v_s                    v_bf16_shr_v_v
#define v_i32_shr_v_s                     v_i32_shr_v_v
#define v_u32_shr_v_s                     v_u32_shr_v_v
#define v_i16_shr_v_s                     v_i16_shr_v_v
#define v_u16_shr_v_s                     v_u16_shr_v_v
#define v_i8_shr_v_s                      v_i8_shr_v_v
#define v_u8_shr_v_s                      v_u8_shr_v_v


// SHL
#define s_f32_shl_s_s_b(a, b, i, p, o)    s_f32_shl(a, b, 0, i, p, o)
#define s_bf16_shl_s_s_b(a, b, i, p, o)   s_bf16_shl(a, b, 0, i, p, o)
#define s_i32_shl_s_s_b(a, b, i, p, o)    s_i32_shl(a, b, 0, i, p, o)
#define s_u32_shl_s_s_b(a, b, i, p, o)    s_u32_shl(a, b, 0, i, p, o)
#define s_i16_shl_s_s_b(a, b, i, p, o)    s_i16_shl(a, b, 0, i, p, o)
#define s_u16_shl_s_s_b(a, b, i, p, o)    s_u16_shl(a, b, 0, i, p, o)
#define s_i8_shl_s_s_b(a, b, i, p, o)     s_i8_shl(a, b, 0, i, p, o)
#define s_u8_shl_s_s_b(a, b, i, p, o)     s_u8_shl(a, b, 0, i, p, o)

#define s_f32_shl_s_s(a, b)               s_f32_shl_s_s_b(a, b, 0, 1, 0)
#define s_bf16_shl_s_s(a, b)              s_bf16_shl_s_s_b(a, b, 0, 1, 0)
#define s_i32_shl_s_s(a, b)               s_i32_shl_s_s_b(a, b, 0, 1, 0)
#define s_u32_shl_s_s(a, b)               s_u32_shl_s_s_b(a, b, 0, 1, 0)
#define s_i16_shl_s_s(a, b)               s_i16_shl_s_s_b(a, b, 0, 1, 0)
#define s_u16_shl_s_s(a, b)               s_u16_shl_s_s_b(a, b, 0, 1, 0)
#define s_i8_shl_s_s(a, b)                s_i8_shl_s_s_b(a, b, 0, 1, 0)
#define s_u8_shl_s_s(a, b)                s_u8_shl_s_s_b(a, b, 0, 1, 0)

#define i_i32_shl_i_i_b(a, b, i, m, p, o) i_i32_shl(a, b, m, 0, i, p, o)
#define i_i32_shl_i_s_b                   i_i32_shl_i_i_b
#define i_i32_shl_i_i(a, b, i, m)         i_i32_shl_i_i_b(a, b, i, m, 1, 0)
#define i_i32_shl_i_s                     i_i32_shl_i_i

#define v_f32_shl_v_v_vb(a, b, i, p, o)   v_f32_shl_vb(a, b, 0, i, to_bool64(p), o)
#define v_f32_shl_v_v_b(a, b, i, p, o)    v_f32_shl_b(a, b, 0, i, p, o)
#define v_bf16_shl_v_v_vb(a, b, i, p, o)  v_bf16_shl_vb(a, b, 0, i, to_bool128(p), o)
#define v_bf16_shl_v_v_b(a, b, i, p, o)   v_bf16_shl_b(a, b, 0, i, p, o)

#define v_i32_shl_v_v_vb(a, b, i, p, o)   v_i32_shl_vb(a, b, 0, i, to_bool64(p), o)
#define v_i32_shl_v_v_b(a, b, i, p, o)    v_i32_shl_b(a, b, 0, i, p, o)
#define v_u32_shl_v_v_vb(a, b, i, p, o)   v_u32_shl_vb(a, b, 0, i, to_bool64(p), o)
#define v_u32_shl_v_v_b(a, b, i, p, o)    v_u32_shl_b(a, b, 0, i, p, o)
#define v_i16_shl_v_v_vb(a, b, i, p, o)   v_i16_shl_vb(a, b, 0, i, to_bool128(p), o)
#define v_i16_shl_v_v_b(a, b, i, p, o)    v_i16_shl_b(a, b, 0, i, p, o)
#define v_u16_shl_v_v_vb(a, b, i, p, o)   v_u16_shl_vb(a, b, 0, i, to_bool128(p), o)
#define v_u16_shl_v_v_b(a, b, i, p, o)    v_u16_shl_b(a, b, 0, i, p, o)
#define v_i8_shl_v_v_vb(a, b, i, p, o)    v_i8_shl_vb(a, b, 0, i, p, o)
#define v_i8_shl_v_v_b(a, b, i, p, o)     v_i8_shl_b(a, b, 0, i, p, o)
#define v_u8_shl_v_v_vb(a, b, i, p, o)    v_u8_shl_vb(a, b, 0, i, p, o)
#define v_u8_shl_v_v_b(a, b, i, p, o)     v_u8_shl_b(a, b, 0, i, p, o)

#define v_f32_shl_v_s_vb                  v_f32_shl_v_v_vb
#define v_f32_shl_v_s_b                   v_f32_shl_v_v_b
#define v_bf16_shl_v_s_vb                 v_bf16_shl_v_v_vb
#define v_bf16_shl_v_s_b                  v_bf16_shl_v_v_b
#define v_i32_shl_v_s_vb                  v_i32_shl_v_v_vb
#define v_i32_shl_v_s_b                   v_i32_shl_v_v_b
#define v_u32_shl_v_s_vb                  v_u32_shl_v_v_vb
#define v_u32_shl_v_s_b                   v_u32_shl_v_v_b
#define v_i16_shl_v_s_vb                  v_i16_shl_v_v_vb
#define v_i16_shl_v_s_b                   v_i16_shl_v_v_b
#define v_u16_shl_v_s_vb                  v_u16_shl_v_v_vb
#define v_u16_shl_v_s_b                   v_u16_shl_v_v_b
#define v_i8_shl_v_s_vb                   v_i8_shl_v_v_vb
#define v_i8_shl_v_s_b                    v_i8_shl_v_v_b
#define v_u8_shl_v_s_vb                   v_u8_shl_v_v_vb
#define v_u8_shl_v_s_b                    v_u8_shl_v_v_b

#define v_f32_shl_v_v(a, b)               v_f32_shl_v_v_b(a, b, 0, 1, 0)
#define v_bf16_shl_v_v(a, b)              v_bf16_shl_v_v_b(a, b, 0, 1, 0)
#define v_i32_shl_v_v(a, b)               v_i32_shl_v_v_b(a, b, 0, 1, 0)
#define v_u32_shl_v_v(a, b)               v_u32_shl_v_v_b(a, b, 0, 1, 0)
#define v_i16_shl_v_v(a, b)               v_i16_shl_v_v_b(a, b, 0, 1, 0)
#define v_u16_shl_v_v(a, b)               v_u16_shl_v_v_b(a, b, 0, 1, 0)
#define v_i8_shl_v_v(a, b)                v_i8_shl_v_v_b(a, b, 0, 1, 0)
#define v_u8_shl_v_v(a, b)                v_u8_shl_v_v_b(a, b, 0, 1, 0)

#define v_f32_shl_v_s                     v_f32_shl_v_v
#define v_bf16_shl_v_s                    v_bf16_shl_v_v
#define v_i32_shl_v_s                     v_i32_shl_v_v
#define v_u32_shl_v_s                     v_u32_shl_v_v
#define v_i16_shl_v_s                     v_i16_shl_v_v
#define v_u16_shl_v_s                     v_u16_shl_v_v
#define v_i8_shl_v_s                      v_i8_shl_v_v
#define v_u8_shl_v_s                      v_u8_shl_v_v


// ASH
#define s_i32_ash_s_s_b(a, b, i, rne, p, o)    s_i32_ash(a, b, rne << 1, i, p, o)
#define s_u32_ash_s_s_b(a, b, i, rne, p, o)    s_u32_ash(a, b, rne << 1, i, p, o)
#define s_i16_ash_s_s_b(a, b, i, rne, p, o)    s_i16_ash(a, b, rne << 1, i, p, o)
#define s_u16_ash_s_s_b(a, b, i, rne, p, o)    s_u16_ash(a, b, rne << 1, i, p, o)
#define s_i8_ash_s_s_b(a, b, i, rne, p, o)     s_i8_ash(a, b, rne << 1, i, p, o)
#define s_u8_ash_s_s_b(a, b, i, rne, p, o)     s_u8_ash(a, b, rne << 1, i, p, o)

#define s_i32_ash_s_s(a, b, rne)               s_i32_ash_s_s_b(a, b, 0, rne, 1, 0)
#define s_u32_ash_s_s(a, b, rne)               s_u32_ash_s_s_b(a, b, 0, rne, 1, 0)
#define s_i16_ash_s_s(a, b, rne)               s_i16_ash_s_s_b(a, b, 0, rne, 1, 0)
#define s_u16_ash_s_s(a, b, rne)               s_u16_ash_s_s_b(a, b, 0, rne, 1, 0)
#define s_i8_ash_s_s(a, b, rne)                s_i8_ash_s_s_b(a, b, 0, rne, 1, 0)
#define s_u8_ash_s_s(a, b, rne)                s_u8_ash_s_s_b(a, b, 0, rne, 1, 0)

#define v_bf16_ash_v_v_vb(a, b, i, rne, p, o)  v_bf16_ash_vb(a, b, rne << 1, i, to_bool128(p), o)
#define v_bf16_ash_v_v_b(a, b, i, rne, p, o)   v_bf16_ash_b(a, b, rne << 1, i, p, o)
#define v_i32_ash_v_v_vb(a, b, i, rne, p, o)   v_i32_ash_vb(a, b, rne << 1, i, to_bool64(p), o)
#define v_i32_ash_v_v_b(a, b, i, rne, p, o)    v_i32_ash_b(a, b, rne << 1, i, p, o)
#define v_u32_ash_v_v_vb(a, b, i, rne, p, o)   v_u32_ash_vb(a, b, rne << 1, i, to_bool64(p), o)
#define v_u32_ash_v_v_b(a, b, i, rne, p, o)    v_u32_ash_b(a, b, rne << 1, i, p, o)
#define v_i16_ash_v_v_vb(a, b, i, rne, p, o)   v_i16_ash_vb(a, b, rne << 1, i, to_bool128(p), o)
#define v_i16_ash_v_v_b(a, b, i, rne, p, o)    v_i16_ash_b(a, b, rne << 1, i, p, o)
#define v_u16_ash_v_v_vb(a, b, i, rne, p, o)   v_u16_ash_vb(a, b, rne << 1, i, to_bool128(p), o)
#define v_u16_ash_v_v_b(a, b, i, rne, p, o)    v_u16_ash_b(a, b, rne << 1, i, p, o)
#define v_i8_ash_v_v_vb(a, b, i, rne, p, o)    v_i8_ash_vb(a, b, rne << 1, i, p, o)
#define v_i8_ash_v_v_b(a, b, i, rne, p, o)     v_i8_ash_b(a, b, rne << 1, i, p, o)
#define v_u8_ash_v_v_vb(a, b, i, rne, p, o)    v_u8_ash_vb(a, b, rne << 1, i, p, o)
#define v_u8_ash_v_v_b(a, b, i, rne, p, o)     v_u8_ash_b(a, b, rne << 1, i, p, o)

#define v_i32_ash_v_s_vb                  v_i32_ash_v_v_vb
#define v_i32_ash_v_s_b                   v_i32_ash_v_v_b
#define v_u32_ash_v_s_vb                  v_u32_ash_v_v_vb
#define v_u32_ash_v_s_b                   v_u32_ash_v_v_b
#define v_i16_ash_v_s_vb                  v_i16_ash_v_v_vb
#define v_i16_ash_v_s_b                   v_i16_ash_v_v_b
#define v_u16_ash_v_s_vb                  v_u16_ash_v_v_vb
#define v_u16_ash_v_s_b                   v_u16_ash_v_v_b
#define v_i8_ash_v_s_vb                   v_i8_ash_v_v_vb
#define v_i8_ash_v_s_b                    v_i8_ash_v_v_b
#define v_u8_ash_v_s_vb                   v_u8_ash_v_v_vb
#define v_u8_ash_v_s_b                    v_u8_ash_v_v_b

#define v_i32_ash_v_v(a, b, rne)               v_i32_ash_v_v_b(a, b, 0, rne, 1, 0)
#define v_u32_ash_v_v(a, b, rne)               v_u32_ash_v_v_b(a, b, 0, rne, 1, 0)
#define v_i16_ash_v_v(a, b, rne)               v_i16_ash_v_v_b(a, b, 0, rne, 1, 0)
#define v_u16_ash_v_v(a, b, rne)               v_u16_ash_v_v_b(a, b, 0, rne, 1, 0)
#define v_i8_ash_v_v(a, b, rne)                v_i8_ash_v_v_b(a, b, 0, rne, 1, 0)
#define v_u8_ash_v_v(a, b, rne)                v_u8_ash_v_v_b(a, b, 0, rne, 1, 0)

#define v_i32_ash_v_s                     v_i32_ash_v_v
#define v_u32_ash_v_s                     v_u32_ash_v_v
#define v_i16_ash_v_s                     v_i16_ash_v_v
#define v_u16_ash_v_s                     v_u16_ash_v_v
#define v_i8_ash_v_s                      v_i8_ash_v_v
#define v_u8_ash_v_s                      v_u8_ash_v_v

// ST_L
#define f32_st_l_s_s_b(a, v, s, p, o)     s_f32_st_l(a, v, s, p, o)
#define bf16_st_l_s_s_b(a, v, s, p, o)    s_bf16_st_l(a, v, s, p, o)
#define i32_st_l_s_s_b(a, v, s, p, o)     s_i32_st_l(a, v, s, p, o)
#define u32_st_l_s_s_b(a, v, s, p, o)     s_u32_st_l(a, v, s, p, o)
#define i16_st_l_s_s_b(a, v, s, p, o)     s_i16_st_l(a, v, s, p, o)
#define u16_st_l_s_s_b(a, v, s, p, o)     s_u16_st_l(a, v, s, p, o)
#define i8_st_l_s_s_b(a, v, s, p, o)      s_i8_st_l(a, v, s, p, o)
#define u8_st_l_s_s_b(a, v, s, p, o)      s_u8_st_l(a, v, s, p, o)
#define b_st_l_s_b_b(a, v, s, p, o)       s_i1_st_l(a, v, s, p, o)

#define f32_st_l_s_s(a, v, s)             f32_st_l_s_s_b(a, v, s, 1, 0)
#define bf16_st_l_s_s(a, v, s)            bf16_st_l_s_s_b(a, v, s, 1, 0)
#define f16_st_l_s_s(a, v, s)             f16_st_l_s_s_b(a, v, s, 1, 0)
#define i32_st_l_s_s(a, v, s)             i32_st_l_s_s_b(a, v, s, 1, 0)
#define u32_st_l_s_s(a, v, s)             u32_st_l_s_s_b(a, v, s, 1, 0)
#define i16_st_l_s_s(a, v, s)             i16_st_l_s_s_b(a, v, s, 1, 0)
#define u16_st_l_s_s(a, v, s)             u16_st_l_s_s_b(a, v, s, 1, 0)
#define i8_st_l_s_s(a, v, s)              i8_st_l_s_s_b(a, v, s, 1, 0)
#define u8_st_l_s_s(a, v, s)              u8_st_l_s_s_b(a, v, s, 1, 0)
#define b_st_l_s_b(a, v, s)               b_st_l_s_b_b(a, v, s, 1, 0)


// ST_G
#define f32_st_g_a_s_b(a, v, p, o)     s_f32_st_g(a, v, 0, p, o)
#define bf16_st_g_a_s_b(a, v, p, o)    s_bf16_st_g(a, v, 0, p, o)
#define i32_st_g_a_s_b(a, v, p, o)     s_i32_st_g(a, v, 0, p, o)
#define u32_st_g_a_s_b(a, v, p, o)     s_u32_st_g(a, v, 0, p, o)
#define i16_st_g_a_s_b(a, v, p, o)     s_i16_st_g(a, v, 0, p, o)
#define u16_st_g_a_s_b(a, v, p, o)     s_u16_st_g(a, v, 0, p, o)
#define i8_st_g_a_s_b(a, v, p, o)      s_i8_st_g(a, v, 0, p, o)
#define u8_st_g_a_s_b(a, v, p, o)      s_u8_st_g(a, v, 0, p, o)
#define b_st_g_a_b_b(a, v, p, o)       s_i1_st_g(a, v, 0, p, o)

#define f32_st_g_a_s(a, v)             f32_st_g_a_s_b(a, v, 1, 0)
#define bf16_st_g_a_s(a, v)            bf16_st_g_a_s_b(a, v, 1, 0)
#define f16_st_g_a_s(a, v)             f16_st_g_a_s_b(a, v, 1, 0)
#define i32_st_g_a_s(a, v)             i32_st_g_a_s_b(a, v, 1, 0)
#define u32_st_g_a_s(a, v)             u32_st_g_a_s_b(a, v, 1, 0)
#define i16_st_g_a_s(a, v)             i16_st_g_a_s_b(a, v, 1, 0)
#define u16_st_g_a_s(a, v)             u16_st_g_a_s_b(a, v, 1, 0)
#define i8_st_g_a_s(a, v)              i8_st_g_a_s_b(a, v, 1, 0)
#define u8_st_g_a_s(a, v)              u8_st_g_a_s_b(a, v, 1, 0)
#define b_st_g_a_b(a, v)               b_st_g_a_b_b(a, v, 1, 0)


// ST_L_V

#define f32_st_l_v_s_v_b(a, v, p, o)    v_f32_st_l_v(a, v, 0, p, o)
#define bf16_st_l_v_s_v_b(a, v, p, o)   v_bf16_st_l_v(a, v, 0, p, o)
#define i32_st_l_v_s_v_b(a, v, p, o)    v_i32_st_l_v(a, v, 0, p, o)
#define u32_st_l_v_s_v_b(a, v, p, o)    v_u32_st_l_v(a, v, 0, p, o)
#define i16_st_l_v_s_v_b(a, v, p, o)    v_i16_st_l_v(a, v, 0, p, o)
#define u16_st_l_v_s_v_b(a, v, p, o)    v_u16_st_l_v(a, v, 0, p, o)
#define i8_st_l_v_s_v_b(a, v, p, o)     v_i8_st_l_v(a, v, 0, p, o)
#define u8_st_l_v_s_v_b(a, v, p, o)     v_u8_st_l_v(a, v, 0, p, o)
#define st_l_v_s_bv_b(a, v, p, o)       v_i1_st_l_v(a, v, 0, p, o)

#define f32_st_l_v_s_v(a, v)            f32_st_l_v_s_v_b(a, v, 1, 0)
#define bf16_st_l_v_s_v(a, v)           bf16_st_l_v_s_v_b(a, v, 1, 0)
#define f16_st_l_v_s_v(a, v)            f16_st_l_v_s_v_b(a, v, 1, 0)
#define i32_st_l_v_s_v(a, v)            i32_st_l_v_s_v_b(a, v, 1, 0)
#define u32_st_l_v_s_v(a, v)            u32_st_l_v_s_v_b(a, v, 1, 0)
#define i16_st_l_v_s_v(a, v)            i16_st_l_v_s_v_b(a, v, 1, 0)
#define u16_st_l_v_s_v(a, v)            u16_st_l_v_s_v_b(a, v, 1, 0)
#define i8_st_l_v_s_v(a, v)             i8_st_l_v_s_v_b(a, v, 1, 0)
#define u8_st_l_v_s_v(a, v)             u8_st_l_v_s_v_b(a, v, 1, 0)
#define st_l_v_s_bv(a, v)               st_l_v_s_bv_b(a, v, 1, 0)

#define f32_st_l_v_low_s_v_b(a, v, p, o)    v_f32_st_l_v_low(a, v, 0, p, o)
#define bf16_st_l_v_low_s_v_b(a, v, p, o)   v_bf16_st_l_v_low(a, v, 0, p, o)
#define i32_st_l_v_low_s_v_b(a, v, p, o)    v_i32_st_l_v_low(a, v, 0, p, o)
#define u32_st_l_v_low_s_v_b(a, v, p, o)    v_u32_st_l_v_low(a, v, 0, p, o)
#define i16_st_l_v_low_s_v_b(a, v, p, o)    v_i16_st_l_v_low(a, v, 0, p, o)
#define u16_st_l_v_low_s_v_b(a, v, p, o)    v_u16_st_l_v_low(a, v, 0, p, o)
#define i8_st_l_v_low_s_v_b(a, v, p, o)     v_i8_st_l_v_low(a, v, 0, p, o)
#define u8_st_l_v_low_s_v_b(a, v, p, o)     v_u8_st_l_v_low(a, v, 0, p, o)
#define st_l_v_low_s_bv_b(a, v, p, o)       v_i1_st_l_v_low(a, v, 0, p, o)

#define f32_st_l_v_low_s_v(a, v)            f32_st_l_v_low_s_v_b(a, v, 1, 0)
#define bf16_st_l_v_low_s_v(a, v)           bf16_st_l_v_low_s_v_b(a, v, 1, 0)
#define f16_st_l_v_low_s_v(a, v)            f16_st_l_v_low_s_v_b(a, v, 1, 0)
#define i32_st_l_v_low_s_v(a, v)            i32_st_l_v_low_s_v_b(a, v, 1, 0)
#define u32_st_l_v_low_s_v(a, v)            u32_st_l_v_low_s_v_b(a, v, 1, 0)
#define i16_st_l_v_low_s_v(a, v)            i16_st_l_v_low_s_v_b(a, v, 1, 0)
#define u16_st_l_v_low_s_v(a, v)            u16_st_l_v_low_s_v_b(a, v, 1, 0)
#define i8_st_l_v_low_s_v(a, v)             i8_st_l_v_low_s_v_b(a, v, 1, 0)
#define u8_st_l_v_low_s_v(a, v)             u8_st_l_v_low_s_v_b(a, v, 1, 0)
#define st_l_v_low_s_bv(a, v)               st_l_v_low_s_bv_b(a, v, 1, 0)

#define f32_st_l_v_high_s_v_b(a, v, p, o)   v_f32_st_l_v_high(a, v, 0, p, o)
#define bf16_st_l_v_high_s_v_b(a, v, p, o)  v_bf16_st_l_v_high(a, v, 0, p, o)
#define i32_st_l_v_high_s_v_b(a, v, p, o)   v_i32_st_l_v_high(a, v, 0, p, o)
#define u32_st_l_v_high_s_v_b(a, v, p, o)   v_u32_st_l_v_high(a, v, 0, p, o)
#define i16_st_l_v_high_s_v_b(a, v, p, o)   v_i16_st_l_v_high(a, v, 0, p, o)
#define u16_st_l_v_high_s_v_b(a, v, p, o)   v_u16_st_l_v_high(a, v, 0, p, o)
#define i8_st_l_v_high_s_v_b(a, v, p, o)    v_i8_st_l_v_high(a, v, 0, p, o)
#define u8_st_l_v_high_s_v_b(a, v, p, o)    v_u8_st_l_v_high(a, v, 0, p, o)
#define st_l_v_high_s_bv_b(a, v, p, o)      v_i1_st_l_v_high(a, v, 0, p, o)

#define f32_st_l_v_high_s_v(a, v)           f32_st_l_v_high_s_v_b(a, v, 1, 0)
#define bf16_st_l_v_high_s_v(a, v)          bf16_st_l_v_high_s_v_b(a, v, 1, 0)
#define f16_st_l_v_high_s_v(a, v)           f16_st_l_v_high_s_v_b(a, v, 1, 0)
#define i32_st_l_v_high_s_v(a, v)           i32_st_l_v_high_s_v_b(a, v, 1, 0)
#define u32_st_l_v_high_s_v(a, v)           u32_st_l_v_high_s_v_b(a, v, 1, 0)
#define i16_st_l_v_high_s_v(a, v)           i16_st_l_v_high_s_v_b(a, v, 1, 0)
#define u16_st_l_v_high_s_v(a, v)           u16_st_l_v_high_s_v_b(a, v, 1, 0)
#define i8_st_l_v_high_s_v(a, v)            i8_st_l_v_high_s_v_b(a, v, 1, 0)
#define u8_st_l_v_high_s_v(a, v)            u8_st_l_v_high_s_v_b(a, v, 1, 0)
#define st_l_v_high_s_bv(a, v)              st_l_v_high_s_bv_b(a, v, 1, 0)

#define aso_b(a, b, p, o)     aso(a | (b << 1), p, o)

#define s_u32_udiv_step_s_b(i, a, s, p, o) u32_udiv_step(a, s, 0, i, p, o)
#define s_u32_udiv_step_s(i, a, s)         s_u32_udiv_step_s_b(i, a, s, 1, 0)
#define s_u16_udiv_step_s_b(i, a, s, p, o) u16_udiv_step(a, s, 0, i, p, o)
#define s_u16_udiv_step_s(i, a, s)         s_u16_udiv_step_s_b(i, a, s, 1, 0)
#define s_u8_udiv_step_s_b(i, a, s, p, o)  u8_udiv_step(a, s, 0, i, p, o)
#define s_u8_udiv_step_s(i, a, s)          s_u8_udiv_step_s_b(i, a, s, 1, 0)


#define s_u32_udiv_4step_s_b(i, a, s, p, o) u32_udiv_4step(a, s, 0, i, p, o)
#define s_u32_udiv_4step_s(i, a, s)         s_u32_udiv_4step_s_b(i, a, s, 1, 0)
#define s_u16_udiv_4step_s_b(i, a, s, p, o) u16_udiv_4step(a, s, 0, i, p, o)
#define s_u16_udiv_4step_s(i, a, s)         s_u16_udiv_4step_s_b(i, a, s, 1, 0)
#define s_u8_udiv_4step_s_b(i, a, s, p, o)  u8_udiv_4step(a, s, 0, i, p, o)
#define s_u8_udiv_4step_s(i, a, s)          s_u8_udiv_4step_s_b(i, a, s, 1, 0)

// MOV
#define bv_i32_mov_flavor_s_vb(a, i, f, p, o)   v_i1_mov_flavor_vb(a, f, 0, i, p, o)
#define bv_i32_mov_flavor_s_b(a, i, f, p, o)    v_i1_mov_flavor_b(a, f, 0, i, p, o)
#define bv_i32_mov_flavor_s(a, i, f)            bv_i32_mov_flavor_s_b(a, i, f, 1, 0)
#define bv_u32_mov_flavor_s_vb(a, i, f, p, o)   v_i1_mov_flavor_vb(a, f, 0, i, p, o)
#define bv_u32_mov_flavor_s_b(a, i, f, p, o)    v_i1_mov_flavor_b(a, f, 0, i, p, o)
#define bv_u32_mov_flavor_s(a, i, f)            bv_u32_mov_flavor_s_b(a, i, f, 1, 0)

#define i_i32_mov_s_b(a, i, m, p, o)  i_i32_mov(a, m, 0, i, p, o)
#define i_i32_mov_i_b(a, i, m, p, o)  i_i32_mov(a, m, 0, i, p, o)
#define i_i32_mov_s(a, i, m)          i_i32_mov_s_b(a, i, m, 1, 0)
#define i_i32_mov_i(a, i, m)          i_i32_mov_i_b(a, i, m, 1, 0)

#define b_b_mov_b_b(a, i, p, o)       s_i1_mov(a, 0, i, p, o)
#define b_b_mov_b(a)                  (a)

#define b_f32_mov_s_b(a, i, p, o)     s_i1_mov((as_int(a) & 0x01), 0, i, p, o)
#define b_bf16_mov_s_b(a, i, p, o)    s_i1_mov((as_short(a) & 0x01), 0, i, p, o)
#define b_i32_mov_s_b(a, i, p, o)     s_i1_mov(((a) & 0x01), 0, i, p, o)
#define b_u32_mov_s_b(a, i, p, o)     s_i1_mov(((a) & 0x01), 0, i, p, o)
#define b_i16_mov_s_b(a, i, p, o)     s_i1_mov(((a) & 0x01), 0, i, p, o)
#define b_u16_mov_s_b(a, i, p, o)     s_i1_mov(((a) & 0x01), 0, i, p, o)
#define b_i8_mov_s_b(a, i, p, o)      s_i1_mov(((a) & 0x01), 0, i, p, o)
#define b_u8_mov_s_b(a, i, p, o)      s_i1_mov(((a) & 0x01), 0, i, p, o)

#define b_f32_mov_s(a)                b_f32_mov_s_b(a, 0, 1, 0)
#define b_bf16_mov_s(a)               b_bf16_mov_s_b(a, 0, 1, 0)
#define b_i32_mov_s(a)                b_i32_mov_s_b(a, 0, 1, 0)
#define b_u32_mov_s(a)                b_u32_mov_s_b(a, 0, 1, 0)
#define b_i16_mov_s(a)                b_i16_mov_s_b(a, 0, 1, 0)
#define b_u16_mov_s(a)                b_u16_mov_s_b(a, 0, 1, 0)
#define b_i8_mov_s(a)                 b_i8_mov_s_b(a, 0, 1, 0)
#define b_u8_mov_s(a)                 b_u8_mov_s_b(a, 0, 1, 0)

#define bv_mov_bv_b(a, i, p, o)       v_i1_mov_b(a, 0, i, p, o)
#define bv_mov_bv_vb(a, i, p, o)      v_i1_mov_vb(a, 0, i, p, o)
#define bv_mov_bv(a)                  (a)

#define bv_mov_b_b(a, i, p, o)        v_i1_mov_i1_b(a, 0, i, p, o)
#define bv_mov_b_vb(a, i, p, o)       v_i1_mov_i1_vb(a, 0, i, p, o)
#define bv_mov_b(a)                   bv_mov_b_b(a, (bool256){0}, 1, 0)

#define bv_b_mov_bv_b                 bv_mov_bv_b
#define bv_b_mov_bv_vb                bv_mov_bv_vb
#define bv_b_mov_bv                   bv_mov_bv
#define bv_b_mov_b_b                  bv_mov_b_b
#define bv_b_mov_b_vb                 bv_mov_b_vb
#define bv_b_mov_b                    bv_mov_b

#define v_f32_mov_v_vb(a, i, p, o)    v_f32_mov_vb(a, 0, i, to_bool64(p), o)
#define v_bf16_mov_v_vb(a, i, p, o)   v_bf16_mov_vb(a, 0, i, to_bool128(p), o)
#define v_i32_mov_v_vb(a, i, p, o)    v_i32_mov_vb(a, 0, i, to_bool64(p), o)
#define v_u32_mov_v_vb(a, i, p, o)    v_u32_mov_vb(a, 0, i, to_bool64(p), o)
#define v_i16_mov_v_vb(a, i, p, o)    v_i16_mov_vb(a, 0, i, to_bool128(p), o)
#define v_u16_mov_v_vb(a, i, p, o)    v_u16_mov_vb(a, 0, i, to_bool128(p), o)
#define v_i8_mov_v_vb(a, i, p, o)     v_i8_mov_vb(a, 0, i, p, o)
#define v_u8_mov_v_vb(a, i, p, o)     v_u8_mov_vb(a, 0, i, p, o)

#define v_f32_mov_v_b(a, i, p, o)     v_f32_mov_b(a, 0, i, p, o)
#define v_bf16_mov_v_b(a, i, p, o)    v_bf16_mov_b(a, 0, i, p, o)
#define v_i32_mov_v_b(a, i, p, o)     v_i32_mov_b(a, 0, i, p, o)
#define v_u32_mov_v_b(a, i, p, o)     v_u32_mov_b(a, 0, i, p, o)
#define v_i16_mov_v_b(a, i, p, o)     v_i16_mov_b(a, 0, i, p, o)
#define v_u16_mov_v_b(a, i, p, o)     v_u16_mov_b(a, 0, i, p, o)
#define v_i8_mov_v_b(a, i, p, o)      v_i8_mov_b(a, 0, i, p, o)
#define v_u8_mov_v_b(a, i, p, o)      v_u8_mov_b(a, 0, i, p, o)

#define v_f32_mov_s_b                 v_f32_mov_v_b
#define v_bf16_mov_s_b                v_bf16_mov_v_b
#define v_i32_mov_s_b                 v_i32_mov_v_b
#define v_u32_mov_s_b                 v_u32_mov_v_b
#define v_i16_mov_s_b                 v_i16_mov_v_b
#define v_u16_mov_s_b                 v_u16_mov_v_b
#define v_i8_mov_s_b                  v_i8_mov_v_b
#define v_u8_mov_s_b                  v_u8_mov_v_b

#define v_f32_mov_s_vb                v_f32_mov_v_vb
#define v_bf16_mov_s_vb               v_bf16_mov_v_vb
#define v_i32_mov_s_vb                v_i32_mov_v_vb
#define v_u32_mov_s_vb                v_u32_mov_v_vb
#define v_i16_mov_s_vb                v_i16_mov_v_vb
#define v_u16_mov_s_vb                v_u16_mov_v_vb
#define v_i8_mov_s_vb                 v_i8_mov_v_vb
#define v_u8_mov_s_vb                 v_u8_mov_v_vb

#define v_f32_mov_v(a)                (a)
#define v_bf16_mov_v(a)               (a)
#define v_i32_mov_v(a)                (a)
#define v_u32_mov_v(a)                (a)
#define v_i16_mov_v(a)                (a)
#define v_u16_mov_v(a)                (a)
#define v_i8_mov_v(a)                 (a)
#define v_u8_mov_v(a)                 (a)

#define v_f32_mov_s(a)                (a)
#define v_bf16_mov_s(a)               (a)
#define v_i32_mov_s(a)                v_i32_mov_s_b(a, 0, 1, 0)
#define v_u32_mov_s(a)                v_u32_mov_s_b(a, 0, 1, 0)
#define v_i16_mov_s(a)                v_i16_mov_s_b(a, 0, 1, 0)
#define v_u16_mov_s(a)                v_u16_mov_s_b(a, 0, 1, 0)
#define v_i8_mov_s(a)                 v_i8_mov_s_b(a, 0, 1, 0)
#define v_u8_mov_s(a)                 v_u8_mov_s_b(a, 0, 1, 0)

#define s_f32_mov_s_b(a, i, p, o)     s_f32_mov(a, 0, i, p, o)
#define s_bf16_mov_s_b(a, i, p, o)    s_bf16_mov(a, 0, i, p, o)
#define s_i32_mov_s_b(a, i, p, o)     s_i32_mov(a, 0, i, p, o)
#define s_u32_mov_s_b(a, i, p, o)     s_u32_mov(a, 0, i, p, o)
#define s_i16_mov_s_b(a, i, p, o)     s_i16_mov(a, 0, i, p, o)
#define s_u16_mov_s_b(a, i, p, o)     s_u16_mov(a, 0, i, p, o)
#define s_i8_mov_s_b(a, i, p, o)      s_i8_mov(a, 0, i, p, o)
#define s_u8_mov_s_b(a, i, p, o)      s_u8_mov(a, 0, i, p, o)

#define s_f32_mov_s(a)                (a)
#define s_bf16_mov_s(a)               (a)
#define s_i32_mov_s(a)                (a)
#define s_u32_mov_s(a)                (a)
#define s_i16_mov_s(a)                (a)
#define s_u16_mov_s(a)                (a)
#define s_i8_mov_s(a)                 (a)
#define s_u8_mov_s(a)                 (a)

#endif

#if defined(__gaudi__)
#define v_f32_ld_tnsr_r_b       v_f32_ld_tnsr_b
#define v_i32_ld_tnsr_r_b       v_i32_ld_tnsr_b
#define v_u32_ld_tnsr_r_b       v_u32_ld_tnsr_b
#define v_i16_ld_tnsr_r_b       v_i16_ld_tnsr_b
#define v_u16_ld_tnsr_r_b       v_u16_ld_tnsr_b
#define v_i8_ld_tnsr_r_b        v_i8_ld_tnsr_b
#define v_u8_ld_tnsr_r_b        v_u8_ld_tnsr_b
#define v_i1_ld_tnsr_r_b        v_i1_ld_tnsr_b

#define v_f32_ld_tnsr_low_r_b   v_f32_ld_tnsr_low_b
#define v_i32_ld_tnsr_low_r_b   v_i32_ld_tnsr_low_b
#define v_u32_ld_tnsr_low_r_b   v_u32_ld_tnsr_low_b
#define v_i16_ld_tnsr_low_r_b   v_i16_ld_tnsr_low_b
#define v_u16_ld_tnsr_low_r_b   v_u16_ld_tnsr_low_b
#define v_i8_ld_tnsr_low_r_b    v_i8_ld_tnsr_low_b
#define v_u8_ld_tnsr_low_r_b    v_u8_ld_tnsr_low_b
#define v_i1_ld_tnsr_low_r_b    v_i1_ld_tnsr_low_b

#define v_f32_ld_tnsr_high_r_b  v_f32_ld_tnsr_high_b
#define v_i32_ld_tnsr_high_r_b  v_i32_ld_tnsr_high_b
#define v_u32_ld_tnsr_high_r_b  v_u32_ld_tnsr_high_b
#define v_i16_ld_tnsr_high_r_b  v_i16_ld_tnsr_high_b
#define v_u16_ld_tnsr_high_r_b  v_u16_ld_tnsr_high_b
#define v_i8_ld_tnsr_high_r_b   v_i8_ld_tnsr_high_b
#define v_u8_ld_tnsr_high_r_b   v_u8_ld_tnsr_high_b
#define v_i1_ld_tnsr_high_r_b   v_i1_ld_tnsr_high_b

#define v_f32_st_tnsr_r         v_f32_st_tnsr
#define v_bf16_st_tnsr_r        v_bf16_st_tnsr
#define v_i32_st_tnsr_r         v_i32_st_tnsr
#define v_u32_st_tnsr_r         v_u32_st_tnsr
#define v_i16_st_tnsr_r         v_i16_st_tnsr
#define v_u16_st_tnsr_r         v_u16_st_tnsr
#define v_i8_st_tnsr_r          v_i8_st_tnsr
#define v_u8_st_tnsr_r          v_u8_st_tnsr
#define v_i1_st_tnsr_r          v_i1_st_tnsr

#define v_f32_st_tnsr_low_r     v_f32_st_tnsr_low
#define v_bf16_st_tnsr_low_r    v_bf16_st_tnsr_low
#define v_i32_st_tnsr_low_r     v_i32_st_tnsr_low
#define v_u32_st_tnsr_low_r     v_u32_st_tnsr_low
#define v_i16_st_tnsr_low_r     v_i16_st_tnsr_low
#define v_u16_st_tnsr_low_r     v_u16_st_tnsr_low
#define v_i8_st_tnsr_low_r      v_i8_st_tnsr_low
#define v_u8_st_tnsr_low_r      v_u8_st_tnsr_low
#define v_i1_st_tnsr_low_r      v_i1_st_tnsr_low

#define v_f32_st_tnsr_high_r    v_f32_st_tnsr_high
#define v_bf16_st_tnsr_high_r   v_bf16_st_tnsr_high
#define v_i32_st_tnsr_high_r    v_i32_st_tnsr_high
#define v_u32_st_tnsr_high_r    v_u32_st_tnsr_high
#define v_i16_st_tnsr_high_r    v_i16_st_tnsr_high
#define v_u16_st_tnsr_high_r    v_u16_st_tnsr_high
#define v_i8_st_tnsr_high_r     v_i8_st_tnsr_high
#define v_u8_st_tnsr_high_r     v_u8_st_tnsr_high
#define v_i1_st_tnsr_high_r     v_i1_st_tnsr_high
#endif

#define cache_flush_b         cache_flush
#define cache_invalidate_b    cache_invalidate

//hack to avoid ISA mispell
#define bv_i16_and_bv_bv bv_b_and_bv_bv
#define bv_i16_and_bv_bv_b bv_b_and_bv_bv_b
#define bv_i16_and_bv_bv_vb bv_b_and_bv_bv_vb
#define bv_u16_and_bv_bv bv_b_and_bv_bv
#define bv_u16_and_bv_bv_b bv_b_and_bv_bv_b
#define bv_u16_and_bv_bv_vb bv_b_and_bv_bv_vb


#endif
