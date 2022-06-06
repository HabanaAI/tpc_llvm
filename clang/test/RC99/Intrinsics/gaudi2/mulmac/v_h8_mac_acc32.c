// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1  %s -o - | FileCheck %s

void main(int x0a, int x1a, minihalf xs, int dest, _Bool pred, int vpreda) {
  minihalf256 __local *x0ptr = (minihalf256 __local *)x0a;
  minihalf256 __local *x1ptr = (minihalf256 __local *)x1a;
  float256 __local *dptr  = (float256 __local *)dest;
  bool256 __local *vpptr  = (bool256 __local *)vpreda;
  float256 res = { 0 };
  minihalf256 x0 = *x0ptr;
  minihalf256 x1 = *x1ptr;
  bool256 vpred = *vpptr;

  // Vector + Vector

  res = v_h8_mac_acc32_b(x0, x1, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_h8_mac_acc32_b(x0, x1, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f8_152 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_h8_mac_acc32_b(x0, x1, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_h8_mac_acc32_b(x0, x1, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_h8_mac_acc32_vb(x0, x1, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_h8_mac_acc32_vb(x0, x1, res, SW_NEG, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_152 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_h8_mac_acc32_vb(x0, x1, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

  res = v_h8_mac_acc32_b(x0, xs, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_h8_mac_acc32_b(x0, xs, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f8_152 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_h8_mac_acc32_b(x0, xs, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP{{[0-9]+}}

  res = v_h8_mac_acc32_b(x0, xs, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%SP{{[0-9]+}}

  res = v_h8_mac_acc32_vb(x0, xs, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_h8_mac_acc32_vb(x0, xs, res, SW_NEG, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_152 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_h8_mac_acc32_vb(x0, xs, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%VP{{[0-9]+}}


  // Vector + Immediate

  res = v_h8_mac_acc32_b(x0, (minihalf)1.5, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e

  res = v_h8_mac_acc32_b(x0, (minihalf)1.5, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f8_152 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e

  res = v_h8_mac_acc32_b(x0, (minihalf)1.5, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e, %SP{{[0-9]+}}

  res = v_h8_mac_acc32_b(x0, (minihalf)1.5, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e, !%SP{{[0-9]+}}

  res = v_h8_mac_acc32_vb(x0, (minihalf)1.5, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e, %VP{{[0-9]+}}

  res = v_h8_mac_acc32_vb(x0, (minihalf)1.5, res, SW_NEG, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_152 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e, %VP{{[0-9]+}}

  res = v_h8_mac_acc32_vb(x0, (minihalf)1.5, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e, !%VP{{[0-9]+}}


}
