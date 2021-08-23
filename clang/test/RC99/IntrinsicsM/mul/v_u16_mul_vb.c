// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s


void main(unsigned short a, unsigned short b, int dest, int src) {
  uint128 __local *dptr = (uint128 __local *)dest;
  ushort128 x0 = *(ushort128 __local *)(src + 0 * 256);
  ushort128 x1 = *(ushort128 __local *)(src + 1 * 256);
  bool128 pred = *(bool128 __local *)(src + 2 * 256);
  uint128 res = { 0 };

  res = v_u16_mul_vb(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u16_mul_vb(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}

  res = v_u16_mul_vb(x0, a, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0, %VP{{[0-9]+}}

  res = v_u16_mul_vb(x0, a, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0, !%VP{{[0-9]+}}

  res = v_u16_mul_vb(x0, 123, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %VP{{[0-9]+}}

  res = v_u16_mul_vb(x0, 123, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%VP{{[0-9]+}}
}
