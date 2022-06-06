// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(half x0, half x1, int dest, _Bool pred) {
  float __local *dptr = (float __local *)dest;
  float res = 0;

  res = s_f16_mac_acc32(x0, x1, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %S{{[0-9]+}}, %S0, %S1

  res = s_f16_mac_acc32(x0, 1.5, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f16 neg acc_fp32 %S{{[0-9]+}}, %S0, 0x3e00

  res = s_f16_mac_acc32(x0, x1, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f16 acc_fp32 %S{{[0-9]+}}, %S0, %S1, %SP{{[0-9]+}}

  res = s_f16_mac_acc32(x0, 1.5, res, SW_NEG, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f16 neg acc_fp32 %S{{[0-9]+}}, %S0, 0x3e00, %SP{{[0-9]+}}
}
