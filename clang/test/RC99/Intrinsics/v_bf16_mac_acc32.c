// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, _BFloat16 xs, int dest, _Bool pred, int vpreda) {
  bfloat128 __local *x0ptr = (bfloat128 __local *)x0a;
  bfloat128 __local *x1ptr = (bfloat128 __local *)x1a;
  float128 __local *dptr  = (float128 __local *)dest;
  bool128 __local *vpptr  = (bool128 __local *)vpreda;
  float128 res = { 0 };
  bfloat128 x0 = *x0ptr;
  bfloat128 x1 = *x1ptr;
  bool128 vpred = *vpptr;

  // Vector + Vector

  res = v_bf16_mac_acc32_b(x0, x1, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_bf16_mac_acc32_b(x0, x1, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_bf16_mac_acc32_b(x0, x1, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_bf16_mac_acc32_b(x0, x1, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_bf16_mac_acc32_vb(x0, x1, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_bf16_mac_acc32_vb(x0, x1, res, SW_NEG, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_bf16_mac_acc32_vb(x0, x1, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

  res = v_bf16_mac_acc32_b(x0, xs, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_bf16_mac_acc32_b(x0, xs, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_bf16_mac_acc32_b(x0, xs, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP{{[0-9]+}}

  res = v_bf16_mac_acc32_b(x0, xs, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%SP{{[0-9]+}}

  res = v_bf16_mac_acc32_vb(x0, xs, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_bf16_mac_acc32_vb(x0, xs, res, SW_NEG, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_bf16_mac_acc32_vb(x0, xs, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%VP{{[0-9]+}}


  // Vector + Immediate

  res = v_bf16_mac_acc32_b(x0, 1.5, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0

  res = v_bf16_mac_acc32_b(x0, 1.5, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0

  res = v_bf16_mac_acc32_b(x0, 1.5, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %SP{{[0-9]+}}

  res = v_bf16_mac_acc32_b(x0, 1.5, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, !%SP{{[0-9]+}}

  res = v_bf16_mac_acc32_vb(x0, 1.5, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %VP{{[0-9]+}}

  res = v_bf16_mac_acc32_vb(x0, 1.5, res, SW_NEG, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %VP{{[0-9]+}}

  res = v_bf16_mac_acc32_vb(x0, 1.5, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, !%VP{{[0-9]+}}


  // Compatibility functions
  bool256 vpred_c = from_bool128(vpred);

  // Vector + Vector

  res = v_bf16_mac_acc32_b(x0, x1, res, (0) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_bf16_mac_acc32_b(x0, x1, res, (1) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_bf16_mac_acc32_b(x0, x1, res, (0) << 1, pred, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_bf16_mac_acc32_b(x0, x1, res, (1) << 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_bf16_mac_acc32_vb(x0, x1, res, (0) << 1, to_bool128(vpred_c), 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_bf16_mac_acc32_vb(x0, x1, res, (1) << 1, to_bool128(vpred_c), 1);
  *dptr++ = res;
  // CHECK: mac.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}

  // Vector + Scalar

  res = v_bf16_mac_acc32_b(x0, xs, res, (0) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_bf16_mac_acc32_b(x0, xs, res, (1) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_bf16_mac_acc32_b(x0, xs, res, (0) << 1, pred, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP{{[0-9]+}}

  res = v_bf16_mac_acc32_b(x0, xs, res, (1) << 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%SP{{[0-9]+}}

  res = v_bf16_mac_acc32_vb(x0, xs, res, (0) << 1, to_bool128(vpred_c), 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_bf16_mac_acc32_vb(x0, xs, res, (1) << 1, to_bool128(vpred_c), 1);
  *dptr++ = res;
  // CHECK: mac.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%VP{{[0-9]+}}

  // Vector + Immediate

  res = v_bf16_mac_acc32_b(x0, 1.5, res, (0) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0

  res = v_bf16_mac_acc32_b(x0, 1.5, res, (1) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0

  res = v_bf16_mac_acc32_b(x0, 1.5, res, (0) << 1, pred, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %SP{{[0-9]+}}

  res = v_bf16_mac_acc32_b(x0, 1.5, res, (1) << 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, !%SP{{[0-9]+}}

  res = v_bf16_mac_acc32_vb(x0, 1.5, res, (0) << 1, to_bool128(vpred_c), 0);
  *dptr++ = res;
  // CHECK: mac.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %VP{{[0-9]+}}

  res = v_bf16_mac_acc32_vb(x0, 1.5, res, (1) << 1, to_bool128(vpred_c), 1);
  *dptr++ = res;
  // CHECK: mac.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, !%VP{{[0-9]+}}
}
