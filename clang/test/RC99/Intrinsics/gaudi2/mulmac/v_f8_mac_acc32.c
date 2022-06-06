// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1  %s -o - | FileCheck %s

void main(int x0a, int x1a, minifloat xs, int dest, _Bool pred, int vpreda) {
  minifloat256 __local *x0ptr = (minifloat256 __local *)x0a;
  minifloat256 __local *x1ptr = (minifloat256 __local *)x1a;
  float256 __local *dptr  = (float256 __local *)dest;
  bool256 __local *vpptr  = (bool256 __local *)vpreda;
  float256 res = { 0 };
  minifloat256 x0 = *x0ptr;
  minifloat256 x1 = *x1ptr;
  bool256 vpred = *vpptr;

  // Vector + Vector

  res = v_f8_mac_acc32_b(x0, x1, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_f8_mac_acc32_b(x0, x1, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_f8_mac_acc32_b(x0, x1, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_f8_mac_acc32_b(x0, x1, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_f8_mac_acc32_vb(x0, x1, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_f8_mac_acc32_vb(x0, x1, res, SW_NEG, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_f8_mac_acc32_vb(x0, x1, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

  res = v_f8_mac_acc32_b(x0, xs, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_f8_mac_acc32_b(x0, xs, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_f8_mac_acc32_b(x0, xs, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP{{[0-9]+}}

  res = v_f8_mac_acc32_b(x0, xs, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%SP{{[0-9]+}}

  res = v_f8_mac_acc32_vb(x0, xs, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_f8_mac_acc32_vb(x0, xs, res, SW_NEG, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_f8_mac_acc32_vb(x0, xs, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%VP{{[0-9]+}}


  // Vector + Immediate

  res = v_f8_mac_acc32_b(x0, (minifloat)1.5, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3c

  res = v_f8_mac_acc32_b(x0, (minifloat)1.5, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3c

  res = v_f8_mac_acc32_b(x0, (minifloat)1.5, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3c, %SP{{[0-9]+}}

  res = v_f8_mac_acc32_b(x0, (minifloat)1.5, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3c, !%SP{{[0-9]+}}

  res = v_f8_mac_acc32_vb(x0, (minifloat)1.5, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3c, %VP{{[0-9]+}}

  res = v_f8_mac_acc32_vb(x0, (minifloat)1.5, res, SW_NEG, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3c, %VP{{[0-9]+}}

  res = v_f8_mac_acc32_vb(x0, (minifloat)1.5, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3c, !%VP{{[0-9]+}}


}
