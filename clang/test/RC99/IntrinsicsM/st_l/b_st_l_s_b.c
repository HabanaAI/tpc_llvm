// RUN: %codegen -S -O1 -triple tpc-none-none -mllvm -emit-index-factors=false -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -mllvm -emit-index-factors=false -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -mllvm -emit-index-factors=false -std=rc99 -bfloat16 -target-cpu goya2 -mllvm -tpc-lock-mismatch-error=0 %s -o - | FileCheck --check-prefixes=CHECK,GEN3P %s

void main(unsigned dest, _Bool value) {
  s_i1_st_l(dest, value, 0, 1, 0);
  s_i1_st_l(dest+4, value, 1, 1, 0);

#if defined(__goya2__)
  s_i1_st_l(dest+8, value, 3, 1, 0);
#endif
}

// CHECK: mov     %SP[[PRED:[0-9]+]], %S1
// CHECK: st_l    %S0, %SP[[PRED]]
// CHECK: st_l    mmio %S{{[0-9]+}}, %SP[[PRED]]
// CHECK-GEN3P:   st_l    mmio unlock %S{{[0-9]+}}, %SP[[PRED]]
