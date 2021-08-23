// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src) {
  float __local *dptr = (float __local *) dest;
  _BFloat16 __local *sptr = (_BFloat16 __local *) src;

  *dptr = *sptr;
// CHECK: ld_l  %S1, [[REG:%S[0-9]+]]
// CHECK: convert.bf16 target_type=fp32 [[REG2:%S[0-9]+]], [[REG]]
// CHECK: st_l %S0, [[REG2:%S[0-9]+]]

}
