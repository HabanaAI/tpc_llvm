//===--- tpc-special.h-----------------------------------------*- TPC-C-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//
#if defined(INCLUDE_TPC_SPECIAL_H) && !defined(TPC_SPECIAL_H_INCLUDED)
#define TPC_SPECIAL_H_INCLUDED

// These are definitions previously defined in 'tpc-defs.h'. They are not part
// of compiler support library, just only definitions that are wanted to be
// available without inclusion of any header file.

typedef enum _e_round_mode {
  e_round_half_ne = 0,
  e_round_zero = 1,
  e_round_up = 2,
  e_round_down = 3,
  e_round_stochastic = 4,
  e_round_half_az = 6
} e_round_mode;

typedef enum _e_compare_value {
  e_compare_bit_0 = 0,
  e_compare_bit_1 = 1
} e_compare_value;

typedef enum _e_search_dir {
  e_start_from_lsb = 0,
  e_start_from_msb = 1
} e_search_dir;

typedef enum _e_mov_flavor {
  e_bits_0_31 = 0,
  e_bits_32_63 = 1,
  e_bits_64_95 = 2,
  e_bits_96_127 = 3,
  e_bits_128_159 = 4,
  e_bits_160_191 = 5,
  e_bits_192_223 = 6,
  e_bits_224_255 = 7,
  e_standard_mov = 8
} e_mov_flavor;

typedef enum _e_saturation {
  e_no_saturation = 0,
  e_saturate = 1
} e_saturation;

typedef enum _e_negation {
  e_no_negation = 0,
  e_with_negation = 1
} e_negation;

typedef enum _e_aso_op {
  e_aso_increment = 0,
  e_aso_decrement = 1
} e_aso_op;

typedef enum _e_aso_access_type {
  e_aso_prefetch = 0,
  e_aso_commit = 1
}  e_aso_access_type;

typedef enum _e_return_bits {
  e_return_lower_32 = 0,
  e_return_upper_32 = 1
} e_return_bits;

typedef enum _e_count_type {
  e_count_zeros = 0,
  e_count_ones = 1
} e_count_type;

typedef enum _e_mul_behavior {
  e_mul_default = 0,
  e_doube_and_round = 1
} e_mul_behavior;

typedef enum _e_group_source {
  e_group_0 = 0,
  e_group_1 = 1
} e_group_source;

typedef enum _e_element_stride {
  e_every_second_element = 0,
  e_every_forth_element = 1
} e_element_stride;

typedef enum _e_group_half {
  e_lower_half_group = 0,
  e_higher_half_group = 1
} e_group_half;

#ifdef __goya__
typedef enum _e_lookup_data_type {
  e_lookup_fp32 = 0,
  e_lookup_reserved = 1,
  e_lookup_fp16_low = 2,
  e_lookup_fp16_high = 3,
  e_lookup_int8_0 = 4,
  e_lookup_int8_1 = 5,
  e_lookup_int8_2 = 6,
  e_lookup_int8_3 = 7
} e_lookup_data_type;
#elif defined(__gaudi__)
typedef enum _e_lookup_data_type {
  e_lookup_fp32 = 0,
  e_lookup_bv16 = 1
} e_lookup_data_type;
#else
#error "Undefined processor"
#endif

#ifdef __goya__
typedef enum _e_function_id
{
    e_fp32_tanh             = 0,
    e_fp32_rsqrt            = 1,
    e_fp32_log2             = 2,
    e_fp32_sqrt             = 16,
    e_fp32_rcp              = 17,
    e_fp32_sin_cos          = 18,
    e_fp32_pow2_128         = 19,
    e_fp16_tanh_128         = 20,
    e_i16_tanh_128          = 21,
    e_i16_sigmoid_128       = 22,
    e_i16_exp_nep_128       = 23,
    e_i16_rcp_128           = 24,
    e_i8_tanh_linear_128    = 25,
    e_i8_sigmoid_linear_128 = 26,
    e_i8_exp_linear_128     = 27,
    e_i8_tanh_128           = 28,
    e_fp32_pow2             = 32,
    e_fp16_tanh             = 33,
    e_i16_tanh              = 34,
    e_i16_sigmoid           = 35,
    e_i16_exp_nep           = 36,
    e_i16_rcp               = 37,
    e_i8_tanh_linear        = 38,
    e_i8_sigmoid_linear     = 39,
    e_i8_exp_linear         = 40,
    e_i16_gelu_minus_relu   = 41,
    e_fp16_rsqrt            = 48,
    e_fp16_log2             = 49,
    e_fp16_sqrt             = 50,
    e_fp16_rcp              = 51,
    e_fp16_sin_cos          = 52,
    e_fp16_pow2             = 53,
    e_i8_tanh               = 54
} e_function_id;
#elif defined(__gaudi__)
typedef enum _e_function_id {
  e_fp32_tanh = 0,
  e_fp32_rsqrt = 1,
  e_fp32_log2 = 2,
  e_i8_sqrt = 3,
  e_u16_sqrt = 4,
  e_u32_power4 = 5,
  e_i8_sqrt_c0 = 6,
  e_u16_sqrt_c0 = 7,
  e_fp32_sqrt = 128,
  e_fp32_rcp = 129,
  e_fp32_sin_cos = 130,
  e_bf16_rcp_scalar_m7 = 131,
  e_bf16_sin_cos_scalar_m7 = 132,
  e_bf16_sin_cos_linear_m6 = 133,
  e_bf16_sin_cos_poly2_m6_c2c1 = 134,
  e_bf16_sin_cos_poly2_m6_c0 = 135,
  e_bf16_log2_linear_interleaved_m5 = 136,
  e_fp32_pow2 = 256,
  e_bf16_tanh = 257,
  e_i16_tanh = 258,
  e_i16_sigmoid = 259,
  e_i16_exp_neg = 260,
  e_i16_rcp = 261,
  e_i8_tanh_linear = 262,
  e_i8_sigmoid_linear = 263,
  e_i8_exp_linear = 264,
  e_bf16_tanh_c0 = 265,
  e_i16_tanh_c0 = 266,
  e_i16_sigmoid_c0 = 267,
  e_i16_exp_neg_c0 = 268,
  e_i16_rcp_c0 = 269,
  e_i8_tanh_linear_c0 = 270,
  e_i8_sigmoid_linear_c0 = 271,
  e_i8_exp_linear_c0 = 272,
  e_bf16_tanh_linear_m4 = 273,
  e_bf16_tanh_poly2_m4_c2c1 = 274,
  e_bf16_tanh_poly2_m4_c0 = 275,
  e_bf16_sqrt_scalar_m6 = 276,
  e_bf16_log2_linear_m5 = 277,
  e_bf16_sin_cos_linear_m5 = 278,
  e_bf16_sin_cos_poly_m5_c2c1 = 279,
  e_bf16_sin_cos_poly2_m5_co = 280,
  e_bf16_rcp_m7_c0 = 288,
  e_bf16_rsqrt = 384,
  e_bf16_log2 = 385,
  e_bf16_sqrt = 386,
  e_f16_sqrt_poly_m2_c2c1 = 386, // sqrt_coeffs_fp16 table is same as sqrt_coeffs_bf16
  e_bf16_rcp = 387,
  e_bf16_sin_cos = 388,
  e_bf16_pow2 = 389,
  e_i8_tanh = 390,
  e_bf16_rsqrt_c0 = 391,
  e_bf16_log2_c0 = 392,
  e_bf16_sqrt_c0 = 393,
  e_f16_sqrt_poly_m2_c0 = 393, // sqrt_coeffs_fp16 table is same as sqrt_coeffs_bf16
  e_bf16_rcp_c0 = 394,
  e_bf16_sin_cos_c0 = 395,
  e_bf16_pow2_c0 = 396,
  e_i8_tanh_c0 = 397,
  e_bf16_rcp_scalar = 398,
  e_f16_rcp_poly_m3_c2c1 = 398, // recip_coeffs_fp16 table is same as recip_coeffs_bf16
  e_bf16_rcp_linear_m2 = 399,
  e_bf16_rcp_linear_m3 = 400,
  e_bf16_rcp_linear_m4 = 401,
  e_bf16_rcp_poly_m2_c2c1 = 402,
  e_bf16_rcp_poly_m3_c2c1 = 403,
  e_bf16_rcp_poly_m4_c2c1 = 404,
  e_bf16_rcp_poly_m2_c0 = 405,
  e_bf16_rcp_poly_m3_c0 = 406,
  e_bf16_rcp_poly_m4_c0 = 407,
  e_f16_rcp_poly_m3_c0 = 408,
  e_bf16_sqrt_linear_m2 = 409,
  e_bf16_sqrt_linear_m3 = 410,
  e_bf16_sqrt_linear_m4 = 411,
  e_bf16_sqrt_poly2_m2_c2c1 = 412,
  e_bf16_sqrt_poly2_m3_c2c1 = 413,
  e_bf16_sqrt_poly2_m4_c2c1 = 414,
  e_bf16_sqrt_poly2_m2_c0 = 415,
  e_bf16_sqrt_poly2_m3_c0 = 416,
  e_bf16_sqrt_poly2_m4_c0 = 417,
  e_bf16_tanh_linear_m2 = 418,
  e_bf16_tanh_linear_m3 = 419,
  e_bf16_tanh_poly2_m2_c2c1 = 421,
  e_bf16_tanh_poly2_m3_c2c1 = 422,
  e_bf16_tanh_poly2_m2_c0 = 424,
  e_bf16_tanh_poly2_m3_c0 = 425,
  e_bf16_log2ml_linear_m4 = 426,
  e_bf16_sin_cos_linear_m2 = 427,
  e_bf16_sin_cos_linear_m3 = 428,
  e_bf16_sin_cos_linear_m4 = 429,
  e_bf16_sin_cos_poly2_m2_c2c1 = 430,
  e_bf16_sin_cos_poly2_m2_c0 = 431,
  e_bf16_sin_cos_poly2_m3_c2c1 = 432,
  e_bf16_sin_cos_poly2_m3_c0 = 433,
  e_bf16_sin_cos_poly2_m4_c2c1 = 434,
  e_bf16_sin_cos_poly2_m4_c0 = 435,
  e_f16_rsqrt_poly_m3_c2c1 = 440,
  e_f16_rsqrt_poly_m3_c0 = 441,
  e_f16_pow2_poly_m2_c2c1 = 444,
  e_f16_pow2_poly_m2_c0 = 445,
} e_function_id;
#else
#error "Undefined processor"
#endif

typedef enum _e_abc_int_norm_sel {
  e_normalize_ab = 0,
  e_normalize_c = 1
} e_abc_int_norm_sel;

typedef enum _e_func_variant {
  e_func_variant_default = 0,
  e_func_variant_tanh = 1,
  e_func_variant_sqrt_rsqrt = 2,
  e_func_variant_sin_cos = 3
} e_func_variant;

typedef struct _permute_t {
  unsigned int bitfielv4;
} permute_t;

typedef enum _e_fp_special_values {
  e_fp_nan = 0,
  e_fp_pos_inf = 1,
  e_fp_neg_inf = 2,
  e_fp_negative = 3,
  e_fp_zero = 4,
  e_fp_positive = 5,
  e_fp_denormalized = 6
} e_fp_special_values;

typedef struct _fp_special_values_t {
  unsigned int bitfielv4;
} fp_special_values_t;

typedef enum _e_prefetch_level {
  e_prefetch_l1 = 0,
  e_prefetch_l2 = 1,
  e_prefetch_l3 = 3,
} e_prefetch_level;

typedef enum _e_access_type {
  e_access_slm = 0,
  e_access_mmio = 1
} e_access_type;

#if defined(__gaudi__)
typedef enum _e_calc_fp_special {
  e_fp_recip = 0,
  e_fp_rsqrt = 1,
  e_fp_sqrt = 2,
  e_fp_log = 3,
  e_fp_exp = 4,
  e_fp_tanh = 5,
  e_fp_div = 6,
  e_fp_pow = 7
} e_calc_fp_special;

typedef enum _e_rmw_datatype {
  e_rmw_int8 = 0,
  e_rmw_int16 = 1,
  e_rmw_int32 = 2,
  e_rmw_uint8 = 3,
  e_rmw_uint16 = 4,
  e_rmw_uint32 = 5,
  e_rmw_bf16 = 6,
  e_rmw_fp32 = 7
} e_rmw_datatype;

typedef enum _e_rmw_op {
  e_rmw_add = 0,
  e_rmw_sub = 1,
  e_rmw_min = 2,
  e_rmw_max = 3
} e_rmw_op;

typedef enum _e_rmw_set {
  e_rmw_plain = 0,
  e_rmw_atomic = 1
} e_rmw_set;

typedef enum _e_tnsr_dt_location {
  e_tnsr_dt_srf = 0,
  e_tnsr_dt_desc = 1
} e_tnsr_dt_location;
#endif

#ifndef NO_TPC_PRINTF

#define PP_ARG(...) PP_ARG_(__VA_ARGS__, PP_DEFAULT())
#define PP_ARG_(...) PP_ARG_2(__VA_ARGS__)
#define PP_ARG_2(x,y,...) x,y
#ifdef __cplusplus
#  define PP_DEFAULT() false
#else
#  define PP_DEFAULT() (_Bool)0
#endif
#define PP_mediate(...) printf_2(__VA_ARGS__)
#define printf(...) PP_mediate(PP_ARG(__VA_ARGS__))

#if defined(__gaudi__)
#define printf_2(x, y)                  \
  _Generic((y),                         \
    _BFloat16      : printf_bf(x,y),    \
    float          : printf_f(x, y),    \
    int            : printf_i(x, y),    \
    unsigned       : printf_ui(x, y),   \
    short          : printf_s(x, y),    \
    unsigned short : printf_us(x, y),   \
    char           : printf_c(x, y),    \
    unsigned char  : printf_uc(x, y),   \
    default        : printf_st(x))
#else //dali
#define printf_2(x, y)                  \
  _Generic((y),                         \
    float          : printf_f(x, y),    \
    int            : printf_i(x, y),    \
    unsigned       : printf_ui(x, y),   \
    short          : printf_s(x, y),    \
    unsigned short : printf_us(x, y),   \
    char           : printf_c(x, y),    \
    unsigned char  : printf_uc(x, y),   \
    default        : printf_st(x))
#endif

#endif

//////////////////////////////////////////
// Tensor/program query functions
#if defined(__gaudi__) || defined(__goya__)
#define c_tensors_base                      0x400U
#endif

#define c_tensor_addr_pad_offset            0x8U
#define c_tensor_size_stride_arr_offset     0x10U
#define c_dim_stride_offset                 0x4U

#ifdef __goya__
#define c_tensor_desc_size                  0x4cU
#define c_tensor_size_stride_element_size   0xcU
#define c_semaphore_addr                    0x808
#elif defined(__gaudi__)
#define c_tensor_desc_size                  0x38U
#define c_tensor_size_stride_element_size   0x8U
#define c_semaphore_addr                    0x908
#else
#error "Undefined processor"
#endif

inline unsigned int get_dim_size_offset_internal(int a, unsigned int dim) {
  unsigned int address = c_tensors_base +
    a* c_tensor_desc_size +
    c_tensor_size_stride_arr_offset +
    dim * c_tensor_size_stride_element_size;
  return address;
}

inline unsigned int get_tensor_pad_offset_internal(int a) {
  unsigned int address = c_tensors_base +
    a * c_tensor_desc_size +
    c_tensor_addr_pad_offset;
  return address;
}

inline unsigned int get_dim_size(int a, unsigned int dim) {
  unsigned int address = get_dim_size_offset_internal(a, dim);
  unsigned int result = 0;
  result = s_u32_ld_l_s_b(address, result, e_access_mmio, 1, 0);
  return result;
}

inline unsigned int get_dim_stride(int a, unsigned int dim) {
  unsigned int address = get_dim_size_offset_internal(a, dim) + c_dim_stride_offset;
  unsigned int result = 0;
  result = s_u32_ld_l_s_b(address, result, e_access_mmio, 1, 0);
  return result;
}

inline void set_dim_size(int a, unsigned int dim, unsigned int val) {
  unsigned int address = get_dim_size_offset_internal(a, dim);
  u32_st_l_s_s_b(address, val, e_access_mmio, 1, 0);
}

inline void set_dim_stride(int a, unsigned int dim, unsigned int val) {
  unsigned int address = get_dim_size_offset_internal(a, dim) + c_dim_stride_offset;
  u32_st_l_s_s_b(address, val, e_access_mmio, 1, 0);
}

inline unsigned int get_pad_value_uint(int a) {
  unsigned int address = get_tensor_pad_offset_internal(a);
  unsigned int result = 0;
  result = s_u32_ld_l_s_b(address, result, e_access_mmio, 1, 0);
  return result;
}

inline int get_pad_value_int(int a) {
  unsigned int address = get_tensor_pad_offset_internal(a);
  int result = 0;
  result = s_i32_ld_l_s_b(address, result, e_access_mmio, 1, 0);
  return result;
}

inline float get_pad_value_float(int a) {
  unsigned int address = get_tensor_pad_offset_internal(a);
  float result = 0;
  result = s_f32_ld_l_s_b(address, result, e_access_mmio, 1, 0);
  return result;
}

#if defined(__gaudi__)
inline float get_pad_value_bf16(int a) {
  unsigned int address = get_tensor_pad_offset_internal(a);
  float result = 0;
  result = s_bf16_ld_l_s_b(address, result, e_access_mmio, 1, 0);
  return result;
}
#endif


inline short get_pad_value_short(int a) {
  unsigned int address = get_tensor_pad_offset_internal(a);
  short result = 0;
  result = s_i16_ld_l_s_b(address, result, e_access_mmio, 1, 0);
  return result;
}

inline unsigned short get_pad_value_ushort(int a) {
  unsigned int address = get_tensor_pad_offset_internal(a);
  unsigned short result = 0;
  result = s_u16_ld_l_s_b(address, result, e_access_mmio, 1, 0);
  return result;
}

inline char get_pad_value_char(int a) {
  unsigned int address = get_tensor_pad_offset_internal(a);
  char result = 0;
  result = s_i8_ld_l_s_b(address, result, e_access_mmio, 1, 0);
  return result;
}

inline unsigned char get_pad_value_uchar(int a) {
  unsigned int address = get_tensor_pad_offset_internal(a);
  unsigned char result = 0;
  result = s_u8_ld_l_s_b(address, result, e_access_mmio, 1, 0);
  return result;
}

inline void set_pad_value_int(int a, int val) {
  unsigned int address = get_tensor_pad_offset_internal(a);
  i32_st_l_s_s_b(address, val, e_access_mmio, 1, 0);
}

inline void set_pad_value_uint(int a, unsigned int val) {
  unsigned int address = get_tensor_pad_offset_internal(a);
  u32_st_l_s_s(address, val, e_access_mmio);
}

inline void set_pad_value_float(int a, float val) {
  unsigned int address = get_tensor_pad_offset_internal(a);
  f32_st_l_s_s_b(address, val, e_access_mmio, 1, 0);
}

#if defined(__gaudi__)
inline void set_pad_value_bf16(int a, bf16 val) {
	unsigned new_pad_value = (unsigned) val;
	new_pad_value = new_pad_value & 0x0000FFFF;
	new_pad_value = (new_pad_value << 16) | new_pad_value;
	set_pad_value_uint(a, new_pad_value);
}
#endif

inline void set_pad_value_short(int a, short val) {
	unsigned new_pad_value = (unsigned) val;
	new_pad_value = new_pad_value & 0x0000FFFF;
	new_pad_value = (new_pad_value << 16) | new_pad_value;
	set_pad_value_uint(a, new_pad_value);
}

inline void set_pad_value_char(int a, char val) {
	unsigned new_pad_value = (unsigned) val;
	new_pad_value = new_pad_value & 0x000000FF;
	new_pad_value = (new_pad_value << 8) | new_pad_value;
	new_pad_value = (new_pad_value << 16) | new_pad_value;
	set_pad_value_uint(a, new_pad_value);
}


inline void set_pad_value_uchar(int a, unsigned char val) {
	unsigned new_pad_value = (unsigned) val;
	new_pad_value = new_pad_value & 0x000000FF;
	new_pad_value = (new_pad_value << 8) | new_pad_value;
	new_pad_value = (new_pad_value << 16) | new_pad_value;
	set_pad_value_uint(a, new_pad_value);
}

inline void set_pad_value_ushort(int a, unsigned short val) {
	unsigned new_pad_value = (unsigned) val;
	new_pad_value = new_pad_value & 0x0000FFFF;
	new_pad_value = (new_pad_value << 16) | new_pad_value;
	set_pad_value_uint(a, new_pad_value);
}

inline int get_semaphore_value() {
  int result = s_i32_ld_l_s(c_semaphore_addr, e_access_mmio);
  return result;
}

inline void set_semaphore_value(int val) {
  i32_st_l_s_s(c_semaphore_addr, val,  e_access_mmio);
}


// Implementation of special functions are guarded by separate block. If a user
// defines 'TPC_SPECIAL_FUNCS_INCLUDED', he can use definitions of corresponding
// functions from any other header file included explicitly.
#ifndef TPC_SPECIAL_FUNCS_INCLUDED
#define TPC_SPECIAL_FUNCS_INCLUDED

////////////////////////////////// COMMON FP32 MACROS ////////////////////////////////////

#define false  0
#define true   1

#if defined(__gaudi__)
    #define v_f32_lookup_c0_v   v_f32_lookup_1c_v
    #define v_f32_lookup_c1c2_v   v_f32_lookup_2c_v
#endif

// LOOKUP_AND_MAC and CALC_REDUCED_VALUE are defined as macros due to their use
// of FUNC_ID and COEFF_TAB_SHIFT - constant integer values that needs to be
// known during compilation.
// LOOKUP_AND_MAC VPU ops = 4
#define LOOKUP_AND_MAC(INTERVALS, VALUE, RESULT)                                            \
{                                                                                           \
    float64 C0 = v_f32_lookup_c0_v(INTERVALS, e_lookup_fp32, FUNC_ID);                      \
    float64_pair_t C1C2;                                                                    \
    C1C2 = v_f32_lookup_c1c2_v(INTERVALS, e_lookup_fp32, FUNC_ID);                          \
                                                                                            \
    RESULT = C1C2.v1;                                                                       \
    RESULT = v_f32_mac_v_v(C1C2.v2, VALUE, RESULT, false);                                  \
    C0 = v_f32_mac_v_v(RESULT, VALUE, C0, false);                                           \
    RESULT = C0;                                                                            \
}

// CALC_REDUCED_VALUE VPU ops = LOOKUP_AND_MAC VPU ops + 2 = 6
#define CALC_REDUCED_VALUE(INPUT_VAL, FUNC_VARIANT, RESULT)                                 \
{                                                                                           \
    uint64_float64_pair_t all_coeffs_tab;                                                   \
    all_coeffs_tab = v_f32_get_lut_entry_and_interval_start_v(INPUT_VAL,                    \
                                                              COEFF_TAB_SHIFT,              \
                                                              FUNC_VARIANT);                \
    uint64 intervals = all_coeffs_tab.v1;                                                   \
    float64 value = INPUT_VAL - all_coeffs_tab.v2;                                          \
    LOOKUP_AND_MAC(intervals, value, RESULT)                                                \
}

#define UNIT_VAL            0x3f800000
#define SIGNIFICAND_MASK    0x007fffff
#define EXPONENT_MASK       0x7f800000
#define NAN_FP32            0x7fffffff
#define PLUS_INF_FP32       0x7f800000
#define MINUS_INF_FP32      0xff800000
#define EXP_LOWER           0xc2aeac50
#define EXP_UPPER           0x42b17218
#define FLT_MIN             0x800000
#define FLT_MAX             0x7f7fffff
#define M_UNIT_EXP          0xc0800000

//////////////////////////////////////// EXP_F32 //////////////////////////////////////////////////
float64 v_exp_fast_f32(float64 input)
{
#define COEFF_TAB_SHIFT   17             // 23 - (m = 6)
    const int FUNC_ID = e_fp32_pow2;

    const float ln_2_1 =  0.693359375;
    const float ln_2_2 = -2.12194440e-4;
    const float log2_e =  1.44269502;

    float64 result = 0.5;
    result = v_f32_mac_v_s(input, log2_e, result, false);

    int64 floor = v_convert_f32_to_i32_v(result, e_round_half_ne);
    result = v_convert_i32_to_f32_v(floor, e_round_half_ne);

    float64 x_reduced = input;
    x_reduced = v_f32_mac_v_v(result, ln_2_1, x_reduced, true);
    x_reduced = v_f32_mac_v_v(result, ln_2_2, x_reduced, true);
    x_reduced *= log2_e;

    bool256 x_red_geq_0 = bv_f32_cmp_geq_v_s(x_reduced, 0.0f);
    int64 exponent = 0;
    exponent = v_f32_extract_exp_v_vb(x_reduced, exponent, false, x_red_geq_0, 0);
    exponent = v_i32_min_v_s_vb(-exponent, 24, exponent, x_red_geq_0, 0);

    int64 pos_significand = 0;
    pos_significand = v_i32_and_v_s_vb(*((int64*)&x_reduced), SIGNIFICAND_MASK,
          pos_significand, x_red_geq_0, 0);
    pos_significand = v_i32_or_v_s_vb(pos_significand, 1 << 23, pos_significand, x_red_geq_0, 0);
    pos_significand = v_i32_shr_v_v_vb(pos_significand, exponent, pos_significand, x_red_geq_0, 0);
    pos_significand = v_i32_or_v_s_vb(pos_significand, UNIT_VAL, pos_significand, x_red_geq_0, 0);
    result = *((float64*)&pos_significand);

    result = v_f32_add_v_s_vb(x_reduced, 2.0f, result, x_red_geq_0, 1);
    int64 neg_significand = 0;
    neg_significand = v_i32_and_v_s_vb(*((int64*)&result), SIGNIFICAND_MASK, neg_significand,
                                             x_red_geq_0, 1);
    uint64 neg_add = 0;
    neg_add = v_i32_u32_sel_eq_v_s_v_v_vb(neg_significand, 0, 0, 1, neg_add, x_red_geq_0, 1);

    CALC_REDUCED_VALUE(result, e_func_variant_default, result)

    exponent = v_f32_extract_exp_v(result, true);
    exponent += floor - neg_add;
    result = v_f32_form_fp_num_i8_v_v_v((char256) exponent, result, result, SW_EXP_IS_NUM);

    return result;
#undef COEFF_TAB_SHIFT
}

// exp_f32 VPU ops = exp_fast_f32 VPU ops + 3 = 26+3 = 29
float64 v_exp_f32(float64 input)
{

    float64 result = v_exp_fast_f32(input);

// ====================================
//  Processing special values: spec.exp values, +-inf, nan

    const int64 exp_lower = EXP_LOWER;
    const int64 exp_upper = EXP_UPPER;
    const int64 plus_inf  = PLUS_INF_FP32;

    result = v_f32_f32_sel_leq_v_v_v_v(input, *((float64*)&exp_lower), 0.0f, result);
    result = v_f32_f32_sel_geq_v_v_v_v(input, *((float64*)&exp_upper), *((float64*)&plus_inf), result);
    result = v_u32_f32_sel_grt_v_v_v_v(*((uint64*)&input) & NAN_FP32, PLUS_INF_FP32, input, result);
// ====================================
    return result;
}

/////////////////////////////////////// RECIP_F32 /////////////////////////////////////////////////

float64 v_reciprocal_fast_f32(float64 input)
{
  #define COEFF_TAB_SHIFT   16             // 23 - (m = 7)
    const int FUNC_ID = e_fp32_rcp;

    const char zero_exp = 0;
    float64 result = v_f32_form_fp_num_i8_s_v_v(zero_exp, input, input, SW_ADD_BIAS | SW_FORCE_SIGN0 | SW_EXP_IS_NUM);
    CALC_REDUCED_VALUE(result, e_func_variant_default, result)

    int64 res_exp = (*((int64*)&result) & EXPONENT_MASK) - (*((int64*)&input) & EXPONENT_MASK);
    result = v_f32_form_fp_num_v_v_v(*((float64*)&res_exp), input, result, SW_ADD_BIAS);

    float64 signed_zero = v_f32_f32_sel_less_v_v_v_v(input, 0.0f, -0.0f, 0.0f);
    result = v_i32_f32_sel_leq_v_v_v_v(res_exp, M_UNIT_EXP, signed_zero, result);

    return result;
#undef COEFF_TAB_SHIFT
}

float64 v_reciprocal_f32(float64 input)
{
    float64 result = v_reciprocal_fast_f32(input);

// ====================================
//  Processing special values: denorm, +-0. +-inf, nan
    float64 abs_x = v_f32_abs_v(input);
    float64 abs_result = v_f32_abs_v(result);
    const uint64 flt_min = FLT_MIN;
    const float64 flt_min_fp32 = *((float64*)&flt_min);
    const uint64 flt_max = FLT_MAX;
    const float64 flt_max_fp32 = *((float64*)&flt_max);
    const uint64 plus_inf = PLUS_INF_FP32;
    const float64 plus_inf_fp32 = *((float64*)&plus_inf);
    const uint64 nan_int = NAN_FP32;
    const float64 nan_fp32 = *((float64*)&nan_int);

    abs_result = v_f32_f32_sel_less_v_v_v_v(abs_x, flt_min_fp32, plus_inf_fp32, abs_result);
    abs_result = v_f32_f32_sel_grt_v_v_v_v(abs_x, flt_max_fp32, 0.0f, abs_result);

    abs_result = v_u32_f32_sel_grt_v_s_v_v(*((uint64*)&abs_x), PLUS_INF_FP32, nan_fp32, abs_result);
    result = v_f32_form_fp_num_v_v_v(abs_result, input, abs_result, 0x0);
// ====================================

    return result;
}

//////////////////////////// COMMON SQRT_F32 / RSQRT_F32/ /////////////////////////////////////
// BASE_SQRT_FUNC VPU Ops = CALC_REDUCED_VALUE + 3 = 6+3=9
#define BASE_SQRT_FUNC(INPUT_VAL, EXPONENT, RESULT)                                             \
{                                                                                               \
    EXPONENT = v_f32_extract_exp_v(INPUT_VAL, false);                                           \
    RESULT = v_f32_form_fp_num_i8_v_v_v((char256) (EXPONENT&1), INPUT_VAL, INPUT_VAL, SW_EXP_IS_NUM|SW_ADD_BIAS);    \
    CALC_REDUCED_VALUE(RESULT, e_func_variant_sqrt_rsqrt, RESULT)                               \
    EXPONENT -= v_i32_i32_sel_less_v_v_v_v(EXPONENT, 0, EXPONENT&1, 0);                         \
}



//////////////////////////////////////// SQRT_F32 /////////////////////////////////////////////////
float64 v_sqrt_fast_f32(float64 input)
{
#define COEFF_TAB_SHIFT   17                // 23 - (m = 6)
    const int FUNC_ID = e_fp32_sqrt;
    int64 exponent;
    float64 result;
    BASE_SQRT_FUNC(input, exponent, result)

    exponent = *((int64*)&result) + ((exponent >> 1) << 23);
    result = *((float64*)&exponent);
    result = v_f32_f32_sel_eq_v_v_v_v(input, 0.0f, 0.0f, result);
    return result;
#undef COEFF_TAB_SHIFT
}

float64 v_sqrt_f32(float64 input)
{
    float64 result = v_sqrt_fast_f32(input);

// ====================================
//  Processing special values: <0. +inf, nan
    const uint64 plus_inf = PLUS_INF_FP32;
    const float64 plus_inf_fp32 = *((float64*)&plus_inf);
    const uint64 nan_int = NAN_FP32;
    const float64 nan_fp32 = *((float64*)&nan_int);

    result = v_u32_f32_sel_eq_v_s_v_v(*((uint64*)&input), PLUS_INF_FP32, plus_inf_fp32, result);
    result = v_u32_f32_sel_grt_v_s_v_v(*((uint64*)&input), PLUS_INF_FP32, nan_fp32, result);
    result = v_f32_f32_sel_eq_v_v_v_v(input, -0.0f, -0.0f, result);
// ====================================

    return result;
}

//////////////////////////////////////// RSQRT_F32 /////////////////////////////////////////////////
float64 v_rsqrt_fast_f32(float64 input)
{
#define COEFF_TAB_SHIFT   16             // 23 - (m = 7)
    const int FUNC_ID = e_fp32_rsqrt;
    int64 exponent;
    float64 result;
    BASE_SQRT_FUNC(input, exponent, result)

    exponent = *((int64*)&result) - ((exponent >> 1) << 23);
    result = *((float64*)&exponent);
    return result;
#undef COEFF_TAB_SHIFT
}

float64 v_rsqrt_f32(float64 input)
{
    float64 result = v_rsqrt_fast_f32(input);

// ====================================
//  Processing special values: <=0. +inf, nan
    const uint64 plus_inf = PLUS_INF_FP32;
    const float64 plus_inf_fp32 = *((float64*)&plus_inf);

    const uint64 minus_inf = MINUS_INF_FP32;
    const float64 minus_inf_fp32 = *((float64*)&minus_inf);

    const uint64 nan_int = NAN_FP32;
    const float64 nan_fp32 = *((float64*)&nan_int);

    result = v_f32_f32_sel_eq_v_s_v_v(input, 0.0f, plus_inf_fp32, result);
    //result = v_f32_f32_sel_less_v_s_v_v(input, 0.0f, nan_fp32, result);
    result = v_u32_f32_sel_eq_v_s_v_v(*((uint64*)&input), PLUS_INF_FP32, 0.0f, result);
    result = v_u32_f32_sel_grt_v_s_v_v(*((uint64*)&input), PLUS_INF_FP32, nan_fp32, result);
    result = v_f32_f32_sel_eq_v_v_v_v(input, -0.0f, minus_inf_fp32, result);
// ====================================

    return result;
}

//////////////////////////////////////// LOG_F32 GAUDI ///////////////////////////////////////////

/*
float64 log_f32(float64 input)
{//  log32_Gaudi
    const float ln_2 = 0.69314718056;      // 0x3f317218
    int64 exponent = v_f32_extract_exp_v(input, false);
    float64 fl_exponent = v_convert_i32_to_f32_v(exponent, e_round_half_ne) ;
    const char zero_exp = 0;
    float64 result = v_f32_form_fp_num_i8_s_v_v(zero_exp, input, input, SW_EXP_IS_NUM|SW_ADD_BIAS);
    bool256 zero_exp_pred = bv_i32_cmp_eq_v_s(exponent, 0);
    float64 tmp_result = result;

    const int COEFF_TAB_SHIFT = 16;             // 23 - (m = 7)
    const int FUNC_ID = e_fp32_log2;

    uint64_float64_pair_t all_coeffs_tab;
    all_coeffs_tab = v_f32_get_lut_entry_and_interval_start_v(result, COEFF_TAB_SHIFT,
                                                              e_func_variant_default);

    uint64 intervals = all_coeffs_tab.v1;
    intervals = v_u32_add_v_s_vb(intervals, 128, intervals, e_no_saturation, zero_exp_pred, 1);
    float64 value = result - all_coeffs_tab.v2;
    LOOKUP_AND_MAC(intervals, value, result)

    float64 res_minus_one = tmp_result - 1.0f;
    result = v_f32_mul_v_v_vb(res_minus_one, result, result, zero_exp_pred, 0);

    result += fl_exponent;
    result *= ln_2;

// ====================================
//  Processing special values: <=0, +-inf, nan
    const uint64 plus_inf = PLUS_INF_FP32;
    const float64 plus_inf_fp32 = *((float64*)&plus_inf);
    const uint64 minus_inf = MINUS_INF_FP32;
    const float64 minus_inf_fp32 = *((float64*)&minus_inf);
    const uint64 nan_int = NAN_FP32;
    const float64 nan_fp32 = *((float64*)&nan_int);

    result = v_f32_f32_sel_less_v_s_v_v(input, 0.0f, nan_fp32, result);
    result = v_f32_f32_sel_eq_v_s_v_v(input, 0.0f, minus_inf_fp32, result);
    result = v_u32_f32_sel_eq_v_s_v_v(*((uint64*)&input), PLUS_INF_FP32, plus_inf_fp32, result);
    result = v_i32_f32_sel_grt_v_s_v_v(*((int64*)&input), PLUS_INF_FP32, nan_fp32, result);
// ====================================

    return result;
} //  log_f32_Gaudi
*/
//////////////////////////////////////// LOG_F32 CEPHES///////////////////////////////////////////
float64 v_log_fast_f32(float64 input)
{
    //  log32_cephes: log(1+x) = x - 0.5 x**2 + x**3 P(x)
    const float log2_e_m_1 = 0.44269504088896340735992; // log2(e) - 1.0
    const float ln_2 = 0.69314718056;                   // ln(2)        (0x3f317218)
    const float one_sqrt_2 = 0.70710677;                // 1/sqrt(2)    (0x3f3504f3)
    const int poly_tab_size = 9;
    const float coeffs[poly_tab_size] = {
        7.0376836292E-2,
        -1.1514610310E-1,
        1.1676998740E-1,
        -1.2420140846E-1,
        1.4249322787E-1,
        -1.6668057665E-1,
        2.0000714765E-1,
        -2.4999993993E-1,
        3.3333331174E-1};

    int64 exponent = v_f32_extract_exp_v(input, false) + 1;
    const char exp_126 = 126;
    float64 fraction = v_f32_form_fp_num_i8_s_v_v(exp_126, input, input, SW_EXP_IS_NUM);
    float64 fl_exponent = v_convert_i32_to_f32_v(exponent, e_round_half_ne) ;

    float64 diff = fraction - one_sqrt_2;
    int64 diff_sign = v_i32_shr_v_s((*(int64 *) &diff), 31);
    exponent -= diff_sign;
    fl_exponent = v_convert_i32_to_f32_v(exponent, e_round_half_ne) ;
    float64 fl_diff_sign = v_convert_i32_to_f32_v(diff_sign, e_round_half_ne) ;
    fraction += fl_diff_sign * fraction - 1.0f;

    float64 x_sqr = fraction * fraction;
    float64 poly = coeffs[0];
    for (int i = 1; i < poly_tab_size; i++)
        poly = v_f32_mac_v_v(poly, fraction, coeffs[i], false);

    float64 tailor = fraction * (x_sqr * poly);
    tailor -= 0.5 * x_sqr;

    float64 result = tailor * log2_e_m_1;
    result += fraction * log2_e_m_1;
    result += tailor;
    result += fraction;

    result += fl_exponent;
    result *= ln_2;

    return result;
}

float64 v_log_f32(float64 input)
{
    float64 result = v_log_fast_f32(input);
// ====================================
//  Processing special values: <=0, +-inf, nan
    const uint64 plus_inf = PLUS_INF_FP32;
    const float64 plus_inf_fp32 = *((float64*)&plus_inf);
    const uint64 minus_inf = MINUS_INF_FP32;
    const float64 minus_inf_fp32 = *((float64*)&minus_inf);
    const uint64 nan_int = NAN_FP32;
    const float64 nan_fp32 = *((float64*)&nan_int);

    result = v_f32_f32_sel_less_v_s_v_v(input, 0.0f, nan_fp32, result);
    result = v_f32_f32_sel_eq_v_s_v_v(input, 0.0f, minus_inf_fp32, result);
    result = v_u32_f32_sel_eq_v_s_v_v(*((uint64*)&input), PLUS_INF_FP32, plus_inf_fp32, result);
    result = v_i32_f32_sel_grt_v_s_v_v(*((int64*)&input), PLUS_INF_FP32, nan_fp32, result);
// ====================================

    return result;
} // log_f32_cephes

//   Special log values:
//     x < 0.0 -> return nan
//     x = -inf -> return nan
//   x = -0.0 -> return nan
//   x = 0.0 -> return -inf
//   x = +inf -> return +inf
//     x = nan -> return nan

////////////////////////////////// COMMON SIN_COS_F32 //////////////////////////////////////
// 23 + 4 = 27
#define SIN_COS_CALC(SIN_X_COND)                                                                    \
    const float four_by_pi = 1.27323949;    /* 4/pi = 0x3fa2f983 */                                 \
    const float pi4_1 = 7.85156250e-01;        /* pi/4 = 0x3f490000 */                              \
    const float pi4_2 = 2.41875648498e-4;    /* 0x397da000 */                                       \
    const float pi4_3 = 3.7748949774e-8;    /* 0x3fa2f983*/                                         \
                                                                                                    \
    float64 abs_x = v_f32_abs_v(input);                                                             \
    float64 fl_pi4_shift = abs_x * four_by_pi;                                                      \
    int64 pi4_shift = v_convert_f32_to_i32_v(fl_pi4_shift, e_round_down);                           \
    pi4_shift += pi4_shift & 1;             /* Shift x in [-pi/4, +pi/4] */                         \
    fl_pi4_shift = v_convert_i32_to_f32_v(pi4_shift, e_round_half_ne);                              \
    float64 reduced_x = abs_x;                                                                      \
    reduced_x = v_f32_mac_v_s(fl_pi4_shift, pi4_1, reduced_x, true);                                \
    reduced_x = v_f32_mac_v_s(fl_pi4_shift, pi4_2, reduced_x, true);                                \
    reduced_x = v_f32_mac_v_s(fl_pi4_shift, pi4_3, reduced_x, true);                                \
    float64 abs_reduced_x = v_f32_abs_v(reduced_x);                                                 \
                                                                                                    \
    int64 pi2_shift = (pi4_shift >> 1) & 3;        /* remove shift by 2*pi: x in [0, 2*pi) */       \
    float64 fl_sign_shift = v_convert_i32_to_f32_v(pi2_shift & 2, e_round_half_ne);                 \
    sign_res -= fl_sign_shift * sign_res;        /* x>pi? -> shift by pi: cos(pi-x) = -cos(x) */    \
    pi2_shift -= pi2_shift & 2;                    /* remove shift by pi -> pi2_shift in [0, 1] */  \
                                                                                                    \
    /*const int COEFF_TAB_SHIFT = 17;             23 - (m = 6) */                                   \
    const int FUNC_ID = e_fp32_sin_cos;                                                             \
                                                                                                    \
    bool256 sin_x = bv_i32_cmp_eq_v_s(pi2_shift, SIN_X_COND);                                       \
    uint64_float64_pair_t all_coeffs_tab;                                                           \
    all_coeffs_tab = v_f32_get_lut_entry_and_interval_start_v(abs_reduced_x, 17 /*COEFF_TAB_SHIFT*/,\
                                                              e_func_variant_sin_cos);              \
    uint64 intervals = all_coeffs_tab.v1;                                                           \
    intervals = v_u32_add_v_s_vb(intervals, 64, intervals, e_no_saturation, sin_x, 1);              \
    float64 value = abs_reduced_x - all_coeffs_tab.v2;                                              \
    float64 result;                                                                                 \
    LOOKUP_AND_MAC(intervals, value, result)                                                        \
                                                                                                    \
    result = v_f32_mul_v_v_vb(abs_reduced_x, result, result, sin_x, 0);

#define PROCESS_SIN_COS_SPECIAL_VALUES                                                              \
    const uint64 nan_int = NAN_FP32;                                                                \
    const float64 nan_fp32 = *((float64*)&nan_int);                                                 \
    const float sin_max_arg = s_convert_i32_to_f32_s(0xffffff, e_round_half_ne),                    \
                sin_accuracy_limit = s_convert_i32_to_f32_s(0x2000, e_round_half_ne);               \
                                                                                                    \
    result = v_f32_f32_sel_grt_v_s_v_v(abs_x, sin_accuracy_limit, 0.0f, result);                    \
    result = v_f32_f32_sel_grt_v_s_v_v(abs_x, sin_max_arg, nan_fp32, result);                       \
    result = v_u32_f32_sel_geq_v_s_v_v(*((uint64*)&abs_x), PLUS_INF_FP32, nan_fp32, result);

//   Special sin/cos values:
// x = -inf -> return nan
// x -= -0.0 -> return +1.0
// x = +0.0 -> return +1.0
// x = +inf -> return nan
// x = nan -> return nan
// x > sin_max_arg (= 1 << 24 - 1) -> return nan
// x > sin_accuracy_limit (= 1 << 13) -> return 0.0f

//////////////////////////////////////// COS_F32 /////////////////////////////////////////////////
float64 v_cos_fast_f32(float64 input)
{
    float64 sign_res = 1.0f;
    SIN_COS_CALC(1)

    sign_res = v_f32_f32_sel_grt_v_s_v_v_vb(reduced_x, 0.0f, -sign_res, sign_res, sign_res, sin_x, 0);
    result = v_f32_f32_sel_less_v_s_v_v(sign_res, 0.0f, -result, result);

    return result;
}

float64 v_cos_f32(float64 input)
{
    float64 abs_x = v_f32_abs_v(input);
    float64 result = v_cos_fast_f32(input);
// ====================================
//  Processing special values: +-inf, nan, sin/cos limits

    PROCESS_SIN_COS_SPECIAL_VALUES
// ====================================

    return result;
}

//////////////////////////////////////// SIN_F32 /////////////////////////////////////////////////
float64 v_sin_fast_f32(float64 input)
{
    float64 sign_res = v_f32_f32_sel_grt_v_s_v_v(input, 0.0f, 1.0f, -1.0f);
    SIN_COS_CALC(0)

    sign_res = v_f32_f32_sel_less_v_s_v_v_vb(reduced_x, 0.0f, -sign_res, sign_res, sign_res, sin_x, 0);
    result = v_f32_f32_sel_less_v_s_v_v(sign_res, 0.0f, -result, result);
    return result;
}

float64 v_sin_f32(float64 input)
{
    float64 abs_x = v_f32_abs_v(input);
    float64 result = v_sin_fast_f32(input);
// ====================================
//  Processing special values: +-inf, nan, sin/cos limits

    PROCESS_SIN_COS_SPECIAL_VALUES
// ====================================

   return result;
}

//////////////////////////////////////// DIV_F32 /////////////////////////////////////////////////
float64 v_div_fast_f32(float64 input_x, float64 input_y)
{
    float64 result = input_x * v_reciprocal_fast_f32(input_y);

    return result;
}

float64 v_div_f32(float64 input_x, float64 input_y)
{
    float64    result = input_x * v_reciprocal_f32(input_y);

// ====================================
//  Processing special values: 0, +-inf, nan
    const uint64 flt_min = FLT_MIN;
    const float64 flt_min_fp32 = *((float64*)&flt_min);
    const uint64 nan_int = NAN_FP32;
    const float64 nan_fp32 = *((float64*)&nan_int);
    const uint64 plus_inf = PLUS_INF_FP32;
    const float64 plus_inf_fp32 = *((float64*)&plus_inf);

    float64 abs_x = v_f32_abs_v(input_x);                                                           // 15
    float64 abs_y = v_f32_abs_v(input_y);                                                           // 16

// x == nan?
    result = v_f32_f32_sel_grt_v_v_v_v(abs_x, plus_inf_fp32, nan_fp32, result);                     // 17

// sign_x ^ sign_y; (sign_x ^ sign_y) | plus_inf
    int64 sign_res = (((*((int64*)&input_x)) >> 31 ) ^ ((*((int64*)&input_y)) >> 31 )) << 31;       // 18, 19, 20, 21
    const float64 sign_res_fp32 = *((float64*)&sign_res);

//x ==    +-inf?
    float64 inf_x_y = v_f32_or_v_v(sign_res_fp32 ,plus_inf_fp32);                                   // 22
    bool256 inf_x = bv_f32_cmp_eq_v_v(abs_x, plus_inf_fp32);                                        // 23
    result = v_f32_f32_sel_eq_v_v_v_v_vb(abs_y, plus_inf_fp32, nan_fp32, inf_x_y, result, inf_x, 0);// 24

//x == +-0?
    bool256 zero_x = bv_f32_cmp_less_v_v(abs_x, flt_min_fp32);                                      // 25
    result = v_f32_f32_sel_less_v_v_v_v_vb(abs_y, flt_min_fp32, nan_fp32, sign_res_fp32,
                                                                                 result, zero_x, 0);// 26
// ====================================

    return result;
}

// Simlified scalar division (x / y) Markstein's algorithm, very efficient in case of y is actually
// integer values. It assumes that rc (reciprocal of y) comes from outside
float s_div_fast_f32(float x, float y, float rc)
{
    float q = x * rc;

    // Lines below should be called if x isn't +INF or -INF. For simlicity corresponding cheking is missed
    x = s_f32_mac_s_s(y, q, x, e_with_negation);
    q = s_f32_mac_s_s(x, rc, q, e_no_negation);

    return q;
}

//////////////////////////////////////// TANH_F32 /////////////////////////////////////////////////

float64 v_tanh_fast_abs_in_f32(float64 input, float64 abs_x)
{
#define COEFF_TAB_SHIFT   17 // 23 - (m = 6);
    const int FUNC_ID = e_fp32_tanh;

    int64 exponent = v_f32_extract_exp_v(input, false);                                             // 1
    bool256 neg_exp = bv_i32_cmp_less_v_s(exponent, 0);                                             // 2

    float64 result;
    CALC_REDUCED_VALUE(abs_x, e_func_variant_tanh, result)                                          // 3, 4, 5
                                                                                                    // 6, 7, 8
    result = v_f32_mul_v_v_vb(result, abs_x, result, neg_exp, 0);                                   // 9

    bool256 greq_8 = bv_f32_cmp_geq_v_s(abs_x, 8.0f);                                               // 10
    result = v_f32_f32_sel_less_v_s_v_v_vb(abs_x, 9.0f, 0.999999881f, result, result, greq_8, 0);   // 11
    result = v_f32_f32_sel_geq_v_s_v_v(abs_x, 9.0f, 1.0f, result);                                  // 12

    result = v_f32_form_fp_num_v_v_v(result, input, result, 0x0);                                   // 13
    return result;
#undef COEFF_TAB_SHIFT
}

float64 v_tanh_fast_f32(float64 input)
{
    float64 abs_x = v_f32_abs_v(input);                                                             // 1
    float64 result = v_tanh_fast_abs_in_f32(input, abs_x);                                            // 14

    return result;
}

float64 v_tanh_f32(float64 input)
{
    float64 abs_x = v_f32_abs_v(input);                                                             // 1
    float64 result = v_tanh_fast_abs_in_f32(input, abs_x);                                            // 15
// ====================================
//  Processing special values: nan
    const uint64 nan_int = NAN_FP32;
    const float64 nan_fp32 = *((float64*)&nan_int);

    result = v_u32_f32_sel_grt_v_v_v_v(*((uint64*)&abs_x), PLUS_INF_FP32, nan_fp32, result);        // 16
// ====================================

    return result;
}

//////////////////////////////////////// POW2_F32 //////////////////////////////////////////////////

float64 v_pow2_fast_f32(float64 input)
{
  #define COEFF_TAB_SHIFT   17             // 23 - (m = 6);
    const int FUNC_ID = e_fp32_pow2;

    const float log2_1 =  0.0f;
    const float log2_2 =  1.0f;

    float64 result = 0.5f;
    result = v_f32_mac_v_s(input, log2_2, result, false);

    int64 floor = v_convert_f32_to_i32_v(result, e_round_down);
    result = v_convert_i32_to_f32_v(floor, e_round_half_ne);

    float64 x_reduced = input;
    x_reduced = v_f32_mac_v_v(result, log2_2, x_reduced, true);
    x_reduced = v_f32_mac_v_v(result, log2_1, x_reduced, true);
    x_reduced *= log2_2;

    bool256 x_red_geq_0 = bv_f32_cmp_geq_v_s(x_reduced, 0.0f);
    int64 exponent = 0;
    exponent = v_f32_extract_exp_v_vb(x_reduced, exponent, false, x_red_geq_0, 0);
    exponent = v_i32_min_v_s_vb(-exponent, 24, exponent, x_red_geq_0, 0);

    int64 pos_significand = 0;
    pos_significand = v_i32_and_v_s_vb(*((int64*)&x_reduced), SIGNIFICAND_MASK,
          pos_significand, x_red_geq_0, 0);
    pos_significand = v_i32_or_v_s_vb(pos_significand, 1 << 23, pos_significand, x_red_geq_0, 0);
    pos_significand = v_i32_shr_v_v_vb(pos_significand, exponent, pos_significand, x_red_geq_0, 0);
    pos_significand = v_i32_or_v_s_vb(pos_significand, UNIT_VAL, pos_significand, x_red_geq_0, 0);
    result = *((float64*)&pos_significand);

    result = v_f32_add_v_s_vb(x_reduced, 2.0f, result, x_red_geq_0, 1);
    int64 neg_significand = 0;
    neg_significand = v_i32_and_v_s_vb(*((int64*)&result), SIGNIFICAND_MASK, neg_significand,
                                             x_red_geq_0, 1);
    uint64 neg_add = 0;
    neg_add = v_i32_u32_sel_eq_v_s_v_v_vb(neg_significand, 0, 0, 1, neg_add, x_red_geq_0, 1);

    CALC_REDUCED_VALUE(result, e_func_variant_default, result)

    exponent = v_f32_extract_exp_v(result, true);
    exponent += floor - neg_add;
    result = v_f32_form_fp_num_i8_v_v_v((char256) exponent, result, result, SW_EXP_IS_NUM);         // 27
    return result;
#undef COEFF_TAB_SHIFT
}

float64 v_pow2_f32(float64 input)
{
    float64 result = v_pow2_fast_f32(input);
// ====================================
//  Processing special values: spec.exp values, +-inf, nan

    const uint64 flt_max = FLT_MAX;
    const float64 flt_max_fp32 = *((float64*)&flt_max);
    const uint64 plus_inf = PLUS_INF_FP32;
    const float64 plus_inf_fp32 = *((float64*)&plus_inf);

    const float64 pow2_lower = -126.0f;
    const float64 pow2_upper = 128.0f;

    result = v_f32_f32_sel_less_v_v_v_v(input, pow2_lower, 0.0f, result);
    result = v_f32_f32_sel_geq_v_v_v_v(input, pow2_upper, plus_inf_fp32, result);

    result = v_f32_f32_sel_less_v_v_v_v(input, -flt_max_fp32, 0.0f, result);
    result = v_f32_f32_sel_geq_v_v_v_v(input, flt_max_fp32, plus_inf_fp32, result);

    result = v_u32_f32_sel_grt_v_v_v_v(*((uint64*)&input) & NAN_FP32, PLUS_INF_FP32, input, result);
// ====================================

    return result;
}

//////////////////////////////////////// LOG2_F32 CEPHES///////////////////////////////////////////
float64 v_log2_fast_f32(float64 input)
{
    //  log32_cephes: log(1+x) = x - 0.5 x**2 + x**3 P(x)
    const float log2_e_m_1 = 0.44269504088896340735992; // log2(e) - 1.0
    const float one_sqrt_2 = 0.70710677;         // 1/sqrt(2)    (0x3f3504f3)
    const int poly_tab_size = 9;
    const float coeffs[poly_tab_size] = {
        7.0376836292E-2,
        -1.1514610310E-1,
        1.1676998740E-1,
        -1.2420140846E-1,
        1.4249322787E-1,
        -1.6668057665E-1,
        2.0000714765E-1,
        -2.4999993993E-1,
        3.3333331174E-1};

    int64 exponent = v_f32_extract_exp_v(input, false) + 1;
    const char exp_126 = 126;
    float64 fraction = v_f32_form_fp_num_i8_s_v_v(exp_126, input, input, SW_EXP_IS_NUM);            // 2
    float64 fl_exponent = v_convert_i32_to_f32_v(exponent, e_round_half_ne) ;                       // 3

    float64 diff = fraction - one_sqrt_2;
    int64 diff_sign = v_i32_shr_v_s((*(int64 *) &diff), 31);
    exponent -= diff_sign;
    fl_exponent = v_convert_i32_to_f32_v(exponent, e_round_half_ne) ;
    float64 fl_diff_sign = v_convert_i32_to_f32_v(diff_sign, e_round_half_ne) ;
    fraction += fl_diff_sign * fraction - 1.0f;

    float64 x_sqr = fraction * fraction;
    float64 poly = coeffs[0];
    for (int i = 1; i < poly_tab_size; i++)
        poly = v_f32_mac_v_v(poly, fraction, coeffs[i], false);

    float64 tailor = fraction * (x_sqr * poly);
    tailor -= 0.5 * x_sqr;

    float64 result = tailor * log2_e_m_1;
    result += fraction * log2_e_m_1;
    result += tailor;
    result += fraction;

    result += fl_exponent;

    return result;
}

float64 v_log2_f32(float64 input)
{
    float64 result = v_log2_fast_f32(input);
// ====================================
//  Processing special values: <=0, +-inf, nan
    const uint64 plus_inf = PLUS_INF_FP32;
    const float64 plus_inf_fp32 = *((float64*)&plus_inf);
    const uint64 minus_inf = MINUS_INF_FP32;
    const float64 minus_inf_fp32 = *((float64*)&minus_inf);
    const uint64 nan_int = NAN_FP32;
    const float64 nan_fp32 = *((float64*)&nan_int);

    result = v_f32_f32_sel_less_v_s_v_v(input, 0.0f, nan_fp32, result);
    result = v_f32_f32_sel_eq_v_s_v_v(input, 0.0f, minus_inf_fp32, result);
    result = v_u32_f32_sel_eq_v_s_v_v(*((uint64*)&input), PLUS_INF_FP32, plus_inf_fp32, result);
    result = v_i32_f32_sel_grt_v_s_v_v(*((int64*)&input), PLUS_INF_FP32, nan_fp32, result);
// ====================================

    return result;
}

//////////////////////////////////////// POW_F32 //////////////////////////////////////////////////
float64 v_pow_fast_f32(float64 base, float64 exp)
{
    int64 exp_floor = v_convert_f32_to_i32_v(exp, e_round_down);
    float64 exp_floor_fl = v_convert_i32_to_f32_v(exp_floor, e_round_half_ne);

    // predicate mask of all exp elements that are whole numbers
    bool256 p0 = bv_f32_cmp_eq_v_v(exp, exp_floor_fl);
    // apply abs on all exp elements that are whole numbers
    float64 base_b = v_f32_form_fp_num_v_v_v_vb(base, base, base, base, SW_FORCE_SIGN0, p0, 0);

    float64 base_log2 = v_log2_fast_f32(base_b);
    float64 power = v_f32_mul_v_v(exp, base_log2);
    float64 result = v_pow2_fast_f32(power);

    // predicate mask of all exp elements that are whole odd numbers
    bool256 p1 = 0;
    p1 = bv_i32_cmp_eq_v_s_vb((exp_floor & 0x00000001), 0x00000001, p1, p0, 0);
    // return -result if base < 0 and exp is a whole odd number.
    result = v_f32_f32_sel_less_v_s_v_v_vb(base, 0.0f, -result, result, result, p1, 0);

    return result;
}

float64 v_pow_f32(float64 base, float64 exp)
{
    int64 exp_floor = v_convert_f32_to_i32_v(exp, e_round_down);
    float64 exp_floor_fl = v_convert_i32_to_f32_v(exp_floor, e_round_half_ne);

    // predicate mask of all exp elements that are whole numbers
    bool256 p0 = bv_f32_cmp_eq_v_v(exp, exp_floor_fl);
    // apply abs on all exp elements that are whole numbers
    float64 base_b = v_f32_abs_v_vb(base, base, p0, 0);

    float64 base_log2 = v_log2_f32(base_b);
    float64 power = v_f32_mul_v_v(exp, base_log2);
    float64 result = v_pow2_f32(power);

    // predicate mask of all exp elements that are whole odd numbers
    bool256 p1 = 0;
    p1 = bv_i32_cmp_eq_v_s_vb((exp_floor & 0x00000001), 0x00000001, p1, p0, 0);
    // return -result if base < 0 and exp is a whole odd number.
    result = v_f32_f32_sel_less_v_s_v_v_vb(base, 0.0f, -result, result, result, p1, 0);
    // if exp = 0
    result = v_f32_f32_sel_eq_v_v_v_v(exp, 0.0f, 1.0f, result);                                     // 63
    // if exp = 1
    result = v_f32_f32_sel_eq_v_v_v_v(exp, 1.0f, base, result);                                     // 64


    const uint64 flt_max = FLT_MAX;
    const float64 flt_max_fp32 = *((float64*)&flt_max);
    const uint64 plus_inf = PLUS_INF_FP32;
    const float64 plus_inf_fp32 = *((float64*)&plus_inf);

    float64 abs_base = v_f32_abs_v(base);                                                           // 65
    float64 abs_exp  = v_f32_abs_v(exp);                                                            // 66

    bool256 pred0 = bv_f32_cmp_leq_v_v(exp, -flt_max_fp32);
    result = v_f32_f32_sel_grt_v_s_v_v_vb(abs_base, 1.0f, 0.0f, result, result, pred0, 0);
    result = v_f32_f32_sel_less_v_s_v_v_vb(abs_base, 1.0f, plus_inf_fp32, result, result, pred0, 0);

    bool256 pred1 = bv_f32_cmp_geq_v_v(exp, flt_max_fp32);
    result = v_f32_f32_sel_grt_v_s_v_v_vb(abs_base, 1.0f, plus_inf_fp32, result, result, pred1, 0);
    result = v_f32_f32_sel_less_v_s_v_v_vb(abs_base, 1.0f, 0.0f, result, result, pred1, 0);

    bool256 pred2 = bv_f32_cmp_less_v_v(base, -flt_max_fp32);
    result = v_f32_f32_sel_grt_v_s_v_v_vb(exp, 0.0f, plus_inf_fp32, result, result, pred2, 0);
    result = v_f32_f32_sel_less_v_s_v_v_vb(exp, 0.0f, 0.0f, result, result, pred2, 0);

    bool256 pred3 = bv_f32_cmp_geq_v_v(base, flt_max_fp32);
    pred3 = bv_f32_cmp_grt_v_s_vb(abs_exp, 1.0f, pred3, pred3, 0);
    result = v_f32_f32_sel_grt_v_s_v_v_vb(exp, 0.0f, plus_inf_fp32, result, result, pred3, 0);
    result = v_f32_f32_sel_less_v_s_v_v_vb(exp, 0.0f, 0.0f, result, result, pred3, 0);

    return result;
}

//quantized versions of special function is available only on Goya
#ifdef __goya__
/////////////////////////////////////// RECIP_I16 /////////////////////////////////////////////////
inline short128 v_recip_i16(short128 x, short expX)
{
    short128 shift = expX;
    short128 x00_shr;
    short128 x00_and;
    short128_pair_t y00 = {0};

    bool256 bRangeReduced = {0};

    short fourFlex = 4 << (-expX);
    short fourFlexMinusOne = fourFlex - 1;

    short intervalShift     = -4 - expX;
    unsigned short intervalStartMask = (1 << intervalShift) - 1;

    // if (x >= fourFlex)  // 4 << (-expX)
    //     x = x >> 2;
    char cmp_pred = ((-expX) < 13 ); // if exp >=  13, we get overflow,
                                     // so need to avoid this situations
    bRangeReduced = bv_i16_cmp_geq_v_s_b(x, fourFlex, bRangeReduced, cmp_pred, 0);

    x = v_i16_ash_v_s_vb(x, -2, x, 1, bRangeReduced, 0);
    x = v_i16_min_v_s_vb(x, fourFlexMinusOne, x, bRangeReduced, 0);

    x00_shr = v_i16_shr_v_s(x, intervalShift);
    x00_and = v_i16_and_v_s(x, intervalStartMask);

    y00 = v_i16_lookup_c1c2_v(*(ushort128 *)&x00_shr, y00, e_lookup_fp16_low, e_i16_rcp);
    y00 = v_i16_lookup_c1c2_v(*(ushort128 *)&x00_shr, y00, e_lookup_fp16_high, e_i16_rcp);
    y00.v1 = v_i16_msac_v_v_v_s(y00.v2, x00_and, (char256)shift, 0, y00.v1, 1, e_normalize_ab);

    y00.v2 = v_i16_lookup_c0_v(*(ushort128 *)&x00_shr, y00.v2, e_lookup_fp16_low, e_i16_rcp);
    y00.v2 = v_i16_lookup_c0_v(*(ushort128 *)&x00_shr, y00.v2, e_lookup_fp16_high, e_i16_rcp);
    y00.v2 = v_i16_msac_v_v_v_s(y00.v1, x00_and, (char256)shift, 0, y00.v2, 1, e_normalize_ab);

    y00.v2 = v_i16_ash_v_s_vb(y00.v2, -2, y00.v2, 1, bRangeReduced, 0);

    return y00.v2;
}
//////////////////////////////////////// TANH_I16 /////////////////////////////////////////////////
// Maximal input exponentX is expected to be (-12)
// Valid input range (-8,8)
// Output exponent -15
inline short128 v_tanh_i16(
    short128 input,
    short tanhIntervalShift, // -3 - exponentX
    short tanhIntervalStartMask, // (1 << tanhIntervalShift)-1
    char tanhMSAC0diffABToC, // exponentTanhC2 + exponentX - exponentTanhC1
    char tanhMSAC1diffABToC // exponentTanhC1 + exponentX - exponentTanhC0
    )
{
    short128 absX = v_i16_abs_v(input);
    short128 interval = v_i16_shr_v_s(absX, tanhIntervalShift);

    absX &= tanhIntervalStartMask;

    short128_pair_t lookupResC1C2 = {0};
    lookupResC1C2 = v_i16_lookup_c1c2_v(*(ushort128*)&interval,lookupResC1C2, e_lookup_fp16_low, e_i16_tanh);
    lookupResC1C2 = v_i16_lookup_c1c2_v_b(*(ushort128*)&interval, lookupResC1C2, e_lookup_fp16_high, e_i16_tanh, 1, 0);

    short128 C0 = 0;
    C0 = v_i16_lookup_c0_v(*(ushort128*)&interval,C0, e_lookup_fp16_low, e_i16_tanh);
    C0 = v_i16_lookup_c0_v_b(*(ushort128*)&interval, C0, e_lookup_fp16_high, e_i16_tanh, 1, 0);

    short128 C1 = lookupResC1C2.v1;
    short128 C2 = lookupResC1C2.v2;
    // MSACs come here
    char256 shiftABToC = tanhMSAC0diffABToC;
    C1 = v_i16_msac_v_v_v_s_b(C2, absX, shiftABToC, 0, C1, 1, e_normalize_ab, 1, 0);
    shiftABToC = tanhMSAC1diffABToC;
    C0 = v_i16_msac_v_v_v_s_b(C1, absX, shiftABToC, 0, C0, 1, e_normalize_ab, 0, 1);

    bool256 isNegative = bv_i16_cmp_less_v_s(input, 0);

    short128 zerosVec = 0;

    // If the input was negative, subtract the result from zero, negating the result
    C0 = v_i16_sub_v_v_vb(zerosVec, C0, C0, e_no_saturation, isNegative, 0);

    return C0;
}

//////////////////////////////////////// SIGMOID_I16 //////////////////////////////////////////////
// Maximal input exponentX is expected to be (-12)
// Valid input range (-8,8)
// output exponent -15
inline short128 v_sigmoid_i16(
    short128 input,
    short sigmoidIntervalShift,     // -3 - exponentX
    short sigmoidIntervalStartMask, // (1 << tanhIntervalShift)-1
    char sigmoidMSAC0diffABToC,     // exponentSigmoidC2 + exponentX - exponentSigmoidC1
    char sigmoidMSAC1diffABToC      // exponentSigmoidC1 + exponentX - exponentSigmoidC0)
    )
{
    short128 absX = v_i16_abs_v(input);
    short128 interval = v_i16_shr_v_s(absX, sigmoidIntervalShift);

    absX &= sigmoidIntervalStartMask;
    short128_pair_t lookupResC1C2 = {0};
    lookupResC1C2 = v_i16_lookup_c1c2_v(*(ushort128*)&interval, lookupResC1C2, e_lookup_fp16_low, e_i16_sigmoid);
    lookupResC1C2 = v_i16_lookup_c1c2_v_b(*(ushort128*)&interval, lookupResC1C2, e_lookup_fp16_high, e_i16_sigmoid, 1, 0);

    short128 C0 = 0;
    C0 = v_i16_lookup_c0_v(*(ushort128*)&interval, C0, e_lookup_fp16_low, e_i16_sigmoid);
    C0 = v_i16_lookup_c0_v_b(*(ushort128*)&interval, C0, e_lookup_fp16_high, e_i16_sigmoid, 1, 0);

    short128 C1 = lookupResC1C2.v1;
    short128 C2 = lookupResC1C2.v2;
    // MSACs come here
    char256 shiftABToC = sigmoidMSAC0diffABToC;
    C1 = v_i16_msac_v_v_v_s_b(C2, absX, shiftABToC, 0, C1, 1, e_normalize_ab, 1, 0);
    shiftABToC = sigmoidMSAC1diffABToC;
    C0 = v_i16_msac_v_v_v_s_b(C1, absX, shiftABToC, 0, C0, 1, e_normalize_ab, 0, 1);

    bool256 isNegative = bv_i16_cmp_less_v_s(input, 0);

    ushort128 quantizedOnes = 32768;
    ushort128 ushortC0 = v_u16_sub_v_v_vb(quantizedOnes, *(ushort128*)&C0, *(ushort128*)&C0, e_no_saturation, isNegative, 0);
    return *(short128 *)&ushortC0;
}

//////////////////////////////////////// EXP_I8 //////////////////////////////////////////////////
// Maximal Input exponentX is -4
// valid input range (-8,0)
// Result is in under quantization factor of -7. Range (0,1)
inline char256 v_exp_i8(char256 x, char shift)
{
    char256 x00_abs;
    char256 x00_shr;
    char256 x00_and;
    char256_pair_t y00 = {0};

    char intervalShift = -3 - shift;
    char intervalStartMask = ((1 << intervalShift) - 1);

    //char256 norm_shift = (-7 + shift) - (-7);
    char256 norm_shift = shift;

    x00_abs = v_i8_abs_v(x);
    x00_shr = v_i8_shr_v_s(x00_abs, intervalShift);
    x00_and = v_i8_and_v_s(x00_abs, intervalStartMask);

    y00 = v_i8_lookup_c1c2_v(*(uchar256 *)&x00_shr, y00, e_lookup_int8_0, 40);
    y00 = v_i8_lookup_c1c2_v(*(uchar256 *)&x00_shr, y00, e_lookup_int8_1, 40);
    y00 = v_i8_lookup_c1c2_v(*(uchar256 *)&x00_shr, y00, e_lookup_int8_2, 40);
    y00 = v_i8_lookup_c1c2_v(*(uchar256 *)&x00_shr, y00, e_lookup_int8_3, 40);

    y00.v1 = v_i8_msac_v_v_v_s(y00.v2, x00_and, norm_shift, 0, y00.v1, 1, e_normalize_ab);

    return y00.v1;
}
//////////////////////////////////////// EXP_I16 //////////////////////////////////////////////////
// Maximal input exponentX is -11
// valid input range (-16,0)
// Result is in under quantization factor of -15. Range (0,1)
inline short128 v_exp_i16(short128 x, short shift)
{
    short128 x00_abs;
    short128 x00_shr;
    short128 x00_and;
    short128_pair_t y00 = {0};

    short intervalShift = -2 - shift;
    short intervalStartMask = ((1 << intervalShift) - 1);

    //qC2 = -16 , qC1 =  -15, qc0 = -15

    // Exponent normalization ( (-16 + -11) - (-15)) = 15-27 = -12
    short128 shift_lp = -1 + shift;

    // Exponent normalization ( (-15 + -11) - (-15)) = 15-26 = -11
    short128 shift_hp = shift;

    //(C2 * x + C1) * x  + C0
    x00_abs = v_i16_abs_v(x);
    x00_shr = v_i16_shr_v_s(x00_abs, intervalShift);
    x00_and = v_i16_and_v_s(x00_abs, intervalStartMask);

    y00 = v_i16_lookup_c1c2_v(*(ushort128 *)&x00_shr, y00, e_lookup_fp16_low, e_i16_exp_nep);
    y00 = v_i16_lookup_c1c2_v(*(ushort128 *)&x00_shr, y00, e_lookup_fp16_high, e_i16_exp_nep);

    y00.v1 = v_i16_msac_v_v_v_s(y00.v2, x00_and, (char256)shift_lp, 0, y00.v1, 1, e_normalize_ab);

    y00.v2 = v_i16_lookup_c0_v(*(ushort128 *)&x00_shr, y00.v2, e_lookup_fp16_low, e_i16_exp_nep);
    y00.v2 = v_i16_lookup_c0_v(*(ushort128 *)&x00_shr, y00.v2, e_lookup_fp16_high, e_i16_exp_nep);

    y00.v2 = v_i16_msac_v_v_v_s(y00.v1, x00_and, (char256)shift_hp, 0, y00.v2, 1, e_normalize_ab);

    return y00.v2;
}

#endif //#ifdef __goya__
//////////////////////////////////////// rcp_I16 /////////////////////////////////////////////////
// Maximal input exponentX is expected to be (-11)
// Valid input range (1,16)
// output exponent -15

///////////////////////////////////// EXP_CEPHES_F32 //////////////////////////////////////////////
float64 v_exp_cephes_fast_f32(float64 input)
{
    const float ln_2_1 =  0.693359375;
    const float ln_2_2 = -2.12194440e-4;
    const float log2_e =  1.44269504088896341;
    const float c1 = 1.9875691500E-4;
    const float64 c2 = 1.3981999507E-3;
    const float64 c3 = 8.3334519073E-3;
    const float64 c4 = 4.1665795894E-2;
    const float64 c5 = 1.6666665459E-1;
    const float64 c6 = 5.0000001201E-1;

    float64 z = 0.5;
    z = v_f32_mac_v_s(input, log2_e, z, false);

    z = v_f32_nearbyint_v(z, e_round_down);

    float64 x_reduced = input;
    x_reduced = v_f32_mac_v_v(z, ln_2_1, x_reduced, true);
    x_reduced = v_f32_mac_v_v(z, ln_2_2, x_reduced, true);

    float64 sqr_x = v_f32_mul_v_v(x_reduced, x_reduced);
    float64 result;
    result = v_f32_mac_v_s(x_reduced ,c1 , c2, false);
    result = v_f32_mac_v_v(result, x_reduced, c3, false);
    result = v_f32_mac_v_v(result, x_reduced, c4, false);
    result = v_f32_mac_v_v(result, x_reduced, c5, false);
    result = v_f32_mac_v_v(result, x_reduced, c6, false);

    float64 x_plus1 = 1.0;
    x_plus1 = v_f32_add_v_v(x_reduced, x_plus1);

    result = v_f32_mac_v_v(result, sqr_x, x_plus1, false);

    int64 res_i32 = *(int64*)&result;

    int64 z_i32 = v_convert_f32_to_i32_v(z, e_round_down);
    z_i32 = v_i32_shl_v_s(z_i32, 23);
    res_i32 = v_i32_add_v_v(res_i32, z_i32, 0);

    result = *(float64*)&res_i32;

    return result;
}

float64 v_exp_cephes_f32(float64 input)
{
    float64 result = v_exp_cephes_fast_f32(input);
// ====================================
//  Processing special values: spec.exp values, +-inf, nan

    const float64 exp_lower = -87.336;
    const float64 exp_upper =  88.722;
    const int64 plus_inf  = PLUS_INF_FP32;

    result = v_f32_f32_sel_leq_v_v_v_v(input, exp_lower, 0.0f, result);
    result = v_f32_f32_sel_geq_v_v_v_v(input, exp_upper, *((float64*)&plus_inf), result);
    result = v_u32_f32_sel_grt_v_v_v_v(*((uint64*)&input) & NAN_FP32, PLUS_INF_FP32, input, result);
// ====================================

    return result;
}

//////////////////////////////////////// SIGMOID_F32 //////////////////////////////////////////////
inline
float64 v_sigmoid_f32(float64 input)
{//sigmoid(x) = 0.5 * (tanh(0.5*x)+1) instead of (div_f32(1.0, (1.0 + exp_cephes_f32(-input))));
    float64 x = v_f32_mul_v_s(input, 0.5f);                                                         // 1
    float64 res = 0.5f;
    x = v_tanh_f32(x);                                                                                // 17
    res =  v_f32_mac_v_s(x, 0.5f, res, e_no_negation);                                              // 18

    return res;
}

///////////////////////////////////// ASIN CEPHES F32 /////////////////////////////////////////////
float64 v_asin_cephes_f32(float64 input)
{
    float64 x, abs_x;
    float64 z = 0, result = 0;

    const float64 c1 = 4.2163199048E-2;
    const float64 c2 = 2.4181311049E-2;
    const float64 c3 = 4.5470025998E-2;
    const float64 c4 = 7.4953002686E-2;
    const float64 c5 = 1.6666752422E-1;
    const float64 PIO2F = 1.5707963267948966192;

    bool256 lt_zero = bv_f32_cmp_less_v_s(input, 0);

    abs_x = v_f32_abs_v(input);

    bool256 lt_one = bv_f32_cmp_leq_v_s(abs_x, 1);
    bool256 gt_half = bv_f32_cmp_grt_v_s(abs_x, 0.5);

    // Predicate is set for elements > 0.5 and <= 1.0
    bool256 pred0 = bv_b_and_bv_bv(lt_one, gt_half);

    bool256 lt_half = bv_f32_cmp_leq_v_s(abs_x, 0.5);
    bool256 gt_min_const = bv_f32_cmp_geq_v_s(abs_x, 1.0e-4);

    // Predicate is set for elements >= 1.0e-4 and <= 0.5
    bool256 pred1 = bv_b_and_bv_bv(lt_half, gt_min_const);

    bool256 ele_in_range = bv_b_or_bv_bv(pred0, pred1);
    bool256 ele_lt_zero = bv_b_and_bv_bv(ele_in_range, lt_zero);

    // 0.5 * (1.0 - a);
    z = v_f32_mov_s_vb(0.5, z, pred0, 0);
    z = v_f32_mac_v_s_vb(abs_x, 0.5, z, true, pred0, 0);
    x = v_sqrt_f32(z);

    x = v_f32_mov_v_vb(abs_x, x, pred1, 0);
    z = v_f32_mul_v_v_vb(x, x, z, pred1, 0);

    result = v_f32_mac_v_v(c1, z, c2, 0);
    result = v_f32_mac_v_v(result, z, c3, 0);
    result = v_f32_mac_v_v(result, z, c4, 0);
    result = v_f32_mac_v_v(result, z, c5, 0);
    float64 temp0 = v_f32_mul_v_v(z, x);
    result = v_f32_mac_v_v(result, temp0, x, 0);

    result = v_f32_add_v_v_vb(result, result, result, pred0, 0);
    result = v_f32_sub_v_v_vb(PIO2F, result, result, 0, pred0, 0);

    // sign < 0
    result = v_f32_mul_v_s_vb(result, -1, result, ele_lt_zero, 0);

    // abs(input) > 1.0
    result = v_f32_mov_s_vb(0, result, lt_one, 1);

    // abs(input) < 1.0e-4
    result = v_f32_mov_v_vb(input, result, gt_min_const, 1);

    return result;
}

///////////////////////////////////// ACOS CEPHES F32 /////////////////////////////////////////////
float64 v_acos_cephes_f32(float64 input)
{
    float64 x, acc = 0, scale_const = -2;
    const float PIF = 3.141592653589793238;
    const float PIO2F = 1.5707963267948966192;

    float64 abs_x = v_f32_abs_v(input);

    bool256 leq_one = bv_f32_cmp_leq_v_s(abs_x, 1);
    bool256 gt_half = bv_f32_cmp_grt_v_s(abs_x, 0.5);

    // Predicate is set if abs(input) is > 0.5 and <= 1.0
    bool256 pred0 = bv_b_and_bv_bv(leq_one, gt_half);

    bool256 lt_zero = bv_f32_cmp_less_v_s(input, 0);
    // Predicate is set if input element is < -0.5
    bool256 pred1 = bv_b_and_bv_bv(pred0, lt_zero);

    // Calculate 0.5 * (1.0 - x) when 0.5 < abs(input) <= 1.0
    x = v_f32_mov_s_vb(0.5, abs_x, pred0, 0);
    x = v_f32_mac_v_s_vb(abs_x, 0.5, x, true, pred0, 0);

    x = v_sqrt_f32(x);
    x = v_f32_mov_v_vb(input, x, gt_half, 1);

    // Call Arcsin implementation
    x = v_asin_cephes_f32(x);

    acc = v_f32_mov_s_vb(PIF, acc, pred1, 0);
    acc = v_f32_mov_s_vb(PIO2F, acc, gt_half, 1);

    scale_const = v_f32_mov_s_vb(1, scale_const, gt_half, 1);
    scale_const = v_f32_mov_s_vb(2, scale_const, pred1, 0);

    acc = v_f32_mac_v_v(scale_const, x, acc, true);
    return acc;
}

///////////////////////////////////// ATAN_CEPHES_F32 /////////////////////////////////////////////

float64 v_atan_cephes_f32(float64 input)
{
    const float PI_2 = 1.5707963267948966192;  // pi/2
    const float PI_4 = 0.7853981633974483096;  // pi/4
    const float TAN_3PI_8 = 2.414213562373095; // tan(3pi/8)
    const float TAN_PI_8 = 0.4142135623730950; // tan(pi/8)

    const float C1 = 8.05374449538e-2;
    const float C2 = -1.38776856032e-1;
    const float C3 = 1.99777106478e-1;
    const float C4 = -3.33329491539e-1;

    float64 y, res;

    bool256 lt_zero = bv_f32_cmp_less_v_s(input, 0.0);
    input = v_f32_abs_v(input);

    // if input > TAN_PI_8
    float64 minus_one = v_f32_sub_v_s(input, 1.0, 0);
    float64 plus_one = v_f32_add_v_s(input, 1.0);
    float64 div_res = v_div_f32(minus_one, plus_one);

    y = v_f32_f32_sel_grt_v_v_v_v(input, TAN_PI_8, PI_4, 0);
    res = v_f32_f32_sel_grt_v_v_v_v(input, TAN_PI_8, div_res, input);

    // if input > TAN_3PI_8
    float64 neg_one = -1.0;
    div_res = v_div_f32(neg_one, input);

    y = v_f32_f32_sel_grt_v_v_v_v(input, TAN_3PI_8, PI_2, y);
    input = v_f32_f32_sel_grt_v_v_v_v(input, TAN_3PI_8, div_res, res);

    float64 z = v_f32_mul_v_v(input, input);
    res = v_f32_mac_v_v(C1, z, C2, 0);
    res = v_f32_mac_v_v(res, z, C3, 0);
    res = v_f32_mac_v_v(res, z, C4, 0);
    res = v_f32_mul_v_v(res, input);
    res = v_f32_mac_v_v(res, z, input, 0);

    y = v_f32_add_v_v(res, y);

    // if input < 0 -> y = -y
    y = v_f32_sub_v_s_vb(y, 0.0, y, 1, lt_zero, 0);

    return y;
}

///////////////////////////////////// TAN_CEPHES_F32 /////////////////////////////////////////////

float64 v_tan_cephes_f32(float64 input)
{
    const float DP1 = -0.78515625;
    const float DP2 = -2.4187564849853515625e-4;
    const float DP3 = -3.77489497744594108e-8;
    const float FOPI = 1.27323954473516;  /* 4/pi */
    const float lossth = 8192.0;
    const float low_range = 1.0e-4;

    const float C1 = 9.38540185543E-3;
    const float C2 = 3.11992232697E-3;
    const float C3 = 2.44301354525E-2;
    const float C4 = 5.34112807005E-2;
    const float C5 = 1.33387994085E-1;
    const float C6 = 3.33331568548E-1;

    float64 y, z, sqr_z;
    int64 j, j_and_1, j_and_2;

    bool256 lt_zero = bv_f32_cmp_less_v_s(input, 0.0);
    input = v_f32_abs_v(input);

    float64 res = v_f32_mul_v_v(FOPI, input);
    y = v_f32_nearbyint_v(res, 1);

    // convert res f32 to i32 to perform bitwise operation
    j = v_convert_f32_to_i32_v(res, 1);

    // if (j & 1)
    j_and_1 = v_i32_and_v_s(j, 1);
    bool256 j_1 = bv_i32_cmp_eq_v_s(j_and_1, 1);
    j = v_i32_add_v_s_vb(j, 1, j, 0, j_1, 0);
    y = v_f32_add_v_s_vb(y, 1, y, j_1, 0);

    z = v_f32_mac_v_v(DP1, y, input, 0);
    z = v_f32_mac_v_v(DP2, y, z, 0);
    z = v_f32_mac_v_v(DP3, y, z, 0);

    sqr_z = v_f32_mul_v_v(z, z);

    bool256 leq_low = bv_f32_cmp_leq_v_s(input, low_range);

    y = v_f32_mac_v_v(C1, sqr_z, C2, 0);
    y = v_f32_mac_v_v(y, sqr_z, C3, 0);
    y = v_f32_mac_v_v(y, sqr_z, C4, 0);
    y = v_f32_mac_v_v(y, sqr_z, C5, 0);
    y = v_f32_mac_v_v(y, sqr_z, C6, 0);
    y = v_f32_mul_v_v(y, z);
    y = v_f32_mac_v_v(y, sqr_z, z, 0);

    // if (input <= low_range), y = z
    y = v_f32_mov_v_vb(z, y, leq_low, 0);

    float64 neg_inv_y = v_div_fast_f32(-1.0, y);

    // if (j & 2)
    j_and_2 = v_i32_and_v_s(j, 2);
    bool256 j_2 = bv_i32_cmp_eq_v_s(j_and_2, 2);
    y = v_f32_mov_v_vb(neg_inv_y, y, j_2, 0);

    // if (input < 0), input = -input
    y = v_f32_sub_v_s_vb(y, 0.0, y, 1, lt_zero, 0);

    // if (abs(input) > lossth), y = 0
    bool256 b_lossth = bv_f32_cmp_grt_v_s(input, lossth);
    y = v_f32_mov_s_vb(0.0, y, b_lossth, 0);

    return y;
}

///////////////////////////////////// ASINH_F32 /////////////////////////////////////////////
float64 v_asinh_f32(float64 input)
{
    float C1 = 2.0122003309E-2;
    float C2 = -4.2699340972E-2;
    float C3 = 7.4847586088E-2;
    float C4 = -1.6666288134E-1;

    bool256 lt_zero = bv_f32_cmp_less_v_s(input, 0.0);
    input = v_f32_abs_v(input);

    float64 z = v_f32_mul_v_v(input, input);

    float64 res;
    res = v_f32_mac_v_v(C1, z, C2, 0);
    res = v_f32_mac_v_v(res, z, C3, 0);
    res = v_f32_mac_v_v(res, z, C4, 0);
    res = v_f32_mul_v_v(res, input);
    res = v_f32_mac_v_v(res, z, input, 0);

    z = v_f32_add_v_v(z, 1.0);
    z = v_sqrt_f32(z);
    z = v_f32_add_v_v(z, input);
    z = v_log_f32(z);

    bool256 geq_half = bv_f32_cmp_geq_v_s(input, 0.5);
    res = v_f32_mov_v_vb(z, res, geq_half, 0);

    // sign < 0
    res = v_f32_mul_v_s_vb(res, -1, res, lt_zero, 0);

    return res;
}

///////////////////////////////////// ACOSH_F32 /////////////////////////////////////////////
float64 v_acosh_f32(float64 input)
{
    float C1 = 1.7596881071E-3;
    float C2 = -7.5272886713E-3;
    float C3 = 2.6454905019E-2;
    float C4 = -1.1784741703E-1;
    float C5 = 1.4142135263E0;
    float LOGE2F = 0.693147180559945309;

    bool256 lt_one = bv_f32_cmp_less_v_s(input, 1.0);

    float64 z = v_f32_sub_v_s(input, 1.0, 0);

    bool256 geq_half = bv_f32_cmp_geq_v_s(z, 0.5);
    bool256 grt_limit = bv_f32_cmp_grt_v_s(input, 1500);

    // if z < 0.5
    float64 res;
    res = v_f32_mac_v_v(C1, z, C2, 0);
    res = v_f32_mac_v_v(res, z, C3, 0);
    res = v_f32_mac_v_v(res, z, C4, 0);
    res = v_f32_mac_v_v(res, z, C5, 0);

    float64 sqrt_z = v_sqrt_fast_f32(z);

    res = v_f32_mul_v_v(res, sqrt_z);

    // if z >= 0.5
    float64 plus_one = v_f32_add_v_s(input, 1);
    z = v_f32_mul_v_v(z, plus_one);
    z = v_sqrt_f32(z);
    z = v_f32_add_v_v(z, input);
    z = v_log_f32(z);

    res = v_f32_mov_v_vb(z, res, geq_half, 0);

    // if input > 1500
    z = v_log_f32(input);
    z = v_f32_add_v_s(z, LOGE2F);

    res = v_f32_mov_v_vb(z, res, grt_limit, 0);

    const uint64 nan_int = NAN_FP32;
    const float64 nan_fp32 = *((float64*)&nan_int);

    res = v_f32_mov_v_vb(nan_fp32, res, lt_one, 0);

    return res;
}

///////////////////////////////////// ATANH_F32 /////////////////////////////////////////////
float64 v_atanh_f32(float64 input)
{
    float C1 = 1.81740078349E-1;
    float C2 = 8.24370301058E-2;
    float C3 = 1.46691431730E-1;
    float C4 = 1.99782164500E-1;
    float C5 = 3.33337300303E-1;

    // if input < 0.5
    float64 z = v_f32_mul_v_v(input, input);
    float64 res = v_f32_mac_v_v(C1, z, C2, 0);
    res = v_f32_mac_v_v(res, z, C3, 0);
    res = v_f32_mac_v_v(res, z, C4, 0);
    res = v_f32_mac_v_v(res, z, C5, 0);
    res = v_f32_mul_v_v(res, input);
    res = v_f32_mac_v_v(res, z, input, 0);

    // if input >= 0.5
    float64 one_plus = v_f32_add_v_v(1, input);
    float64 one_minus = v_f32_sub_v_v(1, input, 0);
    z = v_div_fast_f32(one_plus, one_minus);
    z = v_log_fast_f32(z);
    z = v_f32_mul_v_s(z, 0.5);

    float64 abs_in = v_f32_abs_v(input);
    bool256 geq_half = bv_f32_cmp_geq_v_s(abs_in, 0.5);
    res = v_f32_mov_v_vb(z, res, geq_half, 0);

    const uint64 inf_int  = PLUS_INF_FP32;
    const float64 inf_f = *((float64*)&inf_int);
    const uint64 nan_int = NAN_FP32;
    const float64 nan_f = *((float64*)&nan_int);

    bool256 geq_one = bv_f32_cmp_geq_v_s(abs_in, 1.0);

    float64 gt_range = v_f32_f32_sel_less_v_v_v_v(input, 0, -inf_f, inf_f);
    gt_range = v_f32_f32_sel_grt_v_v_v_v(abs_in, 1.0, nan_f, gt_range);
    res = v_f32_mov_v_vb(gt_range, res, geq_one, 0);

    return res;
}

///////////////////////////////////// SINH_F32 /////////////////////////////////////////////
float64 v_sinh_cephes_f32(float64 input)
{
    const float maxlogf = 88.0;

    const float C1 = 2.03721912945E-4;
    const float C2 = 8.33028376239E-3;
    const float C3 = 1.66667160211E-1;

    float64 abs_val, temp_1, temp_2, result_1, result_2, final_res = 0.0f;
    bool256 lt_zero = bv_f32_cmp_less_v_s(input, 0.0);

    abs_val = v_f32_abs_v(input);
    bool256 gt_one = bv_f32_cmp_grt_v_s(abs_val, 1.0);

    // abs_val > 1.0

    result_1 = v_exp_cephes_f32(abs_val);
    temp_1 = v_f32_mul_v_s_vb(result_1, 0.5, result_1, gt_one, 0);
    temp_2 = v_div_f32(0.5, result_1);
    result_1 = v_f32_sub_v_v_vb(temp_1, temp_2, result_1, 0, gt_one, 0);

    bool256 pred0 = bv_b_and_bv_bv(gt_one, lt_zero);
    result_1 = v_f32_mul_v_s_vb(result_1, -1, result_1, pred0, 0);

    // abs_val <= 1.0

    temp_1 = v_f32_mul_v_v(input, input);
    result_2 = v_f32_mac_v_v(C1, temp_1, C2, 0);
    result_2 = v_f32_mac_v_v(result_2, temp_1, C3, 0);
    result_2 = v_f32_mul_v_v(result_2, temp_1);
    result_2 = v_f32_mac_v_v(result_2, input, input, 0);

    final_res = v_f32_mov_v_vb(result_1, final_res, gt_one, 0);
    final_res = v_f32_mov_v_vb(result_2, final_res, gt_one, 1);

    const int64 plus_inf = PLUS_INF_FP32;
    const int64 minus_inf = MINUS_INF_FP32;

    // abs_val > maxlog

    bool256 gt_maxlog = bv_f32_cmp_grt_v_s(abs_val, maxlogf);
    final_res = v_f32_f32_sel_grt_v_s_v_v_vb(input, 0.0, *((float64*)&plus_inf),
            *((float64*)&minus_inf), final_res, gt_maxlog, 0);

    return final_res;
}
///////////////////////////////////// COSH_F32 /////////////////////////////////////////////
float64 v_cosh_cephes_f32(float64 input)
{
    const float maxlogf = 88.0;

    float64 recip, result;

    input = v_f32_abs_v(input);

    result = v_exp_cephes_f32(input);

    // z + ( 1 / z )

    recip = v_reciprocal_f32(result);
    result = v_f32_add_v_v(result, recip);

    result = v_f32_mul_v_v(result, 0.5);

    // v_f32_abs_v > maxlog

    bool256 gt_maxlog = bv_f32_cmp_grt_v_s(input, maxlogf);

    const int64 plus_inf = PLUS_INF_FP32;

    result = v_f32_mov_v_vb(*(float64*)&plus_inf, result, gt_maxlog, 0);

    return result;
}
///////////////////////////////////// MOD_F32 /////////////////////////////////////////////
float64 v_mod_f32(float64 input_x, float64 input_y)
{
    float64 abs_x = v_f32_abs_v(input_x);
    float64 abs_y = v_f32_abs_v(input_y);
    float64 div_result = v_div_f32(abs_x, abs_y);
    float64 round_result = v_f32_nearbyint_v(div_result, e_round_half_ne);
    float64 mul_result = v_f32_mul_v_v(round_result , abs_y);
    float64 result = v_f32_sub_v_v(abs_x, mul_result, 0);

    bool256 bpredv = bv_f32_cmp_less_v_s(result, 0.0);
    result = v_f32_add_v_v_vb(result, abs_y, result, bpredv, 0);

    result = v_f32_form_fp_num_v_v_v(result, input_x, result, 0x0);

    return result;
}

///////////////////////////////////// EXPM1_F32 /////////////////////////////////////////////
float64 v_expm1_f32(float64 input)
{
    const float boundval = 5e-1;

    const uint64 flt_min = FLT_MIN;
    const float64 flt_min_fp32 = *((float64*)&flt_min);
    float64 output = 0, temp1 = 0, temp2 = 0, temp3;

    // Checking boundary

    bool256 leq_bv = bv_f32_cmp_leq_v_s(input, boundval);
    bool256 geq_bv = bv_f32_cmp_geq_v_s(input, -boundval);
    bool256 pred0 =  bv_b_and_bv_bv(leq_bv, geq_bv);

    // Cases within boundary

    output = v_f32_mul_v_s_vb(input, 0.5, output, pred0, 0);
    output = v_tanh_f32(output);

    temp1 = v_f32_mul_v_s_vb(output, 2, temp1, pred0, 0);
    temp2 = v_f32_sub_v_v_vb(1, output, temp2, 0, pred0, 0);
    output = v_div_f32(temp1, temp2);

    // Cases outside boundary

    temp3 =  v_exp_f32(input);
    output = v_f32_sub_v_s_vb(temp3, 1, output, 0, pred0, 1);

    // Special case of min float

    bool256 pred_fmin = bv_f32_cmp_eq_v_v(input, flt_min_fp32);
    output = v_f32_mov_v_vb(input, output, pred_fmin, 0);

    return output;
}

// Remove all pre-processor definitions
#if defined(__gaudi__)
    #undef v_f32_lookup_c0_v
    #undef v_f32_lookup_c1c2_v
#endif
#undef PROCESS_SIN_COS_SPECIAL_VALUES
#undef SIN_COS_CALC
#undef LOOKUP_AND_MAC
#undef CALC_REDUCED_VALUE
#undef false
#undef true
#undef UNIT_VAL
#undef SIGNIFICAND_MASK
#undef EXPONENT_MASK
#undef NAN_FP32
#undef PLUS_INF_FP32
#undef MINUS_INF_FP32
#undef EXP_LOWER
#undef EXP_UPPER
#undef FLT_MIN
#undef FLT_MAX
#undef M_UNIT_EXP


#if __TPC_DROP_VERSION >= VERSION2DEC(35, 0, 0)
#define INCLUDE_TPC_REDUCTION_CORE_H
////////////////////////////////////////////////////////////////////////////////////////////////////
/// F32
////////////////////////////////////////////////////////////////////////////////////////////////////

// float64 v_f32_reduce_add(float64 x);
#define REDUCE_DT 0
#define REDUCE_OP 0
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

// float64 v_f32_reduce_mul(float64 x);
#define REDUCE_DT 0
#define REDUCE_OP 1
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

// float64 v_f32_reduce_min(float64 x);
#define REDUCE_DT 0
#define REDUCE_OP 2
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

// float64 v_f32_reduce_max(float64 x);
#define REDUCE_DT 0
#define REDUCE_OP 3
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

// uint64_float64_pair_t v_f32_reduce_argmin(float64 x);
#define REDUCE_DT 0
#define REDUCE_OP 4
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

// uint64_float64_pair_t v_f32_reduce_argmax(float64 x);
#define REDUCE_DT 0
#define REDUCE_OP 5
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

////////////////////////////////////////////////////////////////////////////////////////////////////
/// I32
////////////////////////////////////////////////////////////////////////////////////////////////////

// int64 v_i32_reduce_add(int64 x);
#define REDUCE_DT 1
#define REDUCE_OP 0
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

// int64 v_i32_reduce_max(int64 x);
#define REDUCE_DT 1
#define REDUCE_OP 3
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP


// uint64_int64_pair_t v_i32_reduce_argmin(int64 x);
#define REDUCE_DT 1
#define REDUCE_OP 4
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

// uint64_int64_pair_t v_i32_reduce_argmax(int64 x);
#define REDUCE_DT 1
#define REDUCE_OP 5
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

////////////////////////////////////////////////////////////////////////////////////////////////////
/// U32
////////////////////////////////////////////////////////////////////////////////////////////////////

// uint64 v_u32_reduce_add(uint64 x);
#define REDUCE_DT 2
#define REDUCE_OP 0
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

////////////////////////////////////////////////////////////////////////////////////////////////////
/// BF16
////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(__gaudi__)

// bfloat128 v_bf16_reduce_add(bfloat128 x);
#define REDUCE_DT 3
#define REDUCE_OP 0
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

// bfloat128 v_bf16_reduce_min(bfloat128 x);
#define REDUCE_DT 3
#define REDUCE_OP 2
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

// bfloat128 v_bf16_reduce_max(bfloat128 x);
#define REDUCE_DT 3
#define REDUCE_OP 3
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// I16
////////////////////////////////////////////////////////////////////////////////////////////////////

// short128 v_i16_reduce_min(short128 x);
#define REDUCE_DT 5
#define REDUCE_OP 2
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

// short128 v_i16_reduce_max(short128 x);
#define REDUCE_DT 5
#define REDUCE_OP 3
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

////////////////////////////////////////////////////////////////////////////////////////////////////
/// U16
////////////////////////////////////////////////////////////////////////////////////////////////////

// ushort128 v_u16_reduce_add(ushort128 x);
#define REDUCE_DT 6
#define REDUCE_OP 0
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

////////////////////////////////////////////////////////////////////////////////////////////////////
/// I8
////////////////////////////////////////////////////////////////////////////////////////////////////

// char256 v_i8_reduce_min(char256 x);
#define REDUCE_DT 7
#define REDUCE_OP 2
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

// char256 v_i8_reduce_max(char256 x);
#define REDUCE_DT 7
#define REDUCE_OP 3
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

////////////////////////////////////////////////////////////////////////////////////////////////////
/// U8
////////////////////////////////////////////////////////////////////////////////////////////////////

// uchar256 v_u8_reduce_min(uchar256 x);
#define REDUCE_DT 8
#define REDUCE_OP 2
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

// uchar256 v_u8_reduce_max(uchar256 x);
#define REDUCE_DT 8
#define REDUCE_OP 3
#include "tpc-reduction_functions_core.h"
#undef REDUCE_DT
#undef REDUCE_OP

#endif //__TPC_DROP_VERSION

#endif // TPC_SPECIAL_FUNCS_INCLUDED
#endif // TPC_SPECIAL_H_INCLUDED
