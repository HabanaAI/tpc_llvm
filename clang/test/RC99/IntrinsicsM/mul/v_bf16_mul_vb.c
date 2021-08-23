// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s


void main(_BFloat16 a, _BFloat16 b, int dest, int src) {
  bfloat128 __local *dptr = (bfloat128 __local *)dest;
  bfloat128 x0 = *(bfloat128 __local *)(src + 0 * 256);
  bfloat128 x1 = *(bfloat128 __local *)(src + 1 * 256);
  bool128 pred = *(bool128 __local *)(src + 2 * 256);
  bfloat128 res = 0;

  res = v_bf16_mul_vb(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_bf16_mul_vb(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}

  res = v_bf16_mul_vb(x0, a, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, %S0, %VP{{[0-9]+}}

  res = v_bf16_mul_vb(x0, a, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, %S0, !%VP{{[0-9]+}}

  res = v_bf16_mul_vb(x0, 1.5, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %VP{{[0-9]+}}

  res = v_bf16_mul_vb(x0, 1.5, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, !%VP{{[0-9]+}}
}
