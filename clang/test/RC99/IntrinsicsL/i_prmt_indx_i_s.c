// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s


void main(int dest, unsigned x) {
  int5 indx = { 0, 0, 0, 0, 0 };
  int5 result = i_prmt_indx_i_s(indx, x);
  *(int __local *)dest = result[0];
}

// CHECK: set_indx    %I[[NDX:[0-9]+]], b11111, 0x0, %SP0
// CHECK: prmt_indx   %I[[NDX2:[0-9]+]], %I[[NDX]], %S1, %SP0
// CHECK: mov_irf_dim 0x0 %S[[VAL:[0-9]+]], %I[[NDX2]], %SP0
// CHECK: st_l        %S0, %S[[VAL]]
// CHECK: halt
