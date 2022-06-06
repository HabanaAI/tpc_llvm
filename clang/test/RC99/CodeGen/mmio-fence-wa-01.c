// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -S %s -o - | FileCheck %s

void main(int addr, float value, _Bool pred) {
  s_f32_st_l(addr, value, SW_MMIO, pred, 0);
}
// CHECK: ld_l mmio [[FENCE:%S[0-9]+]], 0x440
// CHECK: st_l mmio %S0, %S1
// CHECK: st_l mmio [[FENCE]],
// CHECK: halt
