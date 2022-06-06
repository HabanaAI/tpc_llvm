// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(int dest, int src) {
  int5 ndx = {0, 0, 0, 0, 0};
  _Bool __global *dptr = (_Bool __global *) gen_addr(ndx, 0, 0, 0, 1, 0);
  int val = s_i1_ld_g_inc(&dptr, 0, 0, 1, 0);
  *dptr = val;
}

// CHECK: set_indx  [[NDX:%I[0-9]+]], b11111, 0x0
// CHECK: gen_addr  {{.*}}[[ADRF:%AD[0-9]+]], 0x0, [[NDX]]
// CHECK: ld_g      inc_1 %SP{{[0-9]+}}, [[ADRF]]
