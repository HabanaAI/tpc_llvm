// RUN: %codegen -S -O1 -triple tpc-none-none  -mllvm -emit-index-factors=false -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -mllvm -emit-index-factors=false -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none  -mllvm -emit-index-factors=false -std=rc99 -bfloat16 -target-cpu goya2 -mllvm -tpc-lock-mismatch-error=0 %s -o - | FileCheck --check-prefixes=CHECK,GEN3P %s

void main(unsigned dest, _Bool value, _Bool pred) {
  s_i1_st_l(dest, value, 0, pred, 0);
  s_i1_st_l(dest+4, value, 1, pred, 1);
#if defined(__goya2__)
  s_i1_st_l(dest+8, value, 3, pred, 1);
#endif
}

// CHECK-DAG: mov     %SP[[VALUE:[0-9]+]], %S1
// CHECK-DAG: mov     %SP[[PRED:[0-9]+]], %S2
// CHECK:     st_l    %S0, %SP[[VALUE]], %SP[[PRED]]
// CHECK:     st_l    mmio %S{{[0-9]+}}, %SP[[VALUE]], !%SP[[PRED]]
// CHECK-GEN3P:       st_l    mmio unlock %S{{[0-9]+}}, %SP[[VALUE]], !%SP[[PRED]]
