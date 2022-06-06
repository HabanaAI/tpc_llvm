// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu doron1 %s -o - | FileCheck %s

void main(int x0, minihalf x1, int dest) {
  float256     __local *ptr_x0 = (float256 __local *)x0;
  float256     __local *dptr   = (float256 __local *)dest;

  float256 res = {0};
  res = v_h8_add_b(*ptr_x0, x1, 0, res, 1, 0);
  *dptr = res;
  // CHECK:  ld_l_v      %V[[NX0:[0-9]+]], %S0
  // CHECK:  add.f8_152  %A[[NRES:[0-9]+]], %A[[NX0]], %S1
  // CHECK:  st_l_v      %S2, %V[[NRES]]
}
