// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck %s 
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

int gval[4] = { 120, 121, 122, 123 };

void main(int x) {
  *(int __local *)x = gval[0];
}


// CHECK: define void @main(i32 %x) {{.*}} {
// CHECK:   store [4 x i32] [i32 120, i32 121, i32 122, i32 123], [4 x i32] addrspace(1)* null


// CHECK: !llvm.tpc.scalar_data = !{![[SSZ:[0-9]+]]}
// CHECK: !llvm.tpc.vector_data = !{![[VSZ:[0-9]+]]}
// CHECK: ![[SSZ]] = !{i32 16}
// CHECK: ![[VSZ]] = !{i32 0}


// Initialization of global variable
//
// CHECK-ASM-DAG: mov.i32 [[REGS0:%S[0-9]+]], 0x78
// CHECK-ASM-DAG: st_l 0x0, [[REGS0]]
// CHECK-ASM-DAG: mov.i32 [[REGS1:%S[0-9]+]], 0x79
// CHECK-ASM-DAG: st_l 0x4, [[REGS1]]
// CHECK-ASM-DAG: mov.i32 [[REGS2:%S[0-9]+]], 0x7a
// CHECK-ASM-DAG: st_l 0x8, [[REGS2]]
// CHECK-ASM-DAG: mov.i32 [[REGS3:%S[0-9]+]], 0x7b
// CHECK-ASM-DAG: st_l 0xc, [[REGS3]]
