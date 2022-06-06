// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(signed char x0, signed char x1, int dest, _Bool pred) {
  short __local *dptr = (short  __local *)dest;
  short res = 0;

  res = s_i8_mac_acc16(x0, x1, res, 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 %S{{[0-9]+}}, %S0, %S1

  res = s_i8_mac_acc16(x0, 1, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i8 acc_i16 %S{{[0-9]+}}, %S0, 0x1

  res = s_i8_mac_acc16(x0, 1, res, 2, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i8 neg acc_i16 %S{{[0-9]+}}, %S0, 0x1

  res = s_i8_mac_acc16(x0, x1, res, 1, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 st acc_i16 %S{{[0-9]+}}, %S0, %S1, %SP{{[0-9]+}}

  res = s_i8_mac_acc16(x0, 2, res, 2, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 neg acc_i16 %S{{[0-9]+}}, %S0, 0x2, %SP{{[0-9]+}}
}
