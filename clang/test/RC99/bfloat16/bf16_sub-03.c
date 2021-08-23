// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src, _BFloat16 src2) {
  bfloat128 __local *dptr = (bfloat128 __local *) dest;
  bfloat128 __local *sptr = (bfloat128 __local *) src;

  *dptr = *sptr - src2;
// CHECK: ld_l_v [[REG1:%V[0-9]+]], %S1
// CHECK: sub.bf16 [[REG3:%V[0-9]+]], [[REG1]], %S2
// CHECK: st_l_v %S0, [[REG3]]
}
