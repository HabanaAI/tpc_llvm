// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(int dest) {
  int5 ndx = {0, 0, 0, 0, 0};
  int __global *dptr = (int __global *) gen_addr(ndx, 0, 0, 0, 1, 0);
  int32_t_pair_t inc = {0, 0};
  int32_t_pair_t val = s_i32_x2_ld_g_inc(&dptr, SW_L0CS, inc, 1, 0);
  *dptr = val.v1;
}

// CHECK: set_indx  [[NDX:%I[0-9]+]], b11111, 0x0
// CHECK: gen_addr  {{.*}}[[ADRF:%AD[0-9]+]], 0x0, [[NDX]]
// CHECK: ld_g      inc_8 l0cs {{%Z[0-9]+}}, [[ADRF]]
// CHECK: st_g      [[ADRF]], {{%S[0-9]+}}

