// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest) {
  bfloat128 __local *dptr = (bfloat128 __local *) dest;

  *dptr = 8.0bf;
// CHECK: mov.bf16 [[REG:%V[0-9]+]], 0x4100
// CHECK: st_l_v   %S0, [[REG]]
}
