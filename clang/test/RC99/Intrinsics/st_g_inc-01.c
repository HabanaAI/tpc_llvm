// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(int dest, int src) {
  int5 ndx = {0, 0, 0, 0, 0};
  int __global *dptr = (int __global *) gen_addr(ndx, 0, 0, 0, 1, 0);
  s_i32_st_g_inc(&dptr, src, 0, 1, 0);
  *dptr = 22;
}

// CHECK: st_g  inc_4 [[ADRF:%AD[0-9]+]], %S1
// CHECK: st_g  [[ADRF]], %S{{[0-9]+}}

