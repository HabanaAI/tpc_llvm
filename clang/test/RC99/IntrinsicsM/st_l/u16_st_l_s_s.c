// RUN: %codegen -S -O1 -triple tpc-none-none -mllvm -emit-index-factors=false -std=rc99  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -mllvm -emit-index-factors=false -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s


void main(unsigned dest, unsigned short value){
  u16_st_l_s_s(dest, value, 0);
  u16_st_l_s_s(0x100, value, 1);
}

// CHECK: st_l     %S0, %S1, %SP0
// CHECK: st_l     mmio 0x100, %S1, %SP0
// CHECK-GEN3P: st_l     mmio unlock %S{{[0-9]+}}, %S1, %SP0
// CHECK-GEN3P: st_l     mmio unlock 0x200, %S1, %SP0
