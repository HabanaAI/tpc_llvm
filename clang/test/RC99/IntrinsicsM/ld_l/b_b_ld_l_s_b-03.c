// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 -mllvm -tpc-lock-mismatch-error=0 %s -o - | FileCheck %s
//XFAIL: *
void main(unsigned addr, int dest, _Bool pred) {
  int __local *dptr = (int __local *)dest;

  _Bool result = s_i1_ld_l(addr, 0, 0, pred, 0);
  *dptr++ = result;

  result = s_i1_ld_l(0x20, 3, 0, pred, 1);
  *dptr++ = result;

  result = s_i1_ld_l(0x20, 3, 0, pred, 0);
  *dptr = result;

}

//CHECK: ld_l    mmio lock %SP[[Pred3:[0-9]+]], 0x20, !%SP[[PredA:[0-9]+]]
//CHECK: mov %S[[Val3:[0-9]+]], 0x1, %SP[[Pred3]]
//CHECK: mov %S[[Val3]], 0x0, !%SP[[Pred3]]
//CHECK: st_l     %S{{[0-9]+}}, %S[[Val3]]

//CHECK: ld_l    mmio  lock %SP[[Pred4:[0-9]+]], 0x20, %SP[[PredA]]
//CHECK: mov %S[[Val4:[0-9]+]], 0x1, %SP[[Pred4]]
//CHECK: mov %S[[Val4]], 0x0, !%SP[[Pred4]]
//CHECK: st_l     %S{{[0-9]+}}, %S[[Val4]]