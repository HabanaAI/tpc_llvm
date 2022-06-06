// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK,GEN2 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK,GEN2,GEN3 %s


void main(int dest0, int dest1, int xi, short xs, char xc) {
  _Bool pred = xi < dest1;
  // CHECK-DAG: cmp_less.i32 [[PRED:%SP[0-9]+]], %S2, %S1

  volatile int64 __local *dptr = (int64 __local *)dest0;
  volatile int64 __local *sptr = (int64 __local *)dest1;

  // v_f32_mov_b
  {
    volatile float64 __local *dfptr = (float64 __local *)dptr;
    volatile float64 __local *sfptr = (float64 __local *)sptr;
    float64 res = *dfptr++;
    float64 x = *sfptr++;
    float xscal = as_float(xi);
    
    res = v_f32_mov_b(x, 0, res, pred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;
    
    res = v_f32_mov_b(xscal, 0, res, pred, 0);
    // CHECK: mov.f32  %V{{[0-9]+}}, %S2, [[PRED]]
    *dfptr++ = res;
    
    res = v_f32_mov_b(1.0, 0, res, pred, 0);
    // CHECK: mov.f32  %V{{[0-9]+}}, 0x3f800000, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_f32_mov_vb
  {
    volatile float64 __local *dfptr = (float64 __local *)dptr;
    volatile float64 __local *sfptr = (float64 __local *)sptr;
    bool64 vpred = v_f32_cmp_grt_b(*sfptr++, 0.0, 0, (bool64){0}, 1, 0);
    // CHECK: cmp_grt.f32 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    float64 res = *sfptr++;
    float64 x = *sfptr++;
    float xscal = as_float(xi);
  
    res = v_f32_mov_vb(x, 0, res, vpred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;
  
    res = v_f32_mov_vb(xscal, 0, res, vpred, 0);
    // CHECK: mov.f32  %V{{[0-9]+}}, %S2, [[VPRED]]
    *dfptr++ = res;
  
    res = v_f32_mov_vb(1.0, 0, res, vpred, 0);
    // CHECK: mov.f32  %V{{[0-9]+}}, 0x3f800000, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

#if defined(__gaudi__) || defined(__goya2__)
  // v_bf16_mov_b
  {
    volatile bfloat128 __local *dfptr = (bfloat128 __local *)dptr;
    volatile bfloat128 __local *sfptr = (bfloat128 __local *)sptr;
    bfloat128 res = *sfptr++;
    bfloat128 x = *sfptr++;
    bfloat xscal = as_bfloat(xs);
    
    res = v_bf16_mov_b(x, 0, res, pred, 0);
    // GEN2: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;
    
    res = v_bf16_mov_b(xscal, 0, res, pred, 0);
    // GEN2: mov.bf16  %V{{[0-9]+}}, %S3, [[PRED]]
    *dfptr++ = res;
    
    res = v_bf16_mov_b(1.0, 0, res, pred, 0);
    // GEN2: mov.bf16  %V{{[0-9]+}}, 0x3f80, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_bf16_mov_vb
  {
    volatile bfloat128 __local *dfptr = (bfloat128 __local *)dptr;
    volatile bfloat128 __local *sfptr = (bfloat128 __local *)sptr;
    bool128 vpred = v_bf16_cmp_grt_b(*sfptr++, 0.0, 0, (bool128){0}, 1, 0);
    // GEN2: cmp_grt.bf16 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    bfloat128 res = *sfptr++;
    bfloat128 x = *sfptr++;
    bfloat xscal = as_bfloat(xs);
   
    res = v_bf16_mov_vb(x, 0, res, vpred, 0);
    // GEN2: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;
   
    res = v_bf16_mov_vb(xscal, 0, res, vpred, 0);
    // GEN2: mov.bf16  %V{{[0-9]+}}, %S3, [[VPRED]]
    *dfptr++ = res;
   
    res = v_bf16_mov_vb(1.0, 0, res, vpred, 0);
    // GEN2: mov.bf16  %V{{[0-9]+}}, 0x3f80, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }
#endif
/*
#if defined(__goya2__)

  // v_f16_mov_b
  {
    volatile half128 __local *dfptr = (half128 __local *)dptr;
    volatile half128 __local *sfptr = (half128 __local *)sptr;
    half256 res = *sfptr;
    half256 x = *sfptr;
    half xscal = as_half(xs);
    
    res = v_f16_mov_b(x, 0, res, pred, 0);
    // G EN3: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;
    
    res = v_f16_mov_b(xscal, 0, res, pred, 0);
    // G EN3: mov.f16  %V{{[0-9]+}}, %S3, [[PRED]]
    *dfptr++ = res;
    
    res = v_f16_mov_b(1.0, 0, res, pred, 0);
    // G EN3: mov.f16  %V{{[0-9]+}}, 0x3c00, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_f16_mov_vb
  {
    volatile half128 __local *dfptr = (half128 __local *)dptr;
    volatile half128 __local *sfptr = (half128 __local *)sptr;
    bool128 vpred = v_f16_cmp_grt_b(*sfptr++, 0.0, 0, (bool128){0}, 1, 0);
    // G EN3: cmp_grt.f16 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    half256 res = *sfptr;
    half256 x = *sfptr;
    half xscal = as_half(xs);
    
    res = v_f16_mov_vb(x, 0, res, vpred, 0);
    // G EN3: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;
    
    res = v_f16_mov_vb(xscal, 0, res, vpred, 0);
    // G EN3: mov.f16  %V{{[0-9]+}}, %S3, [[VPRED]]
    *dfptr++ = res;
    
    res = v_f16_mov_vb(1.0, 0, res, vpred, 0);
    // G EN3: mov.f16  %V{{[0-9]+}}, 0x3c00, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }
#endif
*/

  // v_i32_mov_b
  {
    volatile int64 __local *dfptr = (int64 __local *)dptr;
    volatile int64 __local *sfptr = (int64 __local *)sptr;
    int64 res = *sfptr++;
    int64 x = *sfptr++;
    int xscal = as_int(xi);
   
    res = v_i32_mov_b(x, 0, res, pred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;
   
    res = v_i32_mov_b(xscal, 0, res, pred, 0);
    // CHECK: mov.i32  %V{{[0-9]+}}, %S2, [[PRED]]
    *dfptr++ = res;
   
    res = v_i32_mov_b(123, 0, res, pred, 0);
    // CHECK: mov.i32  %V{{[0-9]+}}, 0x7b, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i32_mov_vb
  {
    volatile int64 __local *dfptr = (int64 __local *)dptr;
    volatile int64 __local *sfptr = (int64 __local *)sptr;
    bool64 vpred = v_i32_cmp_grt_b(*sfptr++, 0.0, 0, (bool64){0}, 1, 0);
    // CHECK: cmp_grt.i32 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    int64 res = *sfptr++;
    int64 x = *sfptr++;
    int xscal = as_int(xi);
    
    res = v_i32_mov_vb(x, 0, res, vpred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;
    
    res = v_i32_mov_vb(xscal, 0, res, vpred, 0);
    // CHECK: mov.i32  %V{{[0-9]+}}, %S2, [[VPRED]]
    *dfptr++ = res;
    
    res = v_i32_mov_vb(123, 0, res, vpred, 0);
    // CHECK: mov.i32  %V{{[0-9]+}}, 0x7b, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u32_mov_b
  {
    volatile uint64 __local *dfptr = (uint64 __local *)dptr;
    volatile uint64 __local *sfptr = (uint64 __local *)sptr;
    uint64 res = *sfptr++;
    uint64 x = *sfptr++;
    unsigned int xscal = as_uint(xi);
    
    res = v_u32_mov_b(x, 0, res, pred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;
    
    res = v_u32_mov_b(xscal, 0, res, pred, 0);
    // CHECK: mov.u32  %V{{[0-9]+}}, %S2, [[PRED]]
    *dfptr++ = res;
    
    res = v_u32_mov_b(123, 0, res, pred, 0);
    // CHECK: mov.u32  %V{{[0-9]+}}, 0x7b, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u32_mov_vb
  {
    volatile uint64 __local *dfptr = (uint64 __local *)dptr;
    volatile uint64 __local *sfptr = (uint64 __local *)sptr;
    bool64 vpred = v_u32_cmp_grt_b(*sfptr++, 0.0, 0, (bool64){0}, 1, 0);
    // CHECK: cmp_grt.u32 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    uint64 res = *sfptr++;
    uint64 x = *sfptr++;
    unsigned int xscal = as_uint(xi);
    
    res = v_u32_mov_vb(x, 0, res, vpred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;
    
    res = v_u32_mov_vb(xscal, 0, res, vpred, 0);
    // CHECK: mov.u32  %V{{[0-9]+}}, %S2, [[VPRED]]
    *dfptr++ = res;
    
    res = v_u32_mov_vb(123, 0, res, vpred, 0);
    // CHECK: mov.u32  %V{{[0-9]+}}, 0x7b, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i16_mov_b
  {
    volatile short128 __local *dfptr = (short128 __local *)dptr;
    volatile short128 __local *sfptr = (short128 __local *)sptr;
    short128 res = *sfptr++;
    short128 x = *sfptr++;
    short xscal = as_short(xs);
    
    res = v_i16_mov_b(x, 0, res, pred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;
    
    res = v_i16_mov_b(xscal, 0, res, pred, 0);
    // CHECK: mov.i16  %V{{[0-9]+}}, %S3, [[PRED]]
    *dfptr++ = res;
    
    res = v_i16_mov_b(123, 0, res, pred, 0);
    // CHECK: mov.i16  %V{{[0-9]+}}, 0x7b, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i16_mov_vb
  {
    volatile short128 __local *dfptr = (short128 __local *)dptr;
    volatile short128 __local *sfptr = (short128 __local *)sptr;
    bool128 vpred = v_i16_cmp_grt_b(*sfptr++, 0.0, 0, (bool128){0}, 1, 0);
    // CHECK: cmp_grt.i16 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    short128 res = *sfptr++;
    short128 x = *sfptr++;
    unsigned short xscal = as_short(xs);
    
    res = v_i16_mov_vb(x, 0, res, vpred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;
    
    res = v_i16_mov_vb(xscal, 0, res, vpred, 0);
    // CHECK: mov.i16  %V{{[0-9]+}}, %S3, [[VPRED]]
    *dfptr++ = res;
    
    res = v_i16_mov_vb(123, 0, res, vpred, 0);
    // CHECK: mov.i16  %V{{[0-9]+}}, 0x7b, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u16_mov_b
  {
    volatile ushort128 __local *dfptr = (ushort128 __local *)dptr;
    volatile ushort128 __local *sfptr = (ushort128 __local *)sptr;
    ushort128 res = *sfptr++;
    ushort128 x = *sfptr++;
    unsigned short xscal = as_ushort(xs);
    
    res = v_u16_mov_b(x, 0, res, pred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;
    
    res = v_u16_mov_b(xscal, 0, res, pred, 0);
    // CHECK: mov.u16  %V{{[0-9]+}}, %S3, [[PRED]]
    *dfptr++ = res;
    
    res = v_u16_mov_b(123, 0, res, pred, 0);
    // CHECK: mov.u16  %V{{[0-9]+}}, 0x7b, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u16_mov_vb
  {
    volatile ushort128 __local *dfptr = (ushort128 __local *)dptr;
    volatile ushort128 __local *sfptr = (ushort128 __local *)sptr;
    bool128 vpred = v_u16_cmp_grt_b(*sfptr++, 0.0, 0, (bool128){0}, 1, 0);
    // CHECK: cmp_grt.u16 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    ushort128 res = *sfptr++;
    ushort128 x = *sfptr++;
    unsigned short xscal = as_ushort(xs);
    
    res = v_u16_mov_vb(x, 0, res, vpred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;
    
    res = v_u16_mov_vb(xscal, 0, res, vpred, 0);
    // CHECK: mov.u16  %V{{[0-9]+}}, %S3, [[VPRED]]
    *dfptr++ = res;
    
    res = v_u16_mov_vb(123, 0, res, vpred, 0);
    // CHECK: mov.u16  %V{{[0-9]+}}, 0x7b, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i8_mov_b
  {
    volatile char256 __local *dfptr = (char256 __local *)dptr;
    volatile char256 __local *sfptr = (char256 __local *)sptr;
    char256 res = *sfptr++;
    char256 x = *sfptr++;
    char xscal = as_char(xc);
    
    res = v_i8_mov_b(x, 0, res, pred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;
    
    res = v_i8_mov_b(xscal, 0, res, pred, 0);
    // CHECK: mov.i8  %V{{[0-9]+}}, %S4, [[PRED]]
    *dfptr++ = res;
    
    res = v_i8_mov_b(123, 0, res, pred, 0);
    // CHECK: mov.i8  %V{{[0-9]+}}, 0x7b, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i8_mov_vb
  {
    volatile char256 __local *dfptr = (char256 __local *)dptr;
    volatile char256 __local *sfptr = (char256 __local *)sptr;
    bool256 vpred = v_i8_cmp_grt_b(*sfptr++, 0.0, 0, (bool256){0}, 1, 0);
    // CHECK: cmp_grt.i8 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    char256 res = *sfptr++;
    char256 x = *sfptr++;
    char xscal = as_char(xc);
    
    res = v_i8_mov_vb(x, 0, res, vpred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;
    
    res = v_i8_mov_vb(xscal, 0, res, vpred, 0);
    // CHECK: mov.i8  %V{{[0-9]+}}, %S4, [[VPRED]]
    *dfptr++ = res;
    
    res = v_i8_mov_vb(123, 0, res, vpred, 0);
    // CHECK: mov.i8  %V{{[0-9]+}}, 0x7b, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u8_mov_b
  {
    volatile uchar256 __local *dfptr = (uchar256 __local *)dptr;
    volatile uchar256 __local *sfptr = (uchar256 __local *)sptr;
    uchar256 res = *sfptr++;
    uchar256 x = *sfptr++;
    unsigned char xscal = as_uchar(xc);
    
    res = v_u8_mov_b(x, 0, res, pred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED]]
    *dfptr++ = res;
    
    res = v_u8_mov_b(xscal, 0, res, pred, 0);
    // CHECK: mov.u8  %V{{[0-9]+}}, %S4, [[PRED]]
    *dfptr++ = res;
    
    res = v_u8_mov_b(123, 0, res, pred, 0);
    // CHECK: mov.u8  %V{{[0-9]+}}, 0x7b, [[PRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u8_mov_vb
  {
    volatile uchar256 __local *dfptr = (uchar256 __local *)dptr;
    volatile uchar256 __local *sfptr = (uchar256 __local *)sptr;
    bool256 vpred = v_u8_cmp_grt_b(*sfptr++, 0.0, 0, (bool256){0}, 1, 0);
    // CHECK: cmp_grt.u8 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    uchar256 res = *sfptr++;
    uchar256 x = *sfptr++;
    unsigned char xscal = as_uchar(xc);
    
    res = v_u8_mov_vb(x, 0, res, vpred, 0);
    // CHECK: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res;
    
    res = v_u8_mov_vb(xscal, 0, res, vpred, 0);
    // CHECK: mov.u8  %V{{[0-9]+}}, %S4, [[VPRED]]
    *dfptr++ = res;
    
    res = v_u8_mov_vb(123, 0, res, vpred, 0);
    // CHECK: mov.u8  %V{{[0-9]+}}, 0x7b, [[VPRED]]
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }
}
