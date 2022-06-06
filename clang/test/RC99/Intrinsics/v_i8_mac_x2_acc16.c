// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, int x2a, int x3a, unsigned char xs, int dest, _Bool pred, int vpreda) {
  char256 __local *x0ptr = (char256 __local *)x0a;
  char256 __local *x1ptr = (char256 __local *)x1a;
  char256 __local *x2ptr = (char256 __local *)x2a;
  char256 __local *x3ptr = (char256 __local *)x3a;
  short256  __local *dptr  = (short256  __local *)dest;
  bool256 __local *vpptr  = (bool256 __local *)vpreda;
  short256 res = { 0 };
  char256 x0 = *x0ptr;
  char256 x1 = *x1ptr;
  char256 x2 = *x2ptr;
  char256 x3 = *x3ptr;
  bool256 vpred = *vpptr;

  // Vector + Vector

  res = v_i8_mac_x2_acc16_b(x0, x1, x2, x3, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, x1, x2, x3, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, x1, x2, x3, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, x1, x2, x3, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_vb(x0, x1, x2, x3, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_vb(x0, x1, x2, x3, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_vb(x0, x1, x2, x3, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

  res = v_i8_mac_x2_acc16_b(x0, xs, x2, x3, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, xs, x2, x3, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, xs, x2, x3, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, xs, x2, x3, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_vb(x0, xs, x2, x3, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_vb(x0, xs, x2, x3, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_vb(x0, xs, x2, x3, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Immediate

  res = v_i8_mac_x2_acc16_b(x0, 123, x2, x3, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, 123, x2, x3, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, 123, x2, x3, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, 123, x2, x3, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_vb(x0, 123, x2, x3, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_vb(x0, 123, x2, x3, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_vb(x0, 123, x2, x3, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}


  // Compatibility functions

  // Vector + Vector

  res = v_i8_mac_x2_acc16_b(x0, x1, x2, x3, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, x1, x2, x3, res, 1,  1, 0);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, x1, x2, x3, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, x1, x2, x3, res, 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_vb(x0, x1, x2, x3, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_vb(x0, x1, x2, x3, res, 1, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}

  // Vector + Scalar

  res = v_i8_mac_x2_acc16_b(x0, xs, x2, x3, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, xs, x2, x3, res, 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, xs, x2, x3, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, xs, x2, x3, res, 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_vb(x0, xs, x2, x3, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_vb(x0, xs, x2, x3, res, 1, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}

  // Vector + Immediate

  res = v_i8_mac_x2_acc16_b(x0, 123, x2, x3, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, 123, x2, x3, res, 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, 123, x2, x3, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_b(x0, 123, x2, x3, res, 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_vb(x0, 123, x2, x3, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_i8_mac_x2_acc16_vb(x0, 123, x2, x3, res, 1, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 x2 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}
}
