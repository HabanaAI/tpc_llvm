// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck %s
//XFAIL: *
void main(unsigned addr, int dest, _Bool pred) {
  int __local *dptr = (int __local *)dest;

  _Bool result = s_i1_ld_l(addr, 0, 0, pred, 0);
  *dptr++ = result;
  
  result = s_i1_ld_l(addr, 1, 0, pred, 1);
  *dptr++ = result;
  
  result = s_i1_ld_l(0x20, 0, 0, pred, 1);
  *dptr++ = result;
  
  result = s_i1_ld_l(0x20, 1, 0, pred, 0);
  *dptr = result;
}

//CHECK: ld_l     %SP[[Pred1:[0-9]+]], %S0, %SP[[PredA:[0-9]+]]
//CHECK: mov %S[[Val1:[0-9]+]], 0x1, %SP[[Pred1]]
//CHECK: mov %S[[Val1]], 0x0, !%SP[[Pred1]]
//CHECK: st_l     %S{{[0-9]+}}, %S[[Val1]]

//CHECK: ld_l     mmio %SP[[Pred2:[0-9]+]], %S0, !%SP[[PredA]]
//CHECK: mov %S[[Val2:[0-9]+]], 0x1, %SP[[Pred2]]
//CHECK: mov %S[[Val2]], 0x0, !%SP[[Pred2]]
//CHECK: st_l     %S{{[0-9]+}}, %S[[Val2]]

//CHECK: ld_l     %SP[[Pred3:[0-9]+]], 0x20, !%SP[[PredA]]
//CHECK: mov %S[[Val3:[0-9]+]], 0x1, %SP[[Pred3]]
//CHECK: mov %S[[Val3]], 0x0, !%SP[[Pred3]]
//CHECK: st_l     %S{{[0-9]+}}, %S[[Val3]]

//CHECK: ld_l     mmio %SP[[Pred4:[0-9]+]], 0x20, %SP[[PredA]]
//CHECK: mov %S[[Val4:[0-9]+]], 0x1, %SP[[Pred4]]
//CHECK: mov %S[[Val4]], 0x0, !%SP[[Pred4]]
//CHECK: st_l     %S{{[0-9]+}}, %S[[Val4]]
