// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s


void main(short a, short b, int dest, int src, _Bool pred) {
  int128 __local *dptr = (int128 __local *)dest;
  short128 x0 = *(short128 __local *)(src + 0 * 256);
  short128 x1 = *(short128 __local *)(src + 1 * 256);
  int128 res = { 0 };

  res = v_i16_mul_b(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_i16_mul_b(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_i16_mul_b(x0, x1, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP0

  res = v_i16_mul_b(x0, a, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0, %SP{{[0-9]+}}

  res = v_i16_mul_b(x0, a, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0, !%SP{{[0-9]+}}

  res = v_i16_mul_b(x0, a, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0, %SP0

  res = v_i16_mul_b(x0, 123, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP{{[0-9]+}}

  res = v_i16_mul_b(x0, 123, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%SP{{[0-9]+}}

  res = v_i16_mul_b(x0, 123, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP0
}
