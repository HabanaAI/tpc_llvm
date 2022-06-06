// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu doron1 %s -o - | FileCheck %s

void main(int x0, int dest) {
  float256     __local *ptr_x0 = (float256 __local *)x0;
  float256     __local *dptr   = (float256 __local *)dest;

  float256 res = {0};
  res = v_f8_sub_b(*ptr_x0, (minifloat)8.0, 0, res, 1, 0);
  *dptr = res;
  // CHECK:  ld_l_v      %V[[NX0:[0-9]+]], %S0
  // CHECK:  sub.f8_143  %A[[NRES:[0-9]+]], %A[[NX0]], 0x50
  // CHECK:  st_l_v      %S1, %V[[NRES]]
}
