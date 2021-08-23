// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src) {
  _BFloat16 __local *dptr = (_BFloat16 __local *) dest;
  _BFloat16 __local *sptr = (_BFloat16 __local *) src;

  *dptr = *sptr;
// CHECK: ld_l  %S1, [[REG1:%S[0-9]+]]
// CHECK: st_l %S0, [[REG1]]
}
