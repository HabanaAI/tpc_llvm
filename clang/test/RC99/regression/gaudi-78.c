// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O1 %s -o - | FileCheck %s

void main(tensor out) {
  volatile int64 a[10] = {0};
}

// CHECK-DAG: mov.i32 [[REGV:%V[0-9]+]], 0x0
// CHECK-DAG: mov.i32 [[REGS0:%S[0-9]+]], 0x0
// CHECK-DAG: st_l_v  [[REGS0]], 0x0, [[REGV]]
// CHECK-DAG: mov.i32 [[REGS1:%S[0-9]+]], 0x100
// CHECK-DAG: st_l_v  [[REGS1]], 0x0, [[REGV]]
// CHECK-DAG: mov.i32 [[REGS2:%S[0-9]+]], 0x200
// CHECK-DAG: st_l_v  [[REGS2]], 0x0, [[REGV]]
// CHECK-DAG: mov.i32 [[REGS9:%S[0-9]+]], 0x900
// CHECK-DAG: st_l_v  [[REGS9]], 0x0, [[REGV]]
