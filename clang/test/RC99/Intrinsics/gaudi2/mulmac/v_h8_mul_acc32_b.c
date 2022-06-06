// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1  %s -o - | FileCheck %s

void main(minihalf a, minifloat b, int dest, int src, _Bool pred) {
  float256 __local *dptr = (float256 __local *)dest;
  minihalf256 x0 = *(minihalf256 __local *)(src + 0 * 256);
  minihalf256 x1 = *(minihalf256 __local *)(src + 1 * 256);
  float256 res = { 0 };

  res = v_h8_mul_acc32_b(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_h8_mul_acc32_b(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_h8_mul_acc32_b(x0, x1, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_h8_mul_acc32_b(x0, a, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S0, %SP{{[0-9]+}}

  res = v_h8_mul_acc32_b(x0, a, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S0, !%SP{{[0-9]+}}

  res = v_h8_mul_acc32_b(x0, a, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S0

  res = v_h8_mul_acc32_b(x0, (minihalf)1.5, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e, %SP{{[0-9]+}}

  res = v_h8_mul_acc32_b(x0, (minihalf)1.5, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e, !%SP{{[0-9]+}}

  res = v_h8_mul_acc32_b(x0, (minihalf)1.5, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e
}
