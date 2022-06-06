// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(unsigned char a, unsigned char b, int dest, int src) {
  uint256 __local *dptr = (uint256 __local *)dest;
  uchar256 x0 = *(uchar256 __local *)(src + 0 * 256);
  uchar256 x1 = *(uchar256 __local *)(src + 1 * 256);
  bool256 pred = *(bool256 __local *)(src + 2 * 256);
  uint256 res = { 0 };

  res = v_u8_mul_vb(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_mul_vb(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}

  res = v_u8_mul_vb(x0, a, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %S0, %VP{{[0-9]+}}

  res = v_u8_mul_vb(x0, a, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %S0, !%VP{{[0-9]+}}

  res = v_u8_mul_vb(x0, 123, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %VP{{[0-9]+}}

  res = v_u8_mul_vb(x0, 123, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%VP{{[0-9]+}}
}
