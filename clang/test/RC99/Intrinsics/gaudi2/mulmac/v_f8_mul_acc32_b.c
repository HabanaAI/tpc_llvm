// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1  %s -o - | FileCheck %s

void main(minifloat a, minifloat b, int dest, int src, _Bool pred) {
  float256 __local *dptr = (float256 __local *)dest;
  minifloat256 x0 = *(minifloat256 __local *)(src + 0 * 256);
  minifloat256 x1 = *(minifloat256 __local *)(src + 1 * 256);
  float256 res = { 0 };

  res = v_f8_mul_acc32_b(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_f8_mul_acc32_b(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_f8_mul_acc32_b(x0, x1, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_f8_mul_acc32_b(x0, a, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S0, %SP{{[0-9]+}}

  res = v_f8_mul_acc32_b(x0, a, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S0, !%SP{{[0-9]+}}

  res = v_f8_mul_acc32_b(x0, a, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S0

  res = v_f8_mul_acc32_b(x0, (minifloat)1.5, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3c, %SP{{[0-9]+}}

  res = v_f8_mul_acc32_b(x0, (minifloat)1.5, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3c, !%SP{{[0-9]+}}

  res = v_f8_mul_acc32_b(x0, (minifloat)1.5, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.f8_143 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3c
}
