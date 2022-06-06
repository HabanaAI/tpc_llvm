// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(unsigned short a, unsigned short b, int dest, int src, _Bool pred) {
  uint128 __local *dptr = (uint128 __local *)dest;
  ushort128 x0 = *(ushort128 __local *)(src + 0 * 256);
  ushort128 x1 = *(ushort128 __local *)(src + 1 * 256);
  uint128 res = { 0 };

  res = v_u16_mul_b(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u16_mul_b(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u16_mul_b(x0, x1, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u16_mul_b(x0, a, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0, %SP{{[0-9]+}}

  res = v_u16_mul_b(x0, a, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0, !%SP{{[0-9]+}}

  res = v_u16_mul_b(x0, a, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0

  res = v_u16_mul_b(x0, 123, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP{{[0-9]+}}

  res = v_u16_mul_b(x0, 123, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%SP{{[0-9]+}}

  res = v_u16_mul_b(x0, 123, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b
}
