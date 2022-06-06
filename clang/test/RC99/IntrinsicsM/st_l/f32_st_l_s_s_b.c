// RUN: %codegen -S -O1 -triple tpc-none-none -mllvm -emit-index-factors=false -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -mllvm -emit-index-factors=false -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -mllvm -emit-index-factors=false -std=rc99 -bfloat16 -target-cpu goya2 -mllvm -tpc-lock-mismatch-error=0 %s -o - | FileCheck --check-prefixes=CHECK,GEN3P %s

void main(unsigned dest, float value, _Bool pred){
  s_f32_st_l(dest, value, 0, pred, 1);
  s_f32_st_l(0x100, value, 1, pred, 0);
#if defined(__goya2__)
  s_f32_st_l(dest+4, value, 3, pred, 1);
  s_f32_st_l(0x200, value, 3, pred, 0);
#endif
}

// CHECK: mov %SP[[PRED:[0-9]+]], %S2
// CHECK: st_l    %S0, %S1, !%SP[[PRED]]
// CHECK: st_l    mmio 0x100, %S1, %SP[[PRED]]
// CHECK-GEN3P: st_l     mmio unlock %S{{[0-9]+}}, %S1, %SP[[PRED]]
// CHECK-GEN3P: st_l     mmio unlock 0x200, %S1, !%SP[[PRED]]