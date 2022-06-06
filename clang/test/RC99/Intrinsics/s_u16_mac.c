// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(unsigned short x0, unsigned short x1, int dest, _Bool pred) {
  unsigned __local *dptr = (unsigned  __local *)dest;
  unsigned res = 0;

  res = s_u16_mac(x0, x1, res, 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 st %S{{[0-9]+}}, %S0, %S1

  res = s_u16_mac(x0, 1, res, 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 st %S{{[0-9]+}}, %S0, 0x1

  res = s_u16_mac(x0, 1, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %S{{[0-9]+}}, %S0, 0x1

  res = s_u16_mac(x0, x1, res, 1, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 st %S{{[0-9]+}}, %S0, %S1, %SP{{[0-9]+}}

  res = s_u16_mac(x0, 2, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %S{{[0-9]+}}, %S0, 0x2, %SP{{[0-9]+}}
}
