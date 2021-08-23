// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src, _BFloat16 src2) {
  _BFloat16 __local *dptr = (_BFloat16 __local *) dest;
  _BFloat16 __local *sptr = (_BFloat16 __local *) src;

  *dptr = *sptr * src2;
// CHECK: ld_l     [[REG:%S[0-9]+]], %S1
// CHECK: mul.bf16 [[REG2:%S[0-9]+]], [[REG]], %S2
// CHECK: st_l     %S0, [[REG2]]
}
