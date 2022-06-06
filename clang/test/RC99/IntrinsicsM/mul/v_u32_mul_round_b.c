// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(unsigned int a, unsigned int b, int dest, int src, _Bool pred) {
  uint64 __local *dptr = (uint64 __local *)dest;
  uint64 x0 = *(uint64 __local *)(src + 0 * 256);
  uint64 x1 = *(uint64 __local *)(src + 1 * 256);
  uint64 res = { 0 };

  res = v_u32_mul_round_b(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.u32 double_and_round32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u32_mul_round_b(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.u32 double_and_round32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u32_mul_round_b(x0, x1, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.u32 double_and_round32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u32_mul_round_b(x0, a, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.u32 double_and_round32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S0, %SP{{[0-9]+}}

  res = v_u32_mul_round_b(x0, a, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.u32 double_and_round32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S0, !%SP{{[0-9]+}}

  res = v_u32_mul_round_b(x0, a, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.u32 double_and_round32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S0

  res = v_u32_mul_round_b(x0, 123, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.u32 double_and_round32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP{{[0-9]+}}

  res = v_u32_mul_round_b(x0, 123, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.u32 double_and_round32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%SP{{[0-9]+}}

  res = v_u32_mul_round_b(x0, 123, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.u32 double_and_round32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x7b
}
