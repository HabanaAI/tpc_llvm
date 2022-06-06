// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck %s 
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

int64 gval[4] = { 0, 1, 2, 3 };

void main(int x) {
  *(int64 __local *)x = gval[0];
}


// CHECK: define dso_local void @main(i32 %x) {{.*}} {
// CHECK:   store [4 x <64 x i32>] [<64 x i32> zeroinitializer, <64 x i32> <i32 1, i32 1, {{.*}}, i32 1>, <64 x i32> <i32 2, i32 2, {{.*}}, i32 2, i32 2>, <64 x i32> <i32 3, i32 3, {{.*}}, i32 3>], [4 x <64 x i32>] addrspace(2)* null


// CHECK: !llvm.tpc.scalar_data = !{![[SSZ:[0-9]+]]}
// CHECK: !llvm.tpc.vector_data = !{![[VSZ:[0-9]+]]}
// CHECK: ![[SSZ]] = !{i32 0}
// CHECK: ![[VSZ]] = !{i32 1024}


// Initialization of global variable
//
// CHECK-ASM-DAG: mov.i32 [[REGV0:%V[0-9]+]], 0x3
// CHECK-ASM-DAG: mov.i32 [[REGS0:%S[0-9]+]], 0x300
// CHECK-ASM-DAG: st_l_v  [[REGS0]], 0x0, [[REGV0]]
