// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -S %s -o - | FileCheck %s

void main(int start, int end, int out) {
  int __local *ptr_dest = (int __local *)out;
  for (int addr = start; addr < end; addr+=4) {
    int res = 0;
    res = s_i32_ld_l(addr, SW_MMIO, res, 1, 0);
    *ptr_dest++ = res;
  }
}
// CHECK: ld_l mmio [[FENCE:%S[0-9]+]], 0x440
// CHECK: ld_l mmio
// CHECK: st_l mmio [[FENCE]],
// CHECK: halt
