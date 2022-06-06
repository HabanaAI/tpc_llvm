// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(_BFloat16 x0, _BFloat16 x1, int dest, _Bool pred) {
  float __local *dptr = (float  __local *)dest;
  float res = 0;

  res = s_bf16_mul_acc32(x0, x1, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.bf16 acc_fp32 %S{{[0-9]+}}, %S0, %S1

  res = s_bf16_mul_acc32(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.bf16 acc_fp32 %S{{[0-9]+}}, %S0, %S1, %SP{{[0-9]+}}

  res = s_bf16_mul_acc32(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.bf16 acc_fp32 %S{{[0-9]+}}, %S0, %S1, !%SP{{[0-9]+}}

  res = s_bf16_mul_acc32(x0, 1.5, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.bf16 acc_fp32 %S{{[0-9]+}}, %S0, 0x3fc0

  res = s_bf16_mul_acc32(x0, 1.5, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.bf16 acc_fp32 %S{{[0-9]+}}, %S0, 0x3fc0, %SP{{[0-9]+}}

  res = s_bf16_mul_acc32(x0, 1.5, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.bf16 acc_fp32 %S{{[0-9]+}}, %S0, 0x3fc0, !%SP{{[0-9]+}}
}
