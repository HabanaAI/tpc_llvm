// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK,GEN2 %s


void main(int dest0, int dest1, int x, short y, char z) {
  _Bool pred = x < dest1;
  // CHECK-DAG: cmp_less.i32 [[PRED:%SP[0-9]+]], %S2, %S1

  volatile int64 __local *dptr = (int64 __local *)dest0;
  volatile int64 __local *sptr = (int64 __local *)dest1;

  // v_f32_mov_v_b
  {
    volatile float64 __local *dfptr = (float64 __local *)dptr;
    volatile float64 __local *sfptr = (float64 __local *)sptr;
    float64 res = *dfptr++;
    float64 x = *sfptr++;
    
    res = v_f32_mov_v_b(x, res, pred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_f32_mov_v_vb
  {
    volatile float64 __local *dfptr = (float64 __local *)dptr;
    volatile float64 __local *sfptr = (float64 __local *)sptr;
    bool256 vpred = bv_f32_cmp_grt_v_s(*sfptr++, 0.0);
    // CHECK: cmp_grt.f32 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    float64 res = *sfptr++;
    float64 x = *sfptr++;
    
    res = v_f32_mov_v_vb(x, res, vpred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_f32_mov_v
  {
    volatile float64 __local *dfptr = (float64 __local *)dptr;
    volatile float64 __local *sfptr = (float64 __local *)sptr;
    float64 x = *sfptr++;
    float64 res;
    
    res = v_f32_mov_v(x);
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_f32_mov_s_b
  {
    volatile float64 __local *dfptr = (float64 __local *)dptr;
    float64 res = *dfptr++;
    
    res = v_f32_mov_s_b(as_float(x), res, pred, 0);
    // CHECK: mov.f32  %V{{[0-9]+}}, %S2, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }

  // v_f32_mov_s_vb
  {
    volatile float64 __local *dfptr = (float64 __local *)dptr;
    volatile float64 __local *sfptr = (float64 __local *)sptr;
    bool256 vpred = bv_f32_cmp_grt_v_s(*sfptr++, 0.0);
    // CHECK: cmp_grt.f32 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    float64 res = *sfptr++;
    
    res = v_f32_mov_s_vb(as_float(x), res, vpred, 0);
    // CHECK: mov.f32  %V{{[0-9]+}}, %S2, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_f32_mov_s
  {
    volatile float64 __local *dfptr = (float64 __local *)dptr;
    float64 res;

    res = v_f32_mov_s(as_float(x));
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }
  
#if defined(__gaudi__)
  // v_bf16_mov_v_b
  {
    volatile bfloat128 __local *dfptr = (bfloat128 __local *)dptr;
    volatile bfloat128 __local *sfptr = (bfloat128 __local *)sptr;
    bfloat128 res = *dfptr++;
    bfloat128 x = *sfptr++;
    
    res = v_bf16_mov_v_b(x, res, pred, 0);
    // GEN2: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_bf16_mov_v_vb
  {
    volatile bfloat128 __local *dfptr = (bfloat128 __local *)dptr;
    volatile bfloat128 __local *sfptr = (bfloat128 __local *)sptr;
    bool256 vpred = bv_bf16_cmp_grt_v_s(*sfptr++, 0.0);
    // GEN2: cmp_grt.bf16 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    bfloat128 res = *sfptr++;
    bfloat128 x = *sfptr++;
    
    res = v_bf16_mov_v_vb(x, res, vpred, 0);
    // GEN2: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_bf16_mov_v
  {
    volatile bfloat128 __local *dfptr = (bfloat128 __local *)dptr;
    volatile bfloat128 __local *sfptr = (bfloat128 __local *)sptr;
    bfloat128 x = *sfptr++;
    bfloat128 res;
    
    res = v_bf16_mov_v(x);
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_bf16_mov_s_b
  {
    volatile bfloat128 __local *dfptr = (bfloat128 __local *)dptr;
    bfloat128 res = *dfptr++;
    bfloat x = as_bfloat(y);
    
    res = v_bf16_mov_s_b(x, res, pred, 0);
    // GEN2: mov.bf16  %V{{[0-9]+}}, %S3, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }

  // v_bf16_mov_s_vb
  {
    volatile bfloat128 __local *dfptr = (bfloat128 __local *)dptr;
    volatile bfloat128 __local *sfptr = (bfloat128 __local *)sptr;
    bool256 vpred = bv_bf16_cmp_grt_v_s(*sfptr++, 0.0);
    // GEN2: cmp_grt.bf16 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    bfloat128 res = *sfptr++;
    bfloat x = as_bfloat(y);
   
    res = v_bf16_mov_s_vb(x, res, vpred, 0);
    // GEN2: mov.bf16  %V{{[0-9]+}}, %S3, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_bf16_mov_s
  {
    volatile bfloat128 __local *dfptr = (bfloat128 __local *)dptr;
    bfloat128 res;
    bfloat x = as_bfloat(y);

    res = v_bf16_mov_s(x);
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }
#endif

  // v_i32_mov_v_b
  {
    volatile int64 __local *dfptr = (int64 __local *)dptr;
    volatile int64 __local *sfptr = (int64 __local *)sptr;
    int64 res = *dfptr++;
    int64 x = *sfptr++;
    
    res = v_i32_mov_v_b(x, res, pred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i32_mov_v_vb
  {
    volatile int64 __local *dfptr = (int64 __local *)dptr;
    volatile int64 __local *sfptr = (int64 __local *)sptr;
    bool256 vpred = bv_i32_cmp_grt_v_s(*sfptr++, 0.0);
    // CHECK: cmp_grt.i32 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    int64 res = *sfptr++;
    int64 x = *sfptr++;
    
    res = v_i32_mov_v_vb(x, res, vpred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i32_mov_v
  {
    volatile int64 __local *dfptr = (int64 __local *)dptr;
    volatile int64 __local *sfptr = (int64 __local *)sptr;
    int64 x = *sfptr++;
    int64 res;
    
    res = v_i32_mov_v(x);
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i32_mov_s_b
  {
    volatile int64 __local *dfptr = (int64 __local *)dptr;
    int64 res = *dfptr++;
    
    res = v_i32_mov_s_b(x, res, pred, 0);
    // CHECK: mov.i32  %V{{[0-9]+}}, %S2, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }

  // v_i32_mov_s_vb
  {
    volatile int64 __local *dfptr = (int64 __local *)dptr;
    volatile int64 __local *sfptr = (int64 __local *)sptr;
    bool256 vpred = bv_i32_cmp_grt_v_s(*sfptr++, 0.0);
    // CHECK: cmp_grt.i32 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    int64 res = *sfptr++;
    
    res = v_i32_mov_s_vb(x, res, vpred, 0);
    // CHECK: mov.i32  %V{{[0-9]+}}, %S2, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i32_mov_s
  {
    volatile int64 __local *dfptr = (int64 __local *)dptr;
    int64 res;

    res = v_i32_mov_s(x);
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }

  // v_u32_mov_v_b
  {
    volatile uint64 __local *dfptr = (uint64 __local *)dptr;
    volatile uint64 __local *sfptr = (uint64 __local *)sptr;
    uint64 res = *dfptr++;
    uint64 x = *sfptr++;
    
    res = v_u32_mov_v_b(x, res, pred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u32_mov_v_vb
  {
    volatile uint64 __local *dfptr = (uint64 __local *)dptr;
    volatile uint64 __local *sfptr = (uint64 __local *)sptr;
    bool256 vpred = bv_u32_cmp_grt_v_s(*sfptr++, 0.0);
    // CHECK: cmp_grt.u32 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    uint64 res = *sfptr++;
    uint64 x = *sfptr++;
    
    res = v_u32_mov_v_vb(x, res, vpred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u32_mov_v
  {
    volatile uint64 __local *dfptr = (uint64 __local *)dptr;
    volatile uint64 __local *sfptr = (uint64 __local *)sptr;
    uint64 x = *sfptr++;
    uint64 res;
    
    res = v_u32_mov_v(x);
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u32_mov_s_b
  {
    volatile uint64 __local *dfptr = (uint64 __local *)dptr;
    uint64 res = *dfptr++;
    
    res = v_u32_mov_s_b(as_uint(x), res, pred, 0);
    // CHECK: mov.u32  %V{{[0-9]+}}, %S2, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }

  // v_u32_mov_s_vb
  {
    volatile uint64 __local *dfptr = (uint64 __local *)dptr;
    volatile uint64 __local *sfptr = (uint64 __local *)sptr;
    bool256 vpred = bv_u32_cmp_grt_v_s(*sfptr++, 0.0);
    // CHECK: cmp_grt.u32 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    uint64 res = *sfptr++;
    
    res = v_u32_mov_s_vb(as_uint(x), res, vpred, 0);
    // CHECK: mov.u32  %V{{[0-9]+}}, %S2, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u32_mov_s
  {
    volatile uint64 __local *dfptr = (uint64 __local *)dptr;
    uint64 res;

    res = v_u32_mov_s(as_uint(x));
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }

  // v_i16_mov_v_b
  {
    volatile short128 __local *dfptr = (short128 __local *)dptr;
    volatile short128 __local *sfptr = (short128 __local *)sptr;
    short128 res = *dfptr++;
    short128 x = *sfptr++;
    
    res = v_i16_mov_v_b(x, res, pred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i16_mov_v_vb
  {
    volatile short128 __local *dfptr = (short128 __local *)dptr;
    volatile short128 __local *sfptr = (short128 __local *)sptr;
    bool256 vpred = bv_i16_cmp_grt_v_s(*sfptr++, 0.0);
    // CHECK: cmp_grt.i16 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    short128 res = *sfptr++;
    short128 x = *sfptr++;
    
    res = v_i16_mov_v_vb(x, res, vpred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i16_mov_v
  {
    volatile short128 __local *dfptr = (short128 __local *)dptr;
    volatile short128 __local *sfptr = (short128 __local *)sptr;
    short128 x = *sfptr++;
    short128 res;
    
    res = v_i16_mov_v(x);
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i16_mov_s_b
  {
    volatile short128 __local *dfptr = (short128 __local *)dptr;
    short128 res = *dfptr++;
    
    res = v_i16_mov_s_b(y, res, pred, 0);
    // CHECK: mov.i16  %V{{[0-9]+}}, %S3, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }

  // v_i16_mov_s_vb
  {
    volatile short128 __local *dfptr = (short128 __local *)dptr;
    volatile short128 __local *sfptr = (short128 __local *)sptr;
    bool256 vpred = bv_i16_cmp_grt_v_s(*sfptr++, 0.0);
    // CHECK: cmp_grt.i16 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    short128 res = *sfptr++;
    
    res = v_i16_mov_s_vb(y, res, vpred, 0);
    // CHECK: mov.i16  %V{{[0-9]+}}, %S3, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i16_mov_s
  {
    volatile short128 __local *dfptr = (short128 __local *)dptr;
    short128 res;

    res = v_i16_mov_s(y);
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }

  // v_u16_mov_v_b
  {
    volatile ushort128 __local *dfptr = (ushort128 __local *)dptr;
    volatile ushort128 __local *sfptr = (ushort128 __local *)sptr;
    ushort128 res = *dfptr++;
    ushort128 x = *sfptr++;
    
    res = v_u16_mov_v_b(x, res, pred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u16_mov_v_vb
  {
    volatile ushort128 __local *dfptr = (ushort128 __local *)dptr;
    volatile ushort128 __local *sfptr = (ushort128 __local *)sptr;
    bool256 vpred = bv_u16_cmp_grt_v_s(*sfptr++, 0.0);
    // CHECK: cmp_grt.u16 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    ushort128 res = *sfptr++;
    ushort128 x = *sfptr++;
    
    res = v_u16_mov_v_vb(x, res, vpred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u16_mov_v
  {
    volatile ushort128 __local *dfptr = (ushort128 __local *)dptr;
    volatile ushort128 __local *sfptr = (ushort128 __local *)sptr;
    ushort128 x = *sfptr++;
    ushort128 res;
    
    res = v_u16_mov_v(x);
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u16_mov_s_b
  {
    volatile ushort128 __local *dfptr = (ushort128 __local *)dptr;
    ushort128 res = *dfptr++;
    
    res = v_u16_mov_s_b(as_ushort(y), res, pred, 0);
    // CHECK: mov.u16  %V{{[0-9]+}}, %S3, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }

  // v_u16_mov_s_vb
  {
    volatile ushort128 __local *dfptr = (ushort128 __local *)dptr;
    volatile ushort128 __local *sfptr = (ushort128 __local *)sptr;
    bool256 vpred = bv_u16_cmp_grt_v_s(*sfptr++, 0.0);
    // CHECK: cmp_grt.u16 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    ushort128 res = *sfptr++;
    
    res = v_u16_mov_s_vb(as_ushort(y), res, vpred, 0);
    // CHECK: mov.u16  %V{{[0-9]+}}, %S3, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u16_mov_s
  {
    volatile ushort128 __local *dfptr = (ushort128 __local *)dptr;
    ushort128 res;

    res = v_u16_mov_s(as_ushort(y));
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }


  // v_i8_mov_v_b
  {
    volatile char256 __local *dfptr = (char256 __local *)dptr;
    volatile char256 __local *sfptr = (char256 __local *)sptr;
    char256 res = *dfptr++;
    char256 x = *sfptr++;
    
    res = v_i8_mov_v_b(x, res, pred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i8_mov_v_vb
  {
    volatile char256 __local *dfptr = (char256 __local *)dptr;
    volatile char256 __local *sfptr = (char256 __local *)sptr;
    bool256 vpred = bv_i8_cmp_grt_v_s(*sfptr++, 0.0);
    // CHECK: cmp_grt.i8 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    char256 res = *sfptr++;
    char256 x = *sfptr++;
    
    res = v_i8_mov_v_vb(x, res, vpred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i8_mov_v
  {
    volatile char256 __local *dfptr = (char256 __local *)dptr;
    volatile char256 __local *sfptr = (char256 __local *)sptr;
    char256 x = *sfptr++;
    char256 res;
    
    res = v_i8_mov_v(x);
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i8_mov_s_b
  {
    volatile char256 __local *dfptr = (char256 __local *)dptr;
    char256 res = *dfptr++;
    
    res = v_i8_mov_s_b(z, res, pred, 0);
    // CHECK: mov.i8  %V{{[0-9]+}}, %S4, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }

  // v_i8_mov_s_vb
  {
    volatile char256 __local *dfptr = (char256 __local *)dptr;
    volatile char256 __local *sfptr = (char256 __local *)sptr;
    bool256 vpred = bv_i8_cmp_grt_v_s(*sfptr++, 0.0);
    // CHECK: cmp_grt.i8 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    char256 res = *sfptr++;
    
    res = v_i8_mov_s_vb(z, res, vpred, 0);
    // CHECK: mov.i8  %V{{[0-9]+}}, %S4, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i8_mov_s
  {
    volatile char256 __local *dfptr = (char256 __local *)dptr;
    char256 res;

    res = v_i8_mov_s(y);
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }

  // v_u8_mov_v_b
  {
    volatile uchar256 __local *dfptr = (uchar256 __local *)dptr;
    volatile uchar256 __local *sfptr = (uchar256 __local *)sptr;
    uchar256 res = *dfptr++;
    uchar256 x = *sfptr++;
    
    res = v_u8_mov_v_b(x, res, pred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u8_mov_v_vb
  {
    volatile uchar256 __local *dfptr = (uchar256 __local *)dptr;
    volatile uchar256 __local *sfptr = (uchar256 __local *)sptr;
    bool256 vpred = bv_u8_cmp_grt_v_s(*sfptr++, 0.0);
    // CHECK: cmp_grt.u8 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    uchar256 res = *sfptr++;
    uchar256 x = *sfptr++;
    
    res = v_u8_mov_v_vb(x, res, vpred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u8_mov_v
  {
    volatile uchar256 __local *dfptr = (uchar256 __local *)dptr;
    volatile uchar256 __local *sfptr = (uchar256 __local *)sptr;
    uchar256 x = *sfptr++;
    uchar256 res;
    
    res = v_u8_mov_v(x);
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u8_mov_s_b
  {
    volatile uchar256 __local *dfptr = (uchar256 __local *)dptr;
    uchar256 res = *dfptr++;
    
    res = v_u8_mov_s_b(as_uchar(z), res, pred, 0);
    // CHECK: mov.u8  %V{{[0-9]+}}, %S4, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }

  // v_u8_mov_s_vb
  {
    volatile uchar256 __local *dfptr = (uchar256 __local *)dptr;
    volatile uchar256 __local *sfptr = (uchar256 __local *)sptr;
    bool256 vpred = bv_u8_cmp_grt_v_s(*sfptr++, 0.0);
    // CHECK: cmp_grt.u8 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    uchar256 res = *sfptr++;
    
    res = v_u8_mov_s_vb(as_uchar(z), res, vpred, 0);
    // CHECK: mov.u8  %V{{[0-9]+}}, %S4, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u8_mov_s
  {
    volatile uchar256 __local *dfptr = (uchar256 __local *)dptr;
    uchar256 res;

    res = v_u8_mov_s(as_uchar(z));
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
  }
}