// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck %s

void main(unsigned addr, int dest, _Bool pred) {
  half __local *dptr = (half __local *)dest;

  half result = s_f16_ld_l(addr, 0, 0, 1, 0);
  *dptr++ = result;
  
  result = s_f16_ld_l(addr, 1, 0, 1, 0);
  *dptr++ = result;
  
  result = s_f16_ld_l(0x20, 0, 0, pred, 0);
  *dptr++ = result;
  
  result = s_f16_ld_l(0x20, 1, 0, pred, 1);
  *dptr = result;
}

//CHECK: ld_l     %S[[Val1:[0-9]+]], %S0
//CHECK: st_l     %S{{[0-9]+}}, %S[[Val1]]

//CHECK: ld_l     mmio %S[[Val2:[0-9]+]], %S0
//CHECK: st_l     %S{{[0-9]+}}, %S[[Val2]]

//CHECK: ld_l     %S[[Val3:[0-9]+]], 0x20, %SP{{[0-9]+}}
//CHECK: st_l     %S{{[0-9]+}}, %S[[Val3]]

//CHECK: ld_l     mmio %S[[Val4:[0-9]+]], 0x20, !%SP{{[0-9]+}}
//CHECK: st_l     %S{{[0-9]+}}, %S[[Val4]]
