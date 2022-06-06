// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -S %s -o - | FileCheck %s

void main(int addr, float value, _Bool pred) {
  s_f32_st_l(addr, value, 0, pred, 0);
}

// CHECK: st_l %S0, %S1
// CHECK-NOT: st_l mmio
// CHECK: halt