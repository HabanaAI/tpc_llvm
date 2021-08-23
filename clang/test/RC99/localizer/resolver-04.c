// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck %s 

volatile int __local gval = 123;

void main(int x) {
  *(int __local *)x = gval;
}


// CHECK: define void @main(i32 %x) {{.*}} {
// CHECK:   store i32 123, i32 addrspace(1)* null


// CHECK: !llvm.tpc.scalar_data = !{![[SSZ:[0-9]+]]}
// CHECK: !llvm.tpc.vector_data = !{![[VSZ:[0-9]+]]}
// CHECK: ![[SSZ]] = !{i32 4}
// CHECK: ![[VSZ]] = !{i32 0}


// Initialization of global variable
//
// CHECK-ASM: mov.i32 [[REGS0:%S[0-9]+]], 0x7b
// CHECK-ASM: st_l 0x0, [[REGS0]]
//
// Storing value of the global variable
//
// CHECK-ASM: LD_L [[REGS1:%S[0-9]+]], 0x0
// CHECK-ASM: st_l %S0, [[REGS1]]
