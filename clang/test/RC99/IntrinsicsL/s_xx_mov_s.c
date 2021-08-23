// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s


void main(int dest, int xi, short xs, char xc) {
  _Bool pred = xi < dest;
  // CHECK-DAG: cmp_less.i32 [[PRED:%SP[0-9]+]], %S1, %S0

  volatile int __local *dptr = (int __local *)dest;

  // s_f32_mov_s_b
  {
    float res = as_float(*dptr++);
    float x = as_float(*dptr++);

    res = s_f32_mov_s_b(x, res, pred, 0);
    // CHECK: mov.f32  %S{{[0-9]+}}, %S{{[0-9]+}}, [[PRED]]
    *dptr++ = res;
  }


  // s_i32_mov_s_b
  {
    int res = as_int(*dptr++);
    int x = as_int(*dptr++);

    res = s_i32_mov_s_b(x, res, pred, 0);
    // CHECK: mov.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, [[PRED]]
    *dptr++ = res;
  }

  // s_u32_mov_s_b
  {
    unsigned res = as_uint(*dptr++);
    unsigned x = as_uint(*dptr++);

    res = s_u32_mov_s_b(x, res, pred, 0);
    // CHECK: mov.u32  %S{{[0-9]+}}, %S{{[0-9]+}}, [[PRED]]
    *dptr++ = res;
  }

  // s_i16_mov_s_b
  {
    short res = *dptr++;
    short x = *dptr++;

    res = s_i16_mov_s_b(x, res, pred, 0);
    // CHECK: mov.i16  %S{{[0-9]+}}, %S{{[0-9]+}}, [[PRED]]
    *dptr++ = res;
  }

  // s_u16_mov_s_b
  {
    unsigned short res = *dptr++;
    unsigned short x = *dptr++;

    res = s_u16_mov_s_b(x, res, pred, 0);
    // CHECK: mov.u16  %S{{[0-9]+}}, %S{{[0-9]+}}, [[PRED]]
    *dptr++ = res;
  }

  // s_i8_mov_s_b
  {
    char res = *dptr++;
    char x = *dptr++;

    res = s_i8_mov_s_b(x, res, pred, 0);
    // CHECK: mov.i8  %S{{[0-9]+}}, %S{{[0-9]+}}, [[PRED]]
    *dptr++ = res;
  }

  // s_u8_mov_s_b
  {
    unsigned char res = *dptr++;
    unsigned char x = *dptr++;

    res = s_u8_mov_s_b(x, res, pred, 0);
    // CHECK: mov.u8  %S{{[0-9]+}}, %S{{[0-9]+}}, [[PRED]]
    *dptr++ = res;
  }

  // s_f32_mov_s
  {
    float x = *dptr++;
    float res = s_f32_mov_s(x);
    *dptr++ = res;
  }

#if defined(__gaudi__)
  // s_bf16_mov_s
  {
    _BFloat16 x = *dptr++;
    _BFloat16 res = s_bf16_mov_s(x);
    *dptr++ = res;
  }
#endif  

  // s_i32_mov_s
  {
    int x = *dptr++;
    int res = s_i32_mov_s(x);
    *dptr++ = res;
  }

  // s_u32_mov_s
  {
    unsigned x = *dptr++;
    unsigned res = s_u32_mov_s(x);
    *dptr++ = res;
  }

  // s_i16_mov_s
  {
    short x = *dptr++;
    short res = s_i16_mov_s(x);
    *dptr++ = res;
  }

  // s_u16_mov_s
  {
    unsigned short x = *dptr++;
    unsigned short res = s_u16_mov_s(x);
    *dptr++ = res;
  }

  // s_i8_mov_s
  {
    char x = *dptr++;
    char res = s_i8_mov_s(x);
    *dptr++ = res;
  }

  // s_u8_mov_s
  {
    unsigned char x = *dptr++;
    unsigned char res = s_u8_mov_s(x);
    *dptr++ = res;
  }
}

