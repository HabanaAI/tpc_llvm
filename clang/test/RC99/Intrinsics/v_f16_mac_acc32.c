// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, half xs, int dest, _Bool pred, int vpreda) {
  half128 __local *x0ptr = (half128 __local *)x0a;
  half128 __local *x1ptr = (half128 __local *)x1a;
  float128 __local *dptr  = (float128 __local *)dest;
  bool128 __local *vpptr  = (bool128 __local *)vpreda;
  float128 res = { 0 };
  half128 x0 = *x0ptr;
  half128 x1 = *x1ptr;
  bool128 vpred = *vpptr;

  // Vector + Vector

  res = v_f16_mac_acc32_b(x0, x1, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_f16_mac_acc32_b(x0, x1, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_f16_mac_acc32_b(x0, x1, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_f16_mac_acc32_b(x0, x1, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_f16_mac_acc32_vb(x0, x1, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_f16_mac_acc32_vb(x0, x1, res, SW_NEG, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_f16_mac_acc32_vb(x0, x1, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

  res = v_f16_mac_acc32_b(x0, xs, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_f16_mac_acc32_b(x0, xs, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_f16_mac_acc32_b(x0, xs, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP{{[0-9]+}}

  res = v_f16_mac_acc32_b(x0, xs, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%SP{{[0-9]+}}

  res = v_f16_mac_acc32_vb(x0, xs, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_f16_mac_acc32_vb(x0, xs, res, SW_NEG, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_f16_mac_acc32_vb(x0, xs, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%VP{{[0-9]+}}


  // Vector + Immediate

  res = v_f16_mac_acc32_b(x0, 1.5, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00

  res = v_f16_mac_acc32_b(x0, 1.5, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00

  res = v_f16_mac_acc32_b(x0, 1.5, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %SP{{[0-9]+}}

  res = v_f16_mac_acc32_b(x0, 1.5, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, !%SP{{[0-9]+}}

  res = v_f16_mac_acc32_vb(x0, 1.5, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %VP{{[0-9]+}}

  res = v_f16_mac_acc32_vb(x0, 1.5, res, SW_NEG, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %VP{{[0-9]+}}

  res = v_f16_mac_acc32_vb(x0, 1.5, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, !%VP{{[0-9]+}}
}
