// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s


void main(short x0, short x1, int dest, _Bool pred) {
  int __local *dptr = (int  __local *)dest;
  int res = 0;

  res = s_i16_mul(x0, x1, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.i16 %S{{[0-9]+}}, %S0, %S1, %SP0

  res = s_i16_mul(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i16 %S{{[0-9]+}}, %S0, %S1, %SP{{[0-9]+}}

  res = s_i16_mul(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i16 %S{{[0-9]+}}, %S0, %S1, !%SP{{[0-9]+}}

  res = s_i16_mul(x0, 123, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.i16 %S{{[0-9]+}}, %S0,  0x7b, %SP0

  res = s_i16_mul(x0, 123, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i16 %S{{[0-9]+}}, %S0,  0x7b, %SP{{[0-9]+}}

  res = s_i16_mul(x0, 123, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i16 %S{{[0-9]+}}, %S0,  0x7b, !%SP{{[0-9]+}}
}
