// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src) {
  bfloat128 __local *dptr = (bfloat128 __local *) dest;
  bfloat128 __local *sptr = (bfloat128 __local *) src;

  *dptr = *sptr;
// CHECK: ld_l_v [[REG:%V[0-9]+]], %S1
// CHECK: st_l_v %S0, [[REG]]

}
