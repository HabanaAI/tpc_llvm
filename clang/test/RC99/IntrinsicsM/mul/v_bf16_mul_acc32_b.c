// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(_BFloat16 a, _BFloat16 b, int dest, int src, _Bool pred) {
  float128 __local *dptr = (float128 __local *)dest;
  bfloat128 x0 = *(bfloat128 __local *)(src + 0 * 256);
  bfloat128 x1 = *(bfloat128 __local *)(src + 1 * 256);
  float128 res = { 0 };

  res = v_bf16_mul_acc32_b(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_bf16_mul_acc32_b(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_bf16_mul_acc32_b(x0, x1, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_bf16_mul_acc32_b(x0, a, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0, %SP{{[0-9]+}}

  res = v_bf16_mul_acc32_b(x0, a, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0, !%SP{{[0-9]+}}

  res = v_bf16_mul_acc32_b(x0, a, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S0

  res = v_bf16_mul_acc32_b(x0, 1.5, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %SP{{[0-9]+}}

  res = v_bf16_mul_acc32_b(x0, 1.5, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, !%SP{{[0-9]+}}

  res = v_bf16_mul_acc32_b(x0, 1.5, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0
}
