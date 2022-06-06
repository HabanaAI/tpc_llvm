// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi2 -float8 %s -o - | FileCheck %s
// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu doron1 -float8 %s -o - | FileCheck %s

void main(int dest, int src) {
  minihalf256 __local *dptr = (minihalf256 __local *) dest;
  minihalf256 __local *sptr = (minihalf256 __local *) src;

  *dptr = *sptr + (minihalf256)2.0;
// CHECK: ld_l_v         [[REG1:%V[0-9]+]], %S1
// CHECK: convert.f8_152 all_lanes target_type=fp32 {{.*}}[[REG2:%A[0-9]+]], [[REG1]]
// CHECK: add.f8_152     [[REG3:%A[0-9]+]], [[REG2]], 0x40
// CHECK: convert.f32    all_lanes target_type=f8_152 rhne [[REG4:%V[0-9]+]], [[REG3]]
// CHECK: st_l_v         %S0, [[REG4]]
}
