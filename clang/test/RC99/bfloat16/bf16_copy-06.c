// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest, _BFloat16 src) {
  bfloat128 __local *dptr = (bfloat128 __local *) dest;

  *dptr = src;
// CHECK: mov.bf16 [[REG:%V[0-9]+]], %S1
// CHECK: st_l_v %S0, [[REG]]
}
