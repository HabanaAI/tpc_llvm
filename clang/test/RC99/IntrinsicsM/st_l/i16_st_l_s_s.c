// RUN: %codegen -S -O1 -triple tpc-none-none -mllvm -emit-index-factors=false -std=rc99  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -mllvm -emit-index-factors=false -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -mllvm -emit-index-factors=false -std=rc99 -bfloat16 -target-cpu goya2 -mllvm -tpc-lock-mismatch-error=0 %s -o - | FileCheck --check-prefixes=CHECK,GEN3P %s

void main(unsigned dest, short value){
  s_i16_st_l(dest, value, 0, 1, 0);
  s_i16_st_l(0x100, value, 1, 1, 0);
#if defined(__goya2__)
  s_i16_st_l(dest+4, value, 3, 1, 0);
  s_i16_st_l(0x200, value, 3, 1, 0);
#endif
}

// CHECK: st_l     %S0, %S1
// CHECK: st_l     mmio 0x100, %S1
// CHECK-GEN3P: st_l     mmio unlock %S{{[0-9]+}}, %S1
// CHECK-GEN3P: st_l     mmio unlock 0x200, %S1
