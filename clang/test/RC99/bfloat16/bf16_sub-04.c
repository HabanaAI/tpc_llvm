// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src) {
  bfloat128 __local *dptr = (bfloat128 __local *) dest;
  bfloat128 __local *sptr = (bfloat128 __local *) src;

  *dptr = *sptr - 8.0bf;
// CHECK: ld_l_v [[REG1:%V[0-9]+]], %S1
// Possible result: SUB.128BF [[REG3:%V[0-9]+]], [[REG1]], 0x4100
// CHECK: add.bf16 [[REG3:%V[0-9]+]], [[REG1]], 0xc100
// CHECK: st_l_v %S0, [[REG3]]
}
