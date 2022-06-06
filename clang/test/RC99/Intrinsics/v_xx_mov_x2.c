// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck %s

void main(int dest0, int dest1, int x, int y) {
  _Bool pred = x < y;
  // CHECK-DAG: cmp_less.i32 [[PRED:%SP[0-9]+]], %S2, %S3

  volatile int64 __local *dptr = (int64 __local *)dest0;
  volatile int64 __local *sptr = (int64 __local *)dest1;

  // v_f32_mov_x2_b
  {
    volatile float64 __local *dfptr = (float64 __local *)dptr;
    volatile float64 __local *sfptr = (float64 __local *)sptr;
    float128 res = {*sfptr++, 0};
    float128 x = {*sfptr++, 0};
    
    res = v_f32_mov_x2_b(x, 0, res, pred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[PRED]]
    *dfptr++ = res.v1;
    *dfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_f32_mov_x2_vb
  {
    volatile float64 __local *dfptr = (float64 __local *)dptr;
    volatile float64 __local *sfptr = (float64 __local *)sptr;
    bool64 vpred = v_f32_cmp_grt_b(*sfptr++, 0.0, 0, (bool64){0}, 1, 0);
    // CHECK: cmp_grt.f32 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    float128 res = {*sfptr++, 0};
    float128 x = {*sfptr++, 0};
    
    res = v_f32_mov_x2_vb(x, 0, res, vpred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res.v1;
    *sfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_bf16_mov_x2_b
  {
    volatile bfloat128 __local *dfptr = (bfloat128 __local *)dptr;
    volatile bfloat128 __local *sfptr = (bfloat128 __local *)sptr;
    bfloat256 res = {*sfptr++, 0};
    bfloat256 x = {*sfptr++, 0};
    
    res = v_bf16_mov_x2_b(x, 0, res, pred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[PRED]]
    *dfptr++ = res.v1;
    *dfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_bf16_mov_x2_vb
  {
    volatile bfloat128 __local *dfptr = (bfloat128 __local *)dptr;
    volatile bfloat128 __local *sfptr = (bfloat128 __local *)sptr;
    bool128 vpred = v_bf16_cmp_grt_b(*sfptr++, 0.0, 0, (bool128){0}, 1, 0);
    // CHECK: cmp_grt.bf16 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    bfloat256 res = {*sfptr++, 0};
    bfloat256 x = {*sfptr++, 0};
    
    res = v_bf16_mov_x2_vb(x, 0, res, vpred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res.v1;
    *sfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_f16_mov_x2_b
  {
    volatile half128 __local *dfptr = (half128 __local *)dptr;
    volatile half128 __local *sfptr = (half128 __local *)sptr;
    half256 res = {*sfptr++, 0};
    half256 x = {*sfptr++, 0};
    sfptr += 2;
    
    res = v_f16_mov_x2_b(x, 0, res, pred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[PRED]]
    *dfptr++ = res.v1;
    *dfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_f16_mov_x2_vb
  {
    volatile half128 __local *dfptr = (half128 __local *)dptr;
    volatile half128 __local *sfptr = (half128 __local *)sptr;
    bool128 vpred = v_f16_cmp_grt_b(*sfptr++, 0.0, 0, (bool128){0}, 1, 0);
    // C HECK: cmp_grt.f16 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    half256 res = {*sfptr++, 0};
    half256 x = {*sfptr++, 0};
    sfptr += 2;
    
    res = v_f16_mov_x2_vb(x, 0, res, vpred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res.v1;
    *sfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i32_mov_x2_b
  {
    volatile int64 __local *dfptr = (int64 __local *)dptr;
    volatile int64 __local *sfptr = (int64 __local *)sptr;
    int128 res = {*sfptr++, 0};
    int128 x = {*sfptr++, 0};
    
    res = v_i32_mov_x2_b(x, 0, res, pred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[PRED]]
    *dfptr++ = res.v1;
    *dfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i32_mov_x2_vb
  {
    volatile int64 __local *dfptr = (int64 __local *)dptr;
    volatile int64 __local *sfptr = (int64 __local *)sptr;
    bool64 vpred = v_i32_cmp_grt_b(*sfptr++, 0.0, 0, (bool64){0}, 1, 0);
    // CHECK: cmp_grt.i32 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    int128 res = {*sfptr++, 0};
    int128 x = {*sfptr++, 0};
    
    res = v_i32_mov_x2_vb(x, 0, res, vpred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res.v1;
    *sfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u32_mov_x2_b
  {
    volatile uint64 __local *dfptr = (uint64 __local *)dptr;
    volatile uint64 __local *sfptr = (uint64 __local *)sptr;
    uint128 res = {*sfptr++, 0};
    uint128 x = {*sfptr++, 0};
    
    res = v_u32_mov_x2_b(x, 0, res, pred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[PRED]]
    *dfptr++ = res.v1;
    *dfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u32_mov_x2_vb
  {
    volatile uint64 __local *dfptr = (uint64 __local *)dptr;
    volatile uint64 __local *sfptr = (uint64 __local *)sptr;
    bool64 vpred = v_u32_cmp_grt_b(*sfptr++, 0.0, 0, (bool64){0}, 1, 0);
    // CHECK: cmp_grt.u32 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    uint128 res = {*sfptr++, 0};
    uint128 x = {*sfptr++, 0};
    
    res = v_u32_mov_x2_vb(x, 0, res, vpred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res.v1;
    *sfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i16_mov_x2_b
  {
    volatile short128 __local *dfptr = (short128 __local *)dptr;
    volatile short128 __local *sfptr = (short128 __local *)sptr;
    short256 res = {*sfptr++, 0};
    short256 x = {*sfptr++, 0};
    
    res = v_i16_mov_x2_b(x, 0, res, pred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[PRED]]
    *dfptr++ = res.v1;
    *dfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i16_mov_x2_vb
  {
    volatile short128 __local *dfptr = (short128 __local *)dptr;
    volatile short128 __local *sfptr = (short128 __local *)sptr;
    bool128 vpred = v_i16_cmp_grt_b(*sfptr++, 0.0, 0, (bool128){0}, 1, 0);
    // CHECK: cmp_grt.i16 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    short256 res = {*sfptr++, 0};
    short256 x = {*sfptr++, 0};
    
    res = v_i16_mov_x2_vb(x, 0, res, vpred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res.v1;
    *sfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u16_mov_x2_b
  {
    volatile ushort128 __local *dfptr = (ushort128 __local *)dptr;
    volatile ushort128 __local *sfptr = (ushort128 __local *)sptr;
    ushort256 res = {*sfptr++, 0};
    ushort256 x = {*sfptr++, 0};
    
    res = v_u16_mov_x2_b(x, 0, res, pred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[PRED]]
    *dfptr++ = res.v1;
    *dfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u16_mov_x2_vb
  {
    volatile ushort128 __local *dfptr = (ushort128 __local *)dptr;
    volatile ushort128 __local *sfptr = (ushort128 __local *)sptr;
    bool128 vpred = v_u16_cmp_grt_b(*sfptr++, 0.0, 0, (bool128){0}, 1, 0);
    // CHECK: cmp_grt.u16 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    ushort256 res = {*sfptr++, 0};
    ushort256 x = {*sfptr++, 0};
    
    res = v_u16_mov_x2_vb(x, 0, res, vpred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res.v1;
    *sfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i8_mov_x2_b
  {
    volatile char256 __local *dfptr = (char256 __local *)dptr;
    volatile char256 __local *sfptr = (char256 __local *)sptr;
    char512 res = {*sfptr++, 0};
    char512 x = {*sfptr++, 0};
    
    res = v_i8_mov_x2_b(x, 0, res, pred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[PRED]]
    *dfptr++ = res.v1;
    *dfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_i8_mov_x2_vb
  {
    volatile char256 __local *dfptr = (char256 __local *)dptr;
    volatile char256 __local *sfptr = (char256 __local *)sptr;
    bool256 vpred = v_i8_cmp_grt_b(*sfptr++, 0.0, 0, (bool256){0}, 1, 0);
    // CHECK: cmp_grt.i8 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    char512 res = {*sfptr++, 0};
    char512 x = {*sfptr++, 0};
    
    res = v_i8_mov_x2_vb(x, 0, res, vpred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res.v1;
    *sfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u8_mov_x2_b
  {
    volatile uchar256 __local *dfptr = (uchar256 __local *)dptr;
    volatile uchar256 __local *sfptr = (uchar256 __local *)sptr;
    uchar512 res = {*sfptr++, 0};
    uchar512 x = {*sfptr++, 0};
    
    res = v_u8_mov_x2_b(x, 0, res, pred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[PRED]]
    *dfptr++ = res.v1;
    *dfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_u8_mov_x2_vb
  {
    volatile uchar256 __local *dfptr = (uchar256 __local *)dptr;
    volatile uchar256 __local *sfptr = (uchar256 __local *)sptr;
    bool256 vpred = v_u8_cmp_grt_b(*sfptr++, 0.0, 0, (bool256){0}, 1, 0);
    // CHECK: cmp_grt.u8 [[VPRED:%VP[0-9]+]], %V{{[0-9]+}}, 0x0

    uchar512 res = {*sfptr++, 0};
    uchar512 x = {*sfptr++, 0};
    
    res = v_u8_mov_x2_vb(x, 0, res, vpred, 0);
    // CHECK: mov x2 %D{{[0-9]+}}, %D{{[0-9]+}}, [[VPRED]]
    *dfptr++ = res.v1;
    *sfptr++ = res.v2;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }
}
