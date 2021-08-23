// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck %s 
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

int gval[10] = { 0 };

void main(int x) {
  *(int __local *)x = gval[0];
}


// CHECK: define void @main(i32 %x) {{.*}} {
// CHECK:   store [10 x i32] zeroinitializer, [10 x i32] addrspace(1)* null


// CHECK: !llvm.tpc.scalar_data = !{![[SSZ:[0-9]+]]}
// CHECK: !llvm.tpc.vector_data = !{![[VSZ:[0-9]+]]}
// CHECK: ![[SSZ]] = !{i32 40}
// CHECK: ![[VSZ]] = !{i32 0}


// Initialization of global variable
//
// CHECK-ASM: mov.i32 [[REGS:%S[0-9]+]], 0
// CHECK-ASM: st_l 0x0, [[REGS]]
