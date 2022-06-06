// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(char a, char b, int dest, int src, _Bool pred) {
  int256 __local *dptr = (int256 __local *)dest;
  char256 x0 = *(char256 __local *)(src + 0 * 256);
  char256 x1 = *(char256 __local *)(src + 1 * 256);
  int256 res = { 0 };

  res = v_i8_mul_b(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i8 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_i8_mul_b(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i8 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_i8_mul_b(x0, x1, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.i8 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_i8_mul_b(x0, a, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i8 %A{{[0-9]+}}, %V{{[0-9]+}}, %S0, %SP{{[0-9]+}}

  res = v_i8_mul_b(x0, a, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i8 %A{{[0-9]+}}, %V{{[0-9]+}}, %S0, !%SP{{[0-9]+}}

  res = v_i8_mul_b(x0, a, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.i8 %A{{[0-9]+}}, %V{{[0-9]+}}, %S0

  res = v_i8_mul_b(x0, 123, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i8 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP{{[0-9]+}}

  res = v_i8_mul_b(x0, 123, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i8 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%SP{{[0-9]+}}

  res = v_i8_mul_b(x0, 123, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.i8 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b
}
