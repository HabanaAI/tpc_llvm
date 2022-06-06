// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(half a, half b, int dest, int src) {
  float128 __local *dptr = (float128 __local *)dest;
  half128 x0 = *(half128 __local *)(src + 0 * 256);
  half128 x1 = *(half128 __local *)(src + 1 * 256);
  bool128 pred = *(bool128 __local *)(src + 2 * 256);
  float128 res = { 0 };

  res = v_f16_mul_acc32_vb(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_f16_mul_acc32_vb(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}

  res = v_f16_mul_acc32_vb(x0, a, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0, %VP{{[0-9]+}}

  res = v_f16_mul_acc32_vb(x0, a, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0, !%VP{{[0-9]+}}

  res = v_f16_mul_acc32_vb(x0, 1.5, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %VP{{[0-9]+}}

  res = v_f16_mul_acc32_vb(x0, 1.5, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, !%VP{{[0-9]+}}
}
