// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src) {
  _BFloat16 __local *dptr = (_BFloat16 __local *) dest;
  _BFloat16 __local *sptr = (_BFloat16 __local *) src;

  *dptr = 8.0bf + *sptr;
// CHECK: ld_l  [[REG:%S[0-9]+]], %S1
// CHECK: add.bf16 [[REG2:%S[0-9]+]], [[REG]], 0x4100
// CHECK: st_l %S0, [[REG2]]
}
