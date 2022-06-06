// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, int x2a, int x3a, unsigned char xs, int dest, _Bool pred, int vpreda) {
  uchar256 __local *x0ptr = (uchar256 __local *)x0a;
  uchar256 __local *x1ptr = (uchar256 __local *)x1a;
  uchar256 __local *x2ptr = (uchar256 __local *)x2a;
  uchar256 __local *x3ptr = (uchar256 __local *)x3a;
  uint256  __local *dptr  = (uint256  __local *)dest;
  bool256 __local *vpptr  = (bool256 __local *)vpreda;
  uint256 res = { 0 };
  uchar256 x0 = *x0ptr;
  uchar256 x1 = *x1ptr;
  uchar256 x2 = *x2ptr;
  uchar256 x3 = *x3ptr;
  bool256 vpred = *vpptr;

  // Vector + Vector

  res = v_u8_mac_x2_b(x0, x1, x2, x3, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, x1, x2, x3, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, x1, x2, x3, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, x1, x2, x3, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u8_mac_x2_vb(x0, x1, x2, x3, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_mac_x2_vb(x0, x1, x2, x3, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_mac_x2_vb(x0, x1, x2, x3, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

  res = v_u8_mac_x2_b(x0, xs, x2, x3, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, xs, x2, x3, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, xs, x2, x3, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, xs, x2, x3, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u8_mac_x2_vb(x0, xs, x2, x3, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_mac_x2_vb(x0, xs, x2, x3, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_mac_x2_vb(x0, xs, x2, x3, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Immediate

  res = v_u8_mac_x2_b(x0, 123, x2, x3, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, 123, x2, x3, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st x2 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, 123, x2, x3, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, 123, x2, x3, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u8_mac_x2_vb(x0, 123, x2, x3, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_mac_x2_vb(x0, 123, x2, x3, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st x2 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_mac_x2_vb(x0, 123, x2, x3, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}


  // Compatibility functions

  // Vector + Vector

  res = v_u8_mac_x2_b(x0, x1, x2, x3, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, x1, x2, x3, res, 1,  1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, x1, x2, x3, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, x1, x2, x3, res, 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u8_mac_x2_vb(x0, x1, x2, x3, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_mac_x2_vb(x0, x1, x2, x3, res, 1, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}

  // Vector + Scalar

  res = v_u8_mac_x2_b(x0, xs, x2, x3, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, xs, x2, x3, res, 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, xs, x2, x3, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, xs, x2, x3, res, 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u8_mac_x2_vb(x0, xs, x2, x3, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_mac_x2_vb(x0, xs, x2, x3, res, 1, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st x2 %A{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}

  // Vector + Immediate

  res = v_u8_mac_x2_b(x0, 123, x2, x3, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, 123, x2, x3, res, 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st x2 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, 123, x2, x3, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u8_mac_x2_b(x0, 123, x2, x3, res, 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st x2 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u8_mac_x2_vb(x0, 123, x2, x3, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 x2 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_mac_x2_vb(x0, 123, x2, x3, res, 1, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st x2 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}
}
