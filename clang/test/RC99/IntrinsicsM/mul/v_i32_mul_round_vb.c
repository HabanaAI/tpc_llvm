// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s


void main(int a, int b, int dest, int src) {
  int64 __local *dptr = (int64 __local *)dest;
  int64 x0 = *(int64 __local *)(src + 0 * 256);
  int64 x1 = *(int64 __local *)(src + 1 * 256);
  bool64 pred = *(bool64 __local *)(src + 2 * 256);
  int64 res = 0;

  res = v_i32_mul_round_vb(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i32 double_and_round32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_i32_mul_round_vb(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i32 double_and_round32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}

  res = v_i32_mul_round_vb(x0, a, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i32 double_and_round32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S0, %VP{{[0-9]+}}

  res = v_i32_mul_round_vb(x0, a, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i32 double_and_round32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S0, !%VP{{[0-9]+}}

  res = v_i32_mul_round_vb(x0, 123, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.i32 double_and_round32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %VP{{[0-9]+}}

  res = v_i32_mul_round_vb(x0, 123, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.i32 double_and_round32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%VP{{[0-9]+}}
}
