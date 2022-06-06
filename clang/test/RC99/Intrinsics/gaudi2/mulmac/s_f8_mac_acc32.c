// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1  %s -o - | FileCheck %s

void main(minifloat x0, minifloat x1, int dest, _Bool pred) {
  float __local *dptr = (float __local *)dest;
  float res = 0;

  res = s_f8_mac_acc32(x0, x1, res, 0,1,0);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %S{{[0-9]+}}, %S0, %S1

  res = s_f8_mac_acc32(x0, (minifloat)1.5, res, 1<<1,1,0);
  *dptr++ = res;
  // CHECK: mac.f8_143 neg acc_fp32 %S{{[0-9]+}}, %S0, 0x3c

  res = s_f8_mac_acc32(x0, x1, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 acc_fp32 %S{{[0-9]+}}, %S0, %S1, %SP{{[0-9]+}}

  res = s_f8_mac_acc32(x0, (minifloat)1.5, res, 1<<1, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f8_143 neg acc_fp32 %S{{[0-9]+}}, %S0, 0x3c, %SP{{[0-9]+}}
}
