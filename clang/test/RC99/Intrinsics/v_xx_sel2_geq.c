
// RUN: %codegen -S -O2 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O2 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK,GEN2 %s
// RUN: %codegen -S -O2 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK,GEN2,GEN3 %s

typedef struct _float64_pair_t  float64_float64;
typedef struct _float64_int64_pair_t  float64_int64;
typedef struct _float64_uint64_pair_t  float64_uint64;
typedef struct _int64_pair_t  int64_int64;
typedef struct _int64_float64_pair_t  int64_float64;
typedef struct _int64_uint64_pair_t  int64_uint64;
typedef struct _uint64_pair_t  uint64_uint64;
typedef struct _uint64_float64_pair_t  uint64_float64;
typedef struct _uint64_int64_pair_t  uint64_int64;
typedef struct _bfloat128_pair_t  bfloat128_bfloat128;
typedef struct _bfloat128_half128_pair_t  bfloat128_half128;
typedef struct _bfloat128_short128_pair_t  bfloat128_short128;
typedef struct _bfloat128_ushort128_pair_t  bfloat128_ushort128;
typedef struct _half128_pair_t  half128_half128;
typedef struct _half128_bfloat128_pair_t  half128_bfloat128;
typedef struct _half128_short128_pair_t  half128_short128;
typedef struct _half128_ushort128_pair_t  half128_ushort128;
typedef struct _short128_pair_t  short128_short128;
typedef struct _short128_bfloat128_pair_t  short128_bfloat128;
typedef struct _short128_half128_pair_t  short128_half128;
typedef struct _short128_ushort128_pair_t  short128_ushort128;
typedef struct _ushort128_pair_t  ushort128_ushort128;
typedef struct _ushort128_bfloat128_pair_t  ushort128_bfloat128;
typedef struct _ushort128_half128_pair_t  ushort128_half128;
typedef struct _ushort128_short128_pair_t  ushort128_short128;
typedef struct _char256_pair_t  char256_char256;
typedef struct _char256_uchar256_pair_t  char256_uchar256;
typedef struct _uchar256_pair_t  uchar256_uchar256;
typedef struct _uchar256_char256_pair_t  uchar256_char256;

void main(int dest, int src1, int src2, int pred_val, int vect_pred,
          float af32, int ai32, unsigned au32, short ai16, unsigned short au16,
          char ai8, unsigned char au8) {
  int64_int64 __local *dest_ptr = (int64_int64 __local *)dest;          
  int64 __local *src_ptr = (int64 __local *)src1;          
  int64 __local *res_ptr = (int64 __local *)src2;
  bool256 __local *vpred_ptr = (bool256 __local *)vect_pred;
  #if defined(__gaudi__) || defined(__goya2__)
  _BFloat16 abf16 = as_bf16(ai16);
  #endif
  #if defined(__goya2__)
  half af16= as_half(au16);
  #endif

  _Bool spred = src1 < src2; 
  bool256 vpred = *vpred_ptr;
  // CHECK-DAG: cmp_less.i32 [[SPRED:%SP[0-9]+]], %S1, %S2
  // CHECK-DAG: ld_l_v [[VPRED:%VP[0-9]+]]

  if (dest > 10) {
    volatile float64_float64 __local *dptr = (float64_float64 __local *)dest_ptr;
    volatile float64 __local *sptr = (float64 __local *)src_ptr;
    volatile float64 __local *rptr = (float64 __local *)res_ptr;

    float64_float64 result = *dptr++;
    float64 x1 = *sptr++;
    float64 x2 = *sptr++;
    float64 x3 = *rptr++;
    float64 x4 = *rptr++;


    //------ v_f32_sel2_geq_f32_b

    result = v_f32_sel2_geq_f32_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_f32_sel2_geq_f32_b(x1, af32, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S5, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_f32_sel2_geq_f32_b(x1, 1.5, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_f32_sel2_geq_f32_b(x1, x2, x3, af32, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S5, [[SPRED]]     

    result = v_f32_sel2_geq_f32_b(x1, x2, x3, 1.5, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, [[SPRED]]

    //------ v_f32_sel2_geq_f32_vb

    result = v_f32_sel2_geq_f32_vb(x1, x2, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_f32_sel2_geq_f32_vb(x1, af32, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S5, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_f32_sel2_geq_f32_vb(x1, 1.5, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_f32_sel2_geq_f32_vb(x1, x2, x3, af32, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S5, [[VPRED]]     

    result = v_f32_sel2_geq_f32_vb(x1, x2, x3, 1.5, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 


  if (dest > 11) {
    volatile float64_int64 __local *dptr = (float64_int64 __local *)dest_ptr;
    volatile int64 __local *sptr = (int64 __local *)src_ptr;
    volatile float64 __local *rptr = (float64 __local *)res_ptr;

    float64_int64 result = *dptr++;
    int64 x1 = *sptr++;
    int64 x2 = *sptr++;
    float64 x3 = *rptr++;
    float64 x4 = *rptr++;


    //------ v_f32_sel2_geq_i32_b

    result = v_f32_sel2_geq_i32_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_f32_sel2_geq_i32_b(x1, ai32, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S6, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_f32_sel2_geq_i32_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_f32_sel2_geq_i32_b(x1, x2, x3, af32, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S5, [[SPRED]]     

    result = v_f32_sel2_geq_i32_b(x1, x2, x3, 1.5, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, [[SPRED]]

    //------ v_f32_sel2_geq_i32_vb

    result = v_f32_sel2_geq_i32_vb(x1, x2, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_f32_sel2_geq_i32_vb(x1, ai32, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S6, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_f32_sel2_geq_i32_vb(x1, 123, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_f32_sel2_geq_i32_vb(x1, x2, x3, af32, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S5, [[VPRED]]     

    result = v_f32_sel2_geq_i32_vb(x1, x2, x3, 1.5, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 


  if (dest > 12) {
    volatile float64_uint64 __local *dptr = (float64_uint64 __local *)dest_ptr;
    volatile uint64 __local *sptr = (uint64 __local *)src_ptr;
    volatile float64 __local *rptr = (float64 __local *)res_ptr;

    float64_uint64 result = *dptr++;
    uint64 x1 = *sptr++;
    uint64 x2 = *sptr++;
    float64 x3 = *rptr++;
    float64 x4 = *rptr++;


    //------ v_f32_sel2_geq_u32_b

    result = v_f32_sel2_geq_u32_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_f32_sel2_geq_u32_b(x1, au32, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S7, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_f32_sel2_geq_u32_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_f32_sel2_geq_u32_b(x1, x2, x3, af32, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S5, [[SPRED]]     

    result = v_f32_sel2_geq_u32_b(x1, x2, x3, 1.5, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, [[SPRED]]

    //------ v_f32_sel2_geq_u32_vb

    result = v_f32_sel2_geq_u32_vb(x1, x2, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_f32_sel2_geq_u32_vb(x1, au32, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S7, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_f32_sel2_geq_u32_vb(x1, 123, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_f32_sel2_geq_u32_vb(x1, x2, x3, af32, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S5, [[VPRED]]     

    result = v_f32_sel2_geq_u32_vb(x1, x2, x3, 1.5, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

#if defined (__gaudi__) || defined(__goya2__)
    
  if (dest > 13) {
    volatile bfloat128_bfloat128 __local *dptr = (bfloat128_bfloat128 __local *)dest_ptr;
    volatile bfloat128 __local *sptr = (bfloat128 __local *)src_ptr;
    volatile bfloat128 __local *rptr = (bfloat128 __local *)res_ptr;

    bfloat128_bfloat128 result = *dptr++;
    bfloat128 x1 = *sptr++;
    bfloat128 x2 = *sptr++;
    bfloat128 x3 = *rptr++;
    bfloat128 x4 = *rptr++;


    //------ v_bf16_sel2_geq_bf16_b

    result = v_bf16_sel2_geq_bf16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_bf16_sel2_geq_bf16_b(x1, abf16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_bf16_sel2_geq_bf16_b(x1, 1.5, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_bf16_sel2_geq_bf16_b(x1, x2, x3, abf16, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     

    result = v_bf16_sel2_geq_bf16_b(x1, x2, x3, 1.5, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[SPRED]]

    //------ v_bf16_sel2_geq_bf16_vb

    result = v_bf16_sel2_geq_bf16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_bf16_sel2_geq_bf16_vb(x1, abf16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_bf16_sel2_geq_bf16_vb(x1, 1.5, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_bf16_sel2_geq_bf16_vb(x1, x2, x3, abf16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     

    result = v_bf16_sel2_geq_bf16_vb(x1, x2, x3, 1.5, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined(__goya2__)
    
  if (dest > 14) {
    volatile bfloat128_half128 __local *dptr = (bfloat128_half128 __local *)dest_ptr;
    volatile half128 __local *sptr = (half128 __local *)src_ptr;
    volatile bfloat128 __local *rptr = (bfloat128 __local *)res_ptr;

    bfloat128_half128 result = *dptr++;
    half128 x1 = *sptr++;
    half128 x2 = *sptr++;
    bfloat128 x3 = *rptr++;
    bfloat128 x4 = *rptr++;


    //------ v_bf16_sel2_geq_f16_b

    result = v_bf16_sel2_geq_f16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_bf16_sel2_geq_f16_b(x1, af16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_bf16_sel2_geq_f16_b(x1, 1.5, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_bf16_sel2_geq_f16_b(x1, x2, x3, abf16, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     

    result = v_bf16_sel2_geq_f16_b(x1, x2, x3, 1.5, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[SPRED]]

    //------ v_bf16_sel2_geq_f16_vb

    result = v_bf16_sel2_geq_f16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_bf16_sel2_geq_f16_vb(x1, af16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_bf16_sel2_geq_f16_vb(x1, 1.5, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_bf16_sel2_geq_f16_vb(x1, x2, x3, abf16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     

    result = v_bf16_sel2_geq_f16_vb(x1, x2, x3, 1.5, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined (__gaudi__) || defined(__goya2__)
    
  if (dest > 15) {
    volatile bfloat128_short128 __local *dptr = (bfloat128_short128 __local *)dest_ptr;
    volatile short128 __local *sptr = (short128 __local *)src_ptr;
    volatile bfloat128 __local *rptr = (bfloat128 __local *)res_ptr;

    bfloat128_short128 result = *dptr++;
    short128 x1 = *sptr++;
    short128 x2 = *sptr++;
    bfloat128 x3 = *rptr++;
    bfloat128 x4 = *rptr++;


    //------ v_bf16_sel2_geq_i16_b

    result = v_bf16_sel2_geq_i16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_bf16_sel2_geq_i16_b(x1, ai16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_bf16_sel2_geq_i16_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_bf16_sel2_geq_i16_b(x1, x2, x3, abf16, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     

    result = v_bf16_sel2_geq_i16_b(x1, x2, x3, 1.5, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[SPRED]]

    //------ v_bf16_sel2_geq_i16_vb

    result = v_bf16_sel2_geq_i16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_bf16_sel2_geq_i16_vb(x1, ai16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_bf16_sel2_geq_i16_vb(x1, 123, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_bf16_sel2_geq_i16_vb(x1, x2, x3, abf16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     

    result = v_bf16_sel2_geq_i16_vb(x1, x2, x3, 1.5, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined (__gaudi__) || defined(__goya2__)
    
  if (dest > 16) {
    volatile bfloat128_ushort128 __local *dptr = (bfloat128_ushort128 __local *)dest_ptr;
    volatile ushort128 __local *sptr = (ushort128 __local *)src_ptr;
    volatile bfloat128 __local *rptr = (bfloat128 __local *)res_ptr;

    bfloat128_ushort128 result = *dptr++;
    ushort128 x1 = *sptr++;
    ushort128 x2 = *sptr++;
    bfloat128 x3 = *rptr++;
    bfloat128 x4 = *rptr++;


    //------ v_bf16_sel2_geq_u16_b

    result = v_bf16_sel2_geq_u16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_bf16_sel2_geq_u16_b(x1, au16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_bf16_sel2_geq_u16_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_bf16_sel2_geq_u16_b(x1, x2, x3, abf16, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     

    result = v_bf16_sel2_geq_u16_b(x1, x2, x3, 1.5, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[SPRED]]

    //------ v_bf16_sel2_geq_u16_vb

    result = v_bf16_sel2_geq_u16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_bf16_sel2_geq_u16_vb(x1, au16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_bf16_sel2_geq_u16_vb(x1, 123, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_bf16_sel2_geq_u16_vb(x1, x2, x3, abf16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     

    result = v_bf16_sel2_geq_u16_vb(x1, x2, x3, 1.5, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined(__goya2__)
    
  if (dest > 17) {
    volatile half128_half128 __local *dptr = (half128_half128 __local *)dest_ptr;
    volatile half128 __local *sptr = (half128 __local *)src_ptr;
    volatile half128 __local *rptr = (half128 __local *)res_ptr;

    half128_half128 result = *dptr++;
    half128 x1 = *sptr++;
    half128 x2 = *sptr++;
    half128 x3 = *rptr++;
    half128 x4 = *rptr++;


    //------ v_f16_sel2_geq_f16_b

    result = v_f16_sel2_geq_f16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_f16_sel2_geq_f16_b(x1, af16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_f16_sel2_geq_f16_b(x1, 1.5, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_f16_sel2_geq_f16_b(x1, x2, x3, af16, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     

    result = v_f16_sel2_geq_f16_b(x1, x2, x3, 1.5, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[SPRED]]

    //------ v_f16_sel2_geq_f16_vb

    result = v_f16_sel2_geq_f16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_f16_sel2_geq_f16_vb(x1, af16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_f16_sel2_geq_f16_vb(x1, 1.5, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_f16_sel2_geq_f16_vb(x1, x2, x3, af16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     

    result = v_f16_sel2_geq_f16_vb(x1, x2, x3, 1.5, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined(__goya2__)
    
  if (dest > 18) {
    volatile half128_bfloat128 __local *dptr = (half128_bfloat128 __local *)dest_ptr;
    volatile bfloat128 __local *sptr = (bfloat128 __local *)src_ptr;
    volatile half128 __local *rptr = (half128 __local *)res_ptr;

    half128_bfloat128 result = *dptr++;
    bfloat128 x1 = *sptr++;
    bfloat128 x2 = *sptr++;
    half128 x3 = *rptr++;
    half128 x4 = *rptr++;


    //------ v_f16_sel2_geq_bf16_b

    result = v_f16_sel2_geq_bf16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_f16_sel2_geq_bf16_b(x1, abf16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_f16_sel2_geq_bf16_b(x1, 1.5, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_f16_sel2_geq_bf16_b(x1, x2, x3, af16, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     

    result = v_f16_sel2_geq_bf16_b(x1, x2, x3, 1.5, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[SPRED]]

    //------ v_f16_sel2_geq_bf16_vb

    result = v_f16_sel2_geq_bf16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_f16_sel2_geq_bf16_vb(x1, abf16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_f16_sel2_geq_bf16_vb(x1, 1.5, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_f16_sel2_geq_bf16_vb(x1, x2, x3, af16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     

    result = v_f16_sel2_geq_bf16_vb(x1, x2, x3, 1.5, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined(__goya2__)
    
  if (dest > 19) {
    volatile half128_short128 __local *dptr = (half128_short128 __local *)dest_ptr;
    volatile short128 __local *sptr = (short128 __local *)src_ptr;
    volatile half128 __local *rptr = (half128 __local *)res_ptr;

    half128_short128 result = *dptr++;
    short128 x1 = *sptr++;
    short128 x2 = *sptr++;
    half128 x3 = *rptr++;
    half128 x4 = *rptr++;


    //------ v_f16_sel2_geq_i16_b

    result = v_f16_sel2_geq_i16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_f16_sel2_geq_i16_b(x1, ai16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_f16_sel2_geq_i16_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_f16_sel2_geq_i16_b(x1, x2, x3, af16, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     

    result = v_f16_sel2_geq_i16_b(x1, x2, x3, 1.5, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[SPRED]]

    //------ v_f16_sel2_geq_i16_vb

    result = v_f16_sel2_geq_i16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_f16_sel2_geq_i16_vb(x1, ai16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_f16_sel2_geq_i16_vb(x1, 123, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_f16_sel2_geq_i16_vb(x1, x2, x3, af16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     

    result = v_f16_sel2_geq_i16_vb(x1, x2, x3, 1.5, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined(__goya2__)
    
  if (dest > 20) {
    volatile half128_ushort128 __local *dptr = (half128_ushort128 __local *)dest_ptr;
    volatile ushort128 __local *sptr = (ushort128 __local *)src_ptr;
    volatile half128 __local *rptr = (half128 __local *)res_ptr;

    half128_ushort128 result = *dptr++;
    ushort128 x1 = *sptr++;
    ushort128 x2 = *sptr++;
    half128 x3 = *rptr++;
    half128 x4 = *rptr++;


    //------ v_f16_sel2_geq_u16_b

    result = v_f16_sel2_geq_u16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_f16_sel2_geq_u16_b(x1, au16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_f16_sel2_geq_u16_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_f16_sel2_geq_u16_b(x1, x2, x3, af16, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     

    result = v_f16_sel2_geq_u16_b(x1, x2, x3, 1.5, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[SPRED]]

    //------ v_f16_sel2_geq_u16_vb

    result = v_f16_sel2_geq_u16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_f16_sel2_geq_u16_vb(x1, au16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_f16_sel2_geq_u16_vb(x1, 123, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_f16_sel2_geq_u16_vb(x1, x2, x3, af16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     

    result = v_f16_sel2_geq_u16_vb(x1, x2, x3, 1.5, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 


  if (dest > 21) {
    volatile int64_int64 __local *dptr = (int64_int64 __local *)dest_ptr;
    volatile int64 __local *sptr = (int64 __local *)src_ptr;
    volatile int64 __local *rptr = (int64 __local *)res_ptr;

    int64_int64 result = *dptr++;
    int64 x1 = *sptr++;
    int64 x2 = *sptr++;
    int64 x3 = *rptr++;
    int64 x4 = *rptr++;


    //------ v_i32_sel2_geq_i32_b

    result = v_i32_sel2_geq_i32_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_i32_sel2_geq_i32_b(x1, ai32, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S6, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_i32_sel2_geq_i32_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_i32_sel2_geq_i32_b(x1, x2, x3, ai32, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S6, [[SPRED]]     

    result = v_i32_sel2_geq_i32_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i32_sel2_geq_i32_vb

    result = v_i32_sel2_geq_i32_vb(x1, x2, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_i32_sel2_geq_i32_vb(x1, ai32, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S6, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_i32_sel2_geq_i32_vb(x1, 123, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_i32_sel2_geq_i32_vb(x1, x2, x3, ai32, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S6, [[VPRED]]     

    result = v_i32_sel2_geq_i32_vb(x1, x2, x3, 123, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 


  if (dest > 22) {
    volatile int64_float64 __local *dptr = (int64_float64 __local *)dest_ptr;
    volatile float64 __local *sptr = (float64 __local *)src_ptr;
    volatile int64 __local *rptr = (int64 __local *)res_ptr;

    int64_float64 result = *dptr++;
    float64 x1 = *sptr++;
    float64 x2 = *sptr++;
    int64 x3 = *rptr++;
    int64 x4 = *rptr++;


    //------ v_i32_sel2_geq_f32_b

    result = v_i32_sel2_geq_f32_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_i32_sel2_geq_f32_b(x1, af32, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S5, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_i32_sel2_geq_f32_b(x1, 1.5, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_i32_sel2_geq_f32_b(x1, x2, x3, ai32, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S6, [[SPRED]]     

    result = v_i32_sel2_geq_f32_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i32_sel2_geq_f32_vb

    result = v_i32_sel2_geq_f32_vb(x1, x2, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_i32_sel2_geq_f32_vb(x1, af32, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S5, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_i32_sel2_geq_f32_vb(x1, 1.5, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_i32_sel2_geq_f32_vb(x1, x2, x3, ai32, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S6, [[VPRED]]     

    result = v_i32_sel2_geq_f32_vb(x1, x2, x3, 123, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 


  if (dest > 23) {
    volatile int64_uint64 __local *dptr = (int64_uint64 __local *)dest_ptr;
    volatile uint64 __local *sptr = (uint64 __local *)src_ptr;
    volatile int64 __local *rptr = (int64 __local *)res_ptr;

    int64_uint64 result = *dptr++;
    uint64 x1 = *sptr++;
    uint64 x2 = *sptr++;
    int64 x3 = *rptr++;
    int64 x4 = *rptr++;


    //------ v_i32_sel2_geq_u32_b

    result = v_i32_sel2_geq_u32_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_i32_sel2_geq_u32_b(x1, au32, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S7, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_i32_sel2_geq_u32_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_i32_sel2_geq_u32_b(x1, x2, x3, ai32, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S6, [[SPRED]]     

    result = v_i32_sel2_geq_u32_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i32_sel2_geq_u32_vb

    result = v_i32_sel2_geq_u32_vb(x1, x2, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_i32_sel2_geq_u32_vb(x1, au32, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S7, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_i32_sel2_geq_u32_vb(x1, 123, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_i32_sel2_geq_u32_vb(x1, x2, x3, ai32, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S6, [[VPRED]]     

    result = v_i32_sel2_geq_u32_vb(x1, x2, x3, 123, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 


  if (dest > 24) {
    volatile uint64_uint64 __local *dptr = (uint64_uint64 __local *)dest_ptr;
    volatile uint64 __local *sptr = (uint64 __local *)src_ptr;
    volatile uint64 __local *rptr = (uint64 __local *)res_ptr;

    uint64_uint64 result = *dptr++;
    uint64 x1 = *sptr++;
    uint64 x2 = *sptr++;
    uint64 x3 = *rptr++;
    uint64 x4 = *rptr++;


    //------ v_u32_sel2_geq_u32_b

    result = v_u32_sel2_geq_u32_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_u32_sel2_geq_u32_b(x1, au32, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S7, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_u32_sel2_geq_u32_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_u32_sel2_geq_u32_b(x1, x2, x3, au32, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S7, [[SPRED]]     

    result = v_u32_sel2_geq_u32_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u32_sel2_geq_u32_vb

    result = v_u32_sel2_geq_u32_vb(x1, x2, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_u32_sel2_geq_u32_vb(x1, au32, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S7, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_u32_sel2_geq_u32_vb(x1, 123, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_u32_sel2_geq_u32_vb(x1, x2, x3, au32, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S7, [[VPRED]]     

    result = v_u32_sel2_geq_u32_vb(x1, x2, x3, 123, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 


  if (dest > 25) {
    volatile uint64_float64 __local *dptr = (uint64_float64 __local *)dest_ptr;
    volatile float64 __local *sptr = (float64 __local *)src_ptr;
    volatile uint64 __local *rptr = (uint64 __local *)res_ptr;

    uint64_float64 result = *dptr++;
    float64 x1 = *sptr++;
    float64 x2 = *sptr++;
    uint64 x3 = *rptr++;
    uint64 x4 = *rptr++;


    //------ v_u32_sel2_geq_f32_b

    result = v_u32_sel2_geq_f32_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_u32_sel2_geq_f32_b(x1, af32, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S5, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_u32_sel2_geq_f32_b(x1, 1.5, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_u32_sel2_geq_f32_b(x1, x2, x3, au32, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S7, [[SPRED]]     

    result = v_u32_sel2_geq_f32_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u32_sel2_geq_f32_vb

    result = v_u32_sel2_geq_f32_vb(x1, x2, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_u32_sel2_geq_f32_vb(x1, af32, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S5, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_u32_sel2_geq_f32_vb(x1, 1.5, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_u32_sel2_geq_f32_vb(x1, x2, x3, au32, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S7, [[VPRED]]     

    result = v_u32_sel2_geq_f32_vb(x1, x2, x3, 123, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.f32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 


  if (dest > 26) {
    volatile uint64_int64 __local *dptr = (uint64_int64 __local *)dest_ptr;
    volatile int64 __local *sptr = (int64 __local *)src_ptr;
    volatile uint64 __local *rptr = (uint64 __local *)res_ptr;

    uint64_int64 result = *dptr++;
    int64 x1 = *sptr++;
    int64 x2 = *sptr++;
    uint64 x3 = *rptr++;
    uint64 x4 = *rptr++;


    //------ v_u32_sel2_geq_i32_b

    result = v_u32_sel2_geq_i32_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_u32_sel2_geq_i32_b(x1, ai32, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S6, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_u32_sel2_geq_i32_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_u32_sel2_geq_i32_b(x1, x2, x3, au32, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S7, [[SPRED]]     

    result = v_u32_sel2_geq_i32_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u32_sel2_geq_i32_vb

    result = v_u32_sel2_geq_i32_vb(x1, x2, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_u32_sel2_geq_i32_vb(x1, ai32, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %S6, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_u32_sel2_geq_i32_vb(x1, 123, x3, x4, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_u32_sel2_geq_i32_vb(x1, x2, x3, au32, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S7, [[VPRED]]     

    result = v_u32_sel2_geq_i32_vb(x1, x2, x3, 123, 0, result, to_bool64(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i32  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 


  if (dest > 27) {
    volatile short128_short128 __local *dptr = (short128_short128 __local *)dest_ptr;
    volatile short128 __local *sptr = (short128 __local *)src_ptr;
    volatile short128 __local *rptr = (short128 __local *)res_ptr;

    short128_short128 result = *dptr++;
    short128 x1 = *sptr++;
    short128 x2 = *sptr++;
    short128 x3 = *rptr++;
    short128 x4 = *rptr++;


    //------ v_i16_sel2_geq_i16_b

    result = v_i16_sel2_geq_i16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_i16_sel2_geq_i16_b(x1, ai16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_i16_sel2_geq_i16_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_i16_sel2_geq_i16_b(x1, x2, x3, ai16, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     

    result = v_i16_sel2_geq_i16_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i16_sel2_geq_i16_vb

    result = v_i16_sel2_geq_i16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_i16_sel2_geq_i16_vb(x1, ai16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_i16_sel2_geq_i16_vb(x1, 123, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_i16_sel2_geq_i16_vb(x1, x2, x3, ai16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     

    result = v_i16_sel2_geq_i16_vb(x1, x2, x3, 123, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

#if defined (__gaudi__) || defined(__goya2__)
    
  if (dest > 28) {
    volatile short128_bfloat128 __local *dptr = (short128_bfloat128 __local *)dest_ptr;
    volatile bfloat128 __local *sptr = (bfloat128 __local *)src_ptr;
    volatile short128 __local *rptr = (short128 __local *)res_ptr;

    short128_bfloat128 result = *dptr++;
    bfloat128 x1 = *sptr++;
    bfloat128 x2 = *sptr++;
    short128 x3 = *rptr++;
    short128 x4 = *rptr++;


    //------ v_i16_sel2_geq_bf16_b

    result = v_i16_sel2_geq_bf16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_i16_sel2_geq_bf16_b(x1, abf16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_i16_sel2_geq_bf16_b(x1, 1.5, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_i16_sel2_geq_bf16_b(x1, x2, x3, ai16, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     

    result = v_i16_sel2_geq_bf16_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i16_sel2_geq_bf16_vb

    result = v_i16_sel2_geq_bf16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_i16_sel2_geq_bf16_vb(x1, abf16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_i16_sel2_geq_bf16_vb(x1, 1.5, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_i16_sel2_geq_bf16_vb(x1, x2, x3, ai16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     

    result = v_i16_sel2_geq_bf16_vb(x1, x2, x3, 123, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined(__goya2__)
    
  if (dest > 29) {
    volatile short128_half128 __local *dptr = (short128_half128 __local *)dest_ptr;
    volatile half128 __local *sptr = (half128 __local *)src_ptr;
    volatile short128 __local *rptr = (short128 __local *)res_ptr;

    short128_half128 result = *dptr++;
    half128 x1 = *sptr++;
    half128 x2 = *sptr++;
    short128 x3 = *rptr++;
    short128 x4 = *rptr++;


    //------ v_i16_sel2_geq_f16_b

    result = v_i16_sel2_geq_f16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_i16_sel2_geq_f16_b(x1, af16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_i16_sel2_geq_f16_b(x1, 1.5, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_i16_sel2_geq_f16_b(x1, x2, x3, ai16, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     

    result = v_i16_sel2_geq_f16_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i16_sel2_geq_f16_vb

    result = v_i16_sel2_geq_f16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_i16_sel2_geq_f16_vb(x1, af16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_i16_sel2_geq_f16_vb(x1, 1.5, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_i16_sel2_geq_f16_vb(x1, x2, x3, ai16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     

    result = v_i16_sel2_geq_f16_vb(x1, x2, x3, 123, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 


  if (dest > 30) {
    volatile short128_ushort128 __local *dptr = (short128_ushort128 __local *)dest_ptr;
    volatile ushort128 __local *sptr = (ushort128 __local *)src_ptr;
    volatile short128 __local *rptr = (short128 __local *)res_ptr;

    short128_ushort128 result = *dptr++;
    ushort128 x1 = *sptr++;
    ushort128 x2 = *sptr++;
    short128 x3 = *rptr++;
    short128 x4 = *rptr++;


    //------ v_i16_sel2_geq_u16_b

    result = v_i16_sel2_geq_u16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_i16_sel2_geq_u16_b(x1, au16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_i16_sel2_geq_u16_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_i16_sel2_geq_u16_b(x1, x2, x3, ai16, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[SPRED]]     

    result = v_i16_sel2_geq_u16_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i16_sel2_geq_u16_vb

    result = v_i16_sel2_geq_u16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_i16_sel2_geq_u16_vb(x1, au16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_i16_sel2_geq_u16_vb(x1, 123, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_i16_sel2_geq_u16_vb(x1, x2, x3, ai16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S8, [[VPRED]]     

    result = v_i16_sel2_geq_u16_vb(x1, x2, x3, 123, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 


  if (dest > 31) {
    volatile ushort128_ushort128 __local *dptr = (ushort128_ushort128 __local *)dest_ptr;
    volatile ushort128 __local *sptr = (ushort128 __local *)src_ptr;
    volatile ushort128 __local *rptr = (ushort128 __local *)res_ptr;

    ushort128_ushort128 result = *dptr++;
    ushort128 x1 = *sptr++;
    ushort128 x2 = *sptr++;
    ushort128 x3 = *rptr++;
    ushort128 x4 = *rptr++;


    //------ v_u16_sel2_geq_u16_b

    result = v_u16_sel2_geq_u16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_u16_sel2_geq_u16_b(x1, au16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_u16_sel2_geq_u16_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_u16_sel2_geq_u16_b(x1, x2, x3, au16, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     

    result = v_u16_sel2_geq_u16_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u16_sel2_geq_u16_vb

    result = v_u16_sel2_geq_u16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_u16_sel2_geq_u16_vb(x1, au16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_u16_sel2_geq_u16_vb(x1, 123, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_u16_sel2_geq_u16_vb(x1, x2, x3, au16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     

    result = v_u16_sel2_geq_u16_vb(x1, x2, x3, 123, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 

#if defined (__gaudi__) || defined(__goya2__)
    
  if (dest > 32) {
    volatile ushort128_bfloat128 __local *dptr = (ushort128_bfloat128 __local *)dest_ptr;
    volatile bfloat128 __local *sptr = (bfloat128 __local *)src_ptr;
    volatile ushort128 __local *rptr = (ushort128 __local *)res_ptr;

    ushort128_bfloat128 result = *dptr++;
    bfloat128 x1 = *sptr++;
    bfloat128 x2 = *sptr++;
    ushort128 x3 = *rptr++;
    ushort128 x4 = *rptr++;


    //------ v_u16_sel2_geq_bf16_b

    result = v_u16_sel2_geq_bf16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_u16_sel2_geq_bf16_b(x1, abf16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_u16_sel2_geq_bf16_b(x1, 1.5, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_u16_sel2_geq_bf16_b(x1, x2, x3, au16, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     

    result = v_u16_sel2_geq_bf16_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u16_sel2_geq_bf16_vb

    result = v_u16_sel2_geq_bf16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_u16_sel2_geq_bf16_vb(x1, abf16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_u16_sel2_geq_bf16_vb(x1, 1.5, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_u16_sel2_geq_bf16_vb(x1, x2, x3, au16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     

    result = v_u16_sel2_geq_bf16_vb(x1, x2, x3, 123, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN2: sel2_geq.bf16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 

#if defined(__goya2__)
    
  if (dest > 33) {
    volatile ushort128_half128 __local *dptr = (ushort128_half128 __local *)dest_ptr;
    volatile half128 __local *sptr = (half128 __local *)src_ptr;
    volatile ushort128 __local *rptr = (ushort128 __local *)res_ptr;

    ushort128_half128 result = *dptr++;
    half128 x1 = *sptr++;
    half128 x2 = *sptr++;
    ushort128 x3 = *rptr++;
    ushort128 x4 = *rptr++;


    //------ v_u16_sel2_geq_f16_b

    result = v_u16_sel2_geq_f16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_u16_sel2_geq_f16_b(x1, af16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_u16_sel2_geq_f16_b(x1, 1.5, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_u16_sel2_geq_f16_b(x1, x2, x3, au16, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     

    result = v_u16_sel2_geq_f16_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u16_sel2_geq_f16_vb

    result = v_u16_sel2_geq_f16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_u16_sel2_geq_f16_vb(x1, af16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S9, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_u16_sel2_geq_f16_vb(x1, 1.5, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_u16_sel2_geq_f16_vb(x1, x2, x3, au16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     

    result = v_u16_sel2_geq_f16_vb(x1, x2, x3, 123, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // GEN3: sel2_geq.f16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
#endif
 


  if (dest > 34) {
    volatile ushort128_short128 __local *dptr = (ushort128_short128 __local *)dest_ptr;
    volatile short128 __local *sptr = (short128 __local *)src_ptr;
    volatile ushort128 __local *rptr = (ushort128 __local *)res_ptr;

    ushort128_short128 result = *dptr++;
    short128 x1 = *sptr++;
    short128 x2 = *sptr++;
    ushort128 x3 = *rptr++;
    ushort128 x4 = *rptr++;


    //------ v_u16_sel2_geq_i16_b

    result = v_u16_sel2_geq_i16_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_u16_sel2_geq_i16_b(x1, ai16, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_u16_sel2_geq_i16_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_u16_sel2_geq_i16_b(x1, x2, x3, au16, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[SPRED]]     

    result = v_u16_sel2_geq_i16_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u16_sel2_geq_i16_vb

    result = v_u16_sel2_geq_i16_vb(x1, x2, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_u16_sel2_geq_i16_vb(x1, ai16, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %S8, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_u16_sel2_geq_i16_vb(x1, 123, x3, x4, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_u16_sel2_geq_i16_vb(x1, x2, x3, au16, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S9, [[VPRED]]     

    result = v_u16_sel2_geq_i16_vb(x1, x2, x3, 123, 0, result, to_bool128(vpred), 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i16  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 


  if (dest > 35) {
    volatile char256_char256 __local *dptr = (char256_char256 __local *)dest_ptr;
    volatile char256 __local *sptr = (char256 __local *)src_ptr;
    volatile char256 __local *rptr = (char256 __local *)res_ptr;

    char256_char256 result = *dptr++;
    char256 x1 = *sptr++;
    char256 x2 = *sptr++;
    char256 x3 = *rptr++;
    char256 x4 = *rptr++;


    //------ v_i8_sel2_geq_i8_b

    result = v_i8_sel2_geq_i8_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_i8_sel2_geq_i8_b(x1, ai8, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %S10, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_i8_sel2_geq_i8_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_i8_sel2_geq_i8_b(x1, x2, x3, ai8, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S10, [[SPRED]]     

    result = v_i8_sel2_geq_i8_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i8_sel2_geq_i8_vb

    result = v_i8_sel2_geq_i8_vb(x1, x2, x3, x4, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_i8_sel2_geq_i8_vb(x1, ai8, x3, x4, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %S10, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_i8_sel2_geq_i8_vb(x1, 123, x3, x4, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_i8_sel2_geq_i8_vb(x1, x2, x3, ai8, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S10, [[VPRED]]     

    result = v_i8_sel2_geq_i8_vb(x1, x2, x3, 123, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 


  if (dest > 36) {
    volatile char256_uchar256 __local *dptr = (char256_uchar256 __local *)dest_ptr;
    volatile uchar256 __local *sptr = (uchar256 __local *)src_ptr;
    volatile char256 __local *rptr = (char256 __local *)res_ptr;

    char256_uchar256 result = *dptr++;
    uchar256 x1 = *sptr++;
    uchar256 x2 = *sptr++;
    char256 x3 = *rptr++;
    char256 x4 = *rptr++;


    //------ v_i8_sel2_geq_u8_b

    result = v_i8_sel2_geq_u8_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_i8_sel2_geq_u8_b(x1, au8, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %S11, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_i8_sel2_geq_u8_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_i8_sel2_geq_u8_b(x1, x2, x3, ai8, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S10, [[SPRED]]     

    result = v_i8_sel2_geq_u8_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_i8_sel2_geq_u8_vb

    result = v_i8_sel2_geq_u8_vb(x1, x2, x3, x4, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_i8_sel2_geq_u8_vb(x1, au8, x3, x4, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %S11, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_i8_sel2_geq_u8_vb(x1, 123, x3, x4, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_i8_sel2_geq_u8_vb(x1, x2, x3, ai8, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S10, [[VPRED]]     

    result = v_i8_sel2_geq_u8_vb(x1, x2, x3, 123, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 


  if (dest > 37) {
    volatile uchar256_uchar256 __local *dptr = (uchar256_uchar256 __local *)dest_ptr;
    volatile uchar256 __local *sptr = (uchar256 __local *)src_ptr;
    volatile uchar256 __local *rptr = (uchar256 __local *)res_ptr;

    uchar256_uchar256 result = *dptr++;
    uchar256 x1 = *sptr++;
    uchar256 x2 = *sptr++;
    uchar256 x3 = *rptr++;
    uchar256 x4 = *rptr++;


    //------ v_u8_sel2_geq_u8_b

    result = v_u8_sel2_geq_u8_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_u8_sel2_geq_u8_b(x1, au8, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %S11, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_u8_sel2_geq_u8_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_u8_sel2_geq_u8_b(x1, x2, x3, au8, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S11, [[SPRED]]     

    result = v_u8_sel2_geq_u8_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u8_sel2_geq_u8_vb

    result = v_u8_sel2_geq_u8_vb(x1, x2, x3, x4, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_u8_sel2_geq_u8_vb(x1, au8, x3, x4, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %S11, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_u8_sel2_geq_u8_vb(x1, 123, x3, x4, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_u8_sel2_geq_u8_vb(x1, x2, x3, au8, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S11, [[VPRED]]     

    result = v_u8_sel2_geq_u8_vb(x1, x2, x3, 123, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.u8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 


  if (dest > 38) {
    volatile uchar256_char256 __local *dptr = (uchar256_char256 __local *)dest_ptr;
    volatile char256 __local *sptr = (char256 __local *)src_ptr;
    volatile uchar256 __local *rptr = (uchar256 __local *)res_ptr;

    uchar256_char256 result = *dptr++;
    char256 x1 = *sptr++;
    char256 x2 = *sptr++;
    uchar256 x3 = *rptr++;
    uchar256 x4 = *rptr++;


    //------ v_u8_sel2_geq_i8_b

    result = v_u8_sel2_geq_i8_b(x1, x2, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]     

    result = v_u8_sel2_geq_i8_b(x1, ai8, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %S10, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]    

    result = v_u8_sel2_geq_i8_b(x1, 123, x3, x4, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[SPRED]]   

    result = v_u8_sel2_geq_i8_b(x1, x2, x3, au8, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S11, [[SPRED]]     

    result = v_u8_sel2_geq_i8_b(x1, x2, x3, 123, 0, result, spred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[SPRED]]

    //------ v_u8_sel2_geq_i8_vb

    result = v_u8_sel2_geq_i8_vb(x1, x2, x3, x4, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]     

    result = v_u8_sel2_geq_i8_vb(x1, ai8, x3, x4, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %S10, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]    

    result = v_u8_sel2_geq_i8_vb(x1, 123, x3, x4, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]   

    result = v_u8_sel2_geq_i8_vb(x1, x2, x3, au8, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %S11, [[VPRED]]     

    result = v_u8_sel2_geq_i8_vb(x1, x2, x3, 123, 0, result, vpred, 0);
    *dptr++ = result;
    // CHECK: sel2_geq.i8  %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, [[VPRED]]

    dest_ptr = (int64_int64 __local *)dptr;
    src_ptr = (int64 __local *)sptr;
    res_ptr = (int64 __local *)rptr;
  }
 
}