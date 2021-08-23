// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck %s 
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

volatile int64 gval = 0;

void main(int x) {
  *(int64 __local *)x = gval;
}


// CHECK: define void @main(i32 %x) {{.*}} {
// CHECK:   store <64 x i32> zeroinitializer, <64 x i32> addrspace(2)* null


// CHECK: !llvm.tpc.scalar_data = !{![[SSZ:[0-9]+]]}
// CHECK: !llvm.tpc.vector_data = !{![[VSZ:[0-9]+]]}
// CHECK: ![[SSZ]] = !{i32 0}
// CHECK: ![[VSZ]] = !{i32 256}


// Initialization of global variable
//
// CHECK-ASM: mov.i32 [[REGV:%V[0-9]+]], 0
// CHECK-ASM: mov.i32 [[REGS:%S[0-9]+]], 0
// CHECK-ASM: st_l_v  [[REGS]], 0x0, [[REGV]]
