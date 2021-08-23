// RUN: %codegen -S -O1 -triple tpc-none-none -mllvm -emit-index-factors=false -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -mllvm -emit-index-factors=false -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s


void main(unsigned dest, _Bool value) {
  b_st_l_s_b(dest, value, 0);
  b_st_l_s_b(dest+4, value, 1);
}

// CHECK: mov     %SP[[PRED:[0-9]+]], %S1
// CHECK: st_l    %S0, %SP[[PRED]], %SP0
// CHECK: st_l    mmio %S{{[0-9]+}}, %SP[[PRED]], %SP0
// CHECK-GEN3P:   st_l    mmio unlock %S{{[0-9]+}}, %SP[[PRED]], %SP0
