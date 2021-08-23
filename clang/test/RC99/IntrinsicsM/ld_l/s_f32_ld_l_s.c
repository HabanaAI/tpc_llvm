// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s


void main(unsigned addr, int dest) {
  float __local *dptr = (float __local *)dest;

  float result = s_f32_ld_l_s(addr, 0);
  *dptr++ = result;
  
  result = s_f32_ld_l_s(addr, 1);
  *dptr++ = result;
  
  result = s_f32_ld_l_s(0x20, 0);
  *dptr++ = result;
  
  result = s_f32_ld_l_s(0x20, 1);
  *dptr = result;
}

//CHECK: ld_l     %S[[Val1:[0-9]+]], %S0, %SP0
//CHECK: st_l     %S{{[0-9]+}}, %S[[Val1]]

//CHECK: ld_l     mmio %S[[Val2:[0-9]+]], %S0, %SP0
//CHECK: st_l     %S{{[0-9]+}}, %S[[Val2]]

//CHECK: ld_l     %S[[Val3:[0-9]+]], 0x20, %SP0
//CHECK: st_l     %S{{[0-9]+}}, %S[[Val3]]

//CHECK: ld_l     mmio %S[[Val4:[0-9]+]], 0x20, %SP0
//CHECK: st_l     %S{{[0-9]+}}, %S[[Val4]]
