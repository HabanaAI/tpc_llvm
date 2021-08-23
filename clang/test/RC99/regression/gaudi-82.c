// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O1 %s -o - | FileCheck %s

volatile int64 a[10];

void main(tensor out, int dest) {
  int64 t = a[5];
  *(int64 __local *)dest = t;
}

// CHECK: mov.i32 [[REGS:%S[0-9]+]], 0x500
// CHECK: ld_l_v  [[REGV:%V[0-9]+]], [[REGS]], 0x0
// CHECK: st_l_v  %S0, 0x0, [[REGV]]
