// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s


void main(unsigned char x0, unsigned char x1, int dest, _Bool pred) {
  unsigned __local *dptr = (unsigned  __local *)dest;
  unsigned res = 0;

  res = s_u8_mac_s_s(x0, x1, res, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st %S{{[0-9]+}}, %S0, %S1, %SP0

  res = s_u8_mac_s_s(x0, 1, res, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st %S{{[0-9]+}}, %S0, 0x1, %SP0

  res = s_u8_mac_s_s(x0, 1, res, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %S{{[0-9]+}}, %S0, 0x1, %SP0

  res = s_u8_mac_s_s_b(x0, x1, res, 1, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st %S{{[0-9]+}}, %S0, %S1, %SP{{[0-9]+}}

  res = s_u8_mac_s_s_b(x0, 2, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %S{{[0-9]+}}, %S0, 0x2, %SP{{[0-9]+}}
}
