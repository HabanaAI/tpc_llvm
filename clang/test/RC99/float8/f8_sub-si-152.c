// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi2 -float8 %s -o - | FileCheck %s
// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu doron1 -float8 %s -o - | FileCheck %s

void main(int dest, int src) {
  minihalf __local *dptr = (minihalf __local *) dest;
  minihalf __local *sptr = (minihalf __local *) src;

  *dptr = *sptr - (minihalf)2.0;
// CHECK: ld_l           [[REG1:%S[0-9]+]], %S1
// CHECK: convert.f8_152 target_type=fp32 [[REG2:%S[0-9]+]], [[REG1]]
// CHECK: add.f32        [[REG3:%S[0-9]+]], [[REG1]], -0x40000000
// CHECK: convert.f32    target_type=f8_152 [[REG4:%S[0-9]+]], [[REG3]]
// CHECK: st_l           %S0, [[REG4]]
}
