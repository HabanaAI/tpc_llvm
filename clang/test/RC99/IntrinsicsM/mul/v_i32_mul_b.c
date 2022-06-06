// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int a, int b, int dest, int src, _Bool pred) {
  int128 __local *dptr = (int128 __local *)dest;
  int64 x0 = *(int64 __local *)(src + 0 * 256);
  int64 x1 = *(int64 __local *)(src + 1 * 256);
  int128 res = { 0 };

  res = v_i32_mul_b(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_i32_mul_b(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_i32_mul_b(x0, x1, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.i32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_i32_mul_b(x0, a, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0, %SP{{[0-9]+}}

  res = v_i32_mul_b(x0, a, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0, !%SP{{[0-9]+}}

  res = v_i32_mul_b(x0, a, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.i32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0

  res = v_i32_mul_b(x0, 123, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP{{[0-9]+}}

  res = v_i32_mul_b(x0, 123, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%SP{{[0-9]+}}

  res = v_i32_mul_b(x0, 123, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.i32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b
}
