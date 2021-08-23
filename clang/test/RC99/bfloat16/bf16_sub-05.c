// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// XFAIL: *

void main(int dest, int src) {
  bfloat128 __local *dptr = (bfloat128 __local *) dest;
  bfloat128 __local *sptr = (bfloat128 __local *) src;

  *dptr = 8.0bf - *sptr;
// CHECK: ld_l_v   [[REG1:%V[0-9]+]], %S1
// CHECK: mov.bf16 [[REG2:%V[0-9]+]], 0x4100
// CHECK: sub.bf16 [[REG3:%V[0-9]+]], [[REG2]], [[REG1]]
// CHECK: st_l_v   %S0, [[REG3]]
}
