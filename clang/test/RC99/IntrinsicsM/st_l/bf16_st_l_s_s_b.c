// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -mllvm -emit-index-factors=false -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s


void main(unsigned dest, bf16 value, _Bool pred){
  bf16_st_l_s_s_b(dest, value, 0, pred, 1);
  bf16_st_l_s_s_b(0x100, value, 1, pred, 0);
}

// CHECK: mov     %SP[[PRED:[0-9]+]], %S2
// CHECK: st_l    %S0, %S1, !%SP[[PRED]]
// CHECK: st_l    mmio 0x100, %S1, %SP[[PRED]]
// CHECK-GEN3P: st_l     mmio unlock %S{{[0-9]+}}, %S1, %SP[[PRED]]
// CHECK-GEN3P: st_l     mmio unlock 0x200, %S1, !%SP[[PRED]]