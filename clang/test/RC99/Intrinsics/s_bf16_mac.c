// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(_BFloat16 x0, _BFloat16 x1, int dest, _Bool pred) {
  _BFloat16 __local *dptr = (_BFloat16  __local *)dest;
  _BFloat16 res = 0;

  res = s_bf16_mac(x0, x1, res, (0) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 %S{{[0-9]+}}, %S0, %S1

  res = s_bf16_mac(x0, 1.5, res, (0) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 %S{{[0-9]+}}, %S0, 0x3fc0

  res = s_bf16_mac(x0, x1, res, (0) << 1, pred, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 %S{{[0-9]+}}, %S0, %S1, %SP{{[0-9]+}}

  res = s_bf16_mac(x0, 1.5, res, (0) << 1, pred, 0);
  *dptr++ = res;
  // CHECK: mac.bf16 %S{{[0-9]+}}, %S0, 0x3fc0, %SP{{[0-9]+}}
}
