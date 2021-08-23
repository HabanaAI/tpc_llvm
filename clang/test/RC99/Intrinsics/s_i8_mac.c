// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s


void main(signed char x0, signed char x1, int dest, _Bool pred) {
  signed __local *dptr = (signed  __local *)dest;
  signed res = 0;

  res = s_i8_mac_s_s(x0, x1, res, 1);
  *dptr++ = res;
  // CHECK: mac.i8 st %S{{[0-9]+}}, %S0, %S1, %SP0

  res = s_i8_mac_s_s(x0, 1, res, 1);
  *dptr++ = res;
  // CHECK: mac.i8 st %S{{[0-9]+}}, %S0, 0x1, %SP0

  res = s_i8_mac_s_s(x0, 1, res, 0);
  *dptr++ = res;
  // CHECK: mac.i8 %S{{[0-9]+}}, %S0, 0x1, %SP0

  res = s_i8_mac_s_s_b(x0, x1, res, 1, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 st %S{{[0-9]+}}, %S0, %S1, %SP{{[0-9]+}}

  res = s_i8_mac_s_s_b(x0, 2, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i8 %S{{[0-9]+}}, %S0, 0x2, %SP{{[0-9]+}}
}
