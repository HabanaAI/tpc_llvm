// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(unsigned short x0, unsigned short x1, int dest, _Bool pred) {
  unsigned int __local *dptr = (unsigned int  __local *)dest;
  unsigned int res = 0;

  res = s_u16_mac_acc32(x0, x1, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 st acc_i32 %S{{[0-9]+}}, %S0, %S1

  res = s_u16_mac_acc32(x0, 1, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 acc_i32 %S{{[0-9]+}}, %S0, 0x1

  res = s_u16_mac_acc32(x0, 1, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 neg acc_i32 %S{{[0-9]+}}, %S0, 0x1

  res = s_u16_mac_acc32(x0, x1, res, SW_SAT, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 st acc_i32 %S{{[0-9]+}}, %S0, %S1, %SP{{[0-9]+}}

  res = s_u16_mac_acc32(x0, 2, res, SW_NEG, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 neg acc_i32 %S{{[0-9]+}}, %S0, 0x2, %SP{{[0-9]+}}
}
