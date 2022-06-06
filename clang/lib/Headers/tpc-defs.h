//------------------------------------------------------------------------------
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
#if defined(__gaudi__) || defined(__gaudib__) || defined(__greco__) || defined(__gaudi2__) || defined(__doron1__)
typedef _BFloat16 bf16;
typedef _BFloat16 bfloat;
typedef _BFloat16     __attribute__((ext_vector_type(128)))   bfloat128;
#endif
#if defined(__gaudib__) || defined(__greco__) || defined(__gaudi2__) || defined(__doron1__)
typedef half          __attribute__((ext_vector_type(128)))   half128;
typedef char          __attribute__((__vector_size__(256)))   nibble512;
typedef unsigned char __attribute__((__vector_size__(256)))   unibble512;
typedef char nibble;
typedef unsigned char unibble;
#endif
#if defined(__gaudi2__) || defined(__doron1__)
typedef _Float8_143 minifloat;
typedef _Float8_152 minihalf;
typedef minifloat __attribute__((ext_vector_type(256)))  minifloat256;
typedef minihalf __attribute__((ext_vector_type(256)))   minihalf256;
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

#if defined(__gaudi__) || defined(__gaudib__) || defined(__greco__) || defined(__gaudi2__) || defined(__doron1__)
#define as_bf16(x)      __builtin_astype((x), bf16)
#define as_bfloat(x)    __builtin_astype((x), bfloat)
#define as_bfloat128(x) __builtin_astype((x), bfloat128)
#endif

#if defined(__gaudib__) || defined(__greco__) || defined(__gaudi2__) || defined(__doron1__)
#define as_half(x)      __builtin_astype((x), half)
#define as_half128(x)   __builtin_astype((x), half128)
#endif

#if defined(__gaudi2__) || defined(__doron1__)
#define as_f8_143(x)      __builtin_astype((x), f8_143)
#define as_f8_152(x)      __builtin_astype((x), f8_152)
#define as_v256f8_143(x)  __builtin_astype((x), v256f8_143)
#define as_v256f8_152(x)  __builtin_astype((x), v256f8_152)
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

#if defined(__gaudi__) || defined(__gaudib__) || defined(__greco__) || defined(__gaudi2__) || defined(__doron1__)
// _BFloat16
typedef struct _bfloat128_pair_t {
  bfloat128 v1;
  bfloat128 v2;
} bfloat128_pair_t;
typedef struct _bfloat128_pair_t bfloat128_bfloat128_pair_t;
typedef struct _bfloat128_pair_t bfloat256;

#if defined(__gaudib__) || defined(__greco__) || defined(__gaudi2__) || defined(__doron1__)
typedef struct _bfloat128_half128_pair_t {
  bfloat128 v1;
  half128 v2;
} bfloat128_half128_pair_t;
#endif

typedef struct _bfloat128_short128_pair_t {
  bfloat128 v1;
  short128 v2;
} bfloat128_short128_pair_t;

typedef struct _bfloat128_ushort128_pair_t {
  bfloat128 v1;
  ushort128 v2;
} bfloat128_ushort128_pair_t;
#endif

// half
#if defined(__gaudib__) || defined(__greco__) || defined(__gaudi2__) || defined(__doron1__)
typedef struct _half128_bfloat128_pair_t {
  half128 v1;
  bfloat128 v2;
} half128_bfloat128_pair_t;

typedef struct _half128_pair_t {
  half128 v1;
  half128 v2;
} half128_pair_t;
typedef struct _half128_pair_t half128_half128_pair_t;
typedef struct _half128_pair_t half256;

typedef struct _half128_short128_pair_t {
  half128 v1;
  short128 v2;
} half128_short128_pair_t;

typedef struct _half128_ushort128_pair_t {
  half128 v1;
  ushort128 v2;
} half128_ushort128_pair_t;
#endif

// short
#if defined(__gaudi__) || defined(__gaudib__) || defined(__greco__) || defined(__gaudi2__) || defined(__doron1__)
typedef struct _short128_bfloat128_pair_t {
  short128 v1;
  bfloat128 v2;
} short128_bfloat128_pair_t;
#endif

#if defined(__gaudib__) || defined(__greco__) || defined(__gaudi2__) || defined(__doron1__)
typedef struct _short128_half128_pair_t {
  short128 v1;
  half128 v2;
} short128_half128_pair_t;
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


// ushort
#if defined(__gaudi__) || defined(__gaudib__) || defined(__greco__) || defined(__gaudi2__) || defined(__doron1__)
typedef struct _ushort128_bfloat128_pair_t {
  ushort128 v1;
  bfloat128 v2;
} ushort128_bfloat128_pair_t;
#endif

#if defined(__gaudib__) || defined(__greco__) || defined(__gaudi2__) || defined(__doron1__)
typedef struct _ushort128_half128_pair_t {
  ushort128 v1;
  half128 v2;
} ushort128_half128_pair_t;
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

#if defined(__gaudi2__) || defined(__doron1__)
typedef struct _minifloat256_pair_t {
  minifloat256 v1;
  minifloat256 v2;
} minifloat256_pair_t;
typedef struct _minifloat256_pair_t minifloat256_minifloat256_pair_t;
typedef struct _minifloat256_pair_t minifloat512;

typedef struct _minifloat256_minihalf256_pair_t {
  minifloat256 v1;
  minihalf256 v2;
} minifloat256_minihalf256_pair_t;

typedef struct _minifloat256_char256_pair_t {
  minifloat256 v1;
  char256 v2;
} minifloat256_char256_pair_t;

typedef struct _minifloat256_uchar256_pair_t {
  minifloat256 v1;
  uchar256 v2;
} minifloat256_uchar256_pair_t;

typedef struct _char256_minifloat256_pair_t {
  char256 v1;
  minifloat256 v2;
} char256_minifloat256_pair_t;

typedef struct _uchar256_minifloat256_pair_t {
  uchar256 v1;
  minifloat256 v2;
} uchar256_minifloat256_pair_t;

typedef struct _minihalf256_pair_t {
  minihalf256 v1;
  minihalf256 v2;
} minihalf256_pair_t;
typedef struct _minihalf256_pair_t minihalf256_minihalf256_pair_t;
typedef struct _minihalf256_pair_t minihalf512;

typedef struct _minihalf256_minifloat256_pair_t {
  minihalf256 v1;
  minifloat256 v2;
} minihalf256_minifloat256_pair_t;

typedef struct _minihalf256_char256_pair_t {
  minihalf256 v1;
  char256 v2;
} minihalf256_char256_pair_t;

typedef struct _minihalf256_uchar256_pair_t {
  minihalf256 v1;
  uchar256 v2;
} minihalf256_uchar256_pair_t;

typedef struct _char256_minihalf256_pair_t {
  char256 v1;
  minihalf256 v2;
} char256_minihalf256_pair_t;

typedef struct _uchar256_minihalf256_pair_t {
  uchar256 v1;
  minihalf256 v2;
} uchar256_minihalf256_pair_t;

typedef long squeeze_cntr;
#endif

// Address space specifiers
#define __local__  __local
#define __global__ __global


// Internal functions.
void static inline __attribute__((always_inline)) __initialize();

// It would be nice to remove this definition some day.
typedef int tensor;




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
  SW_INC                = 0x0 << 8,
  SW_DEC                = 0x1 << 8,
  SW_SPU                = 0x0 << 8,
  SW_VPU                = 0x2 << 8,
  // CACHE_INVALIDATE
  SW_SB                 = 0x1 << 0,
  SW_D                  = 0x1 << 1,
  SW_LU                 = 0x1 << 2,
  SW_RST_LU             = 0x1 << 3,
  SW_RST_D_PREF         = 0x1 << 4,
  SW_INV_LOG            = 0x1 << 6,
  // PREFETCH
  SW_L2                 = 0x1 << 3,
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
  SW_SUP_NAN            = 0x1 << 2,
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
  SW_LINEAR             = 0x1 << 24,
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
  SW_DT_FP8_152         = 0x9 << 0,
  SW_DT_FP8_143         = 0xa << 0,
  SW_DT_INT64           = 0xb << 0,
  SW_DT_OVERRIDE        = 0x1 << 4,
  SW_PAD                = 0x1 << 6,
  // CACHE_FLUSH
  SW_NO_INV             = 0x1 << 0,
  // EXTRACT_EXP
  SW_BIASED             = 0x1 << 0,
  SW_PRE_LOG            = 0x1 << 1,
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
  SW_PRE_LOG_FUNC       = (0x5 << (5 + 8)),
  // GET_LUT_ENTRY_AND_INTERVAL_START
  SW_LUT_TANH           = 0x1 << (5 + 8),
  SW_LUT_SQRT_RSQRT     = 0x2 << (5 + 8),
  SW_LUT_SIN_COS        = 0x3 << (5 + 8),
  SW_LUT_LOG            = 0x4 << (5 + 8),
#if defined (__greco__) || defined (__gaudi2__) || defined (__doron1__)
  SW_LUT_OPT            = 1 << 0,
  SW_LUT_EXP0           = 1 << 1,
#endif
  // LD_L / ST_L
  SW_SLM                = 0x0 << 0,
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
#if defined(__greco__) // temporary until kernels fixed
  SW_AUTO_INC           = 0x1 << 0,
  SW_AUTO_INC_1         = (0x0 << 2) | SW_AUTO_INC,
  SW_AUTO_INC_2         = (0x1 << 2) | SW_AUTO_INC,
  SW_AUTO_INC_4         = (0x2 << 2) | SW_AUTO_INC,
  SW_AUTO_INC_8         = (0x3 << 2) | SW_AUTO_INC,
#endif
#if defined(__doron1__)
  SW_EXC                = 0x1 << 9,
#endif

  SW_BV64               = 0x1 << 4,
  SW_L0CS               = 0x1 << 5,
  SW_L0CD               = 0x1 << 5,
  SW_EV_HINT            = 0x1 << 6,
  SW_PD                 = 0x1 << (7 + 8),
  // LD_TNSR
  SW_UNPCK_16_TO_32     = 0x0 << 1,
  SW_UNPCK_8_TO_16      = 0x1 << 1,
  SW_UNPCK_8_TO_32      = 0x2 << 1,
  SW_UNPCK_4_TO_8       = 0x3 << 1,
  SW_UNPACK             = 0x1 << 4,
  /* SW_L0CS - see previous definition */
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
  SW_NO_NEG             = 0x0 << 1,
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
  SW_LOWER32            = 0x0 << 2,
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
  #if defined (__gaudi2__)
  RMW_SET               = 0x1 << 8,
  #else
  RMW_SET               = 0x1 << 6,
  #endif
  // ST_TNSR_SQZ
  SW_CNT_ONLY           = 0x1 << 0,
  SW_FLUSH              = 0x1 << 4,
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
  // READ_LFSR / S_READ_LFSR
  SW_READ_ONLY          = 1,
  #if defined (__doron1__)
  SW_PART_LOW           = 0x0 << 4,
  SW_PART_HIGH          = 0x1 << 4,
  
  SW_INC_DIM0           = 0x0 << 8,
  SW_INC_DIM1           = 0x1 << 8,
  SW_INC_DIM2           = 0x2 << 8,
  SW_INC_DIM3           = 0x3 << 8,
  SW_INC_DIM4           = 0x4 << 8,
  #endif
};

// Instruction switches. Must agree with definitions in 'lib/Target/TPC/Utils/InstructionDB.h'.
enum {
  #if defined(__gaudib__)
  SW_FP16_FTZ_IN     = 1U << 0,
  SW_CLIP_FP16       = 1U << 7,
  SW_UNPACK_LANE     = 1U << 11,
  #endif
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
  SW_NUM_LANES_SRCB  = 1U << 6,
  SW_LOWER_LANES     = 2U << 6,
  SW_UPPER_LANES     = 3U << 6,
  SW_ALL_LANES_SRCB     = 0,
  SW_SINGLE_LANE_SRCB   = SW_NUM_LANES_SRCB,
#if defined (__greco__) || defined (__gaudi2__) || defined (__doron1__)
  SW_ANDN            = 1 << 1,
#endif

#if defined (__gaudi2__) || defined(__doron1__)
  SW_DIRECT         = 1 << 6,
#endif

#if defined (__gaudi2__) | defined(__doron1__)
  SW_CLIP_X2        = 0x080000,
  SW_IRF44_HIGH = 1,
#endif

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
  SW_UINT16   = 8,
  SW_INT4     = 9,
  SW_UINT4    = 10,
  SW_FP16     = 11,
  SW_FP8_152  = 12,
  SW_FP8_143  = 13,
  SW_INT64    = 14
};

#define MkWrA(w0, w1, w2, w3)  (((w0) << 16) | ((w1) << 18) | ((w2) << 20) | ((w3) << 22))

#define MkWr(l, u)  (((l) ? SW_WR_LOWER_GROUP : 0) | ((u) ? SW_WR_UPPER_GROUP : 0))

#if defined(__gaudi2__) || defined(__doron1__)
  #define MkRMW(dt, op, rmw, dl)  ((dt) | ((op) << 4) | ((rmw) << 8) | ((dl) << 7))
#else
  #define MkRMW(dt, op, rmw, dl)  ((dt) | ((op) << 4) | ((rmw) << 6) | ((dl) << 7))
#endif

inline unsigned MdgCtrlSingle(unsigned src_dual_group, unsigned dst_dual_group, unsigned write_lower_group, unsigned write_upper_group) {
  return (src_dual_group | (dst_dual_group << 2) | (write_lower_group << 4) | (write_upper_group << 5));
}

inline unsigned MdgCtrlAll(unsigned src_dual_group_0, unsigned src_dual_group_1, unsigned src_dual_group_2, unsigned src_dual_group_3) {
  return (src_dual_group_0 | (src_dual_group_1 << 2) | (src_dual_group_2 << 4) | (src_dual_group_3 << 6));
}

// CONVERT
// for FP32 to BF16/FP16, when using tpc-c, there is a switch of ALL_LANES,
// which translates the instruction into 2 assembly instructions
// SINGLE_LANE with LANE_SEL=0 and SINGLE_LANE with LANE_SEL=
#if defined(__gaudib__) || defined(__gaudi__)
#define v_convert_f32_to_bf16_all_vb(src, switches, income, predicate,         \
                                     polarity)                                 \
  v_bf16_mov_vb(v_convert_f32_to_bf16_all_b(src, switches,income,1,0),0,       \
                income,predicate, polarity                                     \
              )
#endif

// Preloaded registers
#define LFSR              read_lfsr_b()
#define LFSR_NO_CHANGE    read_lfsr_b(SW_READ_ONLY)
#define S_LFSR            s_read_lfsr()
#define S_LFSR_NO_CHANGE  s_read_lfsr(SW_READ_ONLY)
#define V_LANE_ID_32      read_lane_id_4b_b()
#define V_LANE_ID_16      read_lane_id_2b_b()
#define V_LANE_ID_8       read_lane_id_1b_b()

#endif

#if defined(__gaudi__) || defined(__gaudib__) || defined(__greco__) || defined(__gaudi2__) || defined(__doron1__)
#define UPPER_HALF (1 << 2)
#if defined(__greco__) || defined(__gaudi2__) || defined(__doron1__)
#define LUT_PTR (1 << 3)
#define X2      (1 << 4)

#endif
#endif
