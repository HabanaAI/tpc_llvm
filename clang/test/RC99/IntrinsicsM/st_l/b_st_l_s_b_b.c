// RUN: %codegen -S -O1 -triple tpc-none-none  -mllvm -emit-index-factors=false -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -mllvm -emit-index-factors=false -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s


void main(unsigned dest, _Bool value, _Bool pred) {
  b_st_l_s_b_b(dest, value, 0, pred, 0);
  b_st_l_s_b_b(dest+4, value, 1, pred, 1);
}

// CHECK-DAG: mov     %SP[[VALUE:[0-9]+]], %S1
// CHECK-DAG: mov     %SP[[PRED:[0-9]+]], %S2
// CHECK:     st_l    %S0, %SP[[VALUE]], %SP[[PRED]]
// CHECK:     st_l    mmio %S{{[0-9]+}}, %SP[[VALUE]], !%SP[[PRED]]
// CHECK-GEN3P:       st_l    mmio unlock %S{{[0-9]+}}, %SP[[VALUE]], !%SP[[PRED]]
