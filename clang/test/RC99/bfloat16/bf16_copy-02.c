// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src) {
  _BFloat16 __local *dptr = (_BFloat16 __local *) dest;
  float __local *sptr = (float __local *) src;

  *dptr = *sptr;
// CHECK: ld_l  %S1, [[REG:%S[0-9]+]]
// CHECK: convert.f32   target_type=bf16 [[REG2:%S[0-9]+]], [[REG]]
// CHECK: st_l %S0, [[REG2]]
  
}
