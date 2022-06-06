// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi2 -float8 %s -o - | FileCheck %s
// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu doron1 -float8 %s -o - | FileCheck %s

void main(int dest, int src, int src2) {
  minihalf256 __local *dptr = (minihalf256 __local *) dest;
  minihalf256 __local *sptr = (minihalf256 __local *) src;
  minihalf256 __local *sptr2 = (minihalf256 __local *) src2;

  *dptr = *sptr - *sptr2;
// CHECK: ld_l_v         [[REG1:%V[0-9]+]], %S1
// CHECK: ld_l_v         [[REG2:%V[0-9]+]], %S2
// CHECK: convert.f8_152 all_lanes target_type=fp32 {{.*}}[[REG3:%A[0-9]+]], [[REG1]]
// CHECK: sub.f8_152     [[REG4:%A[0-9]+]], [[REG3]], [[REG2]]
// CHECK: convert.f32    all_lanes target_type=f8_152 rhne [[REG5:%V[0-9]+]], [[REG4]]
// CHECK: st_l_v         %S0, [[REG5]]
}
