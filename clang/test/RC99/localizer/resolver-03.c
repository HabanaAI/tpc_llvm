// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck %s 
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

int64 vvv[2];

void main(int x) {
  vvv[1] = 1.0;
}


// CHECK: define dso_local void @main(i32 %x) {{.*}} {
// CHECK:   store <64 x i32> <i32 1, {{.*}}, i32 1>, <64 x i32> addrspace(2)* getelementptr inbounds ([2 x <64 x i32>], [2 x <64 x i32>] addrspace(2)* null, i32 0, i32 1), align 256


// CHECK: !llvm.tpc.scalar_data = !{![[SSZ:[0-9]+]]}
// CHECK: !llvm.tpc.vector_data = !{![[VSZ:[0-9]+]]}
// CHECK: ![[SSZ]] = !{i32 0}
// CHECK: ![[VSZ]] = !{i32 512}


// Element 0 must be zero-initialized.
//
// CHECK-ASM-DAG: mov.i32 [[REGV:%V[0-9]+]], 0x1
// CHECK-ASM-DAG: mov.i32 [[REGS:%S[0-9]+]], 0x100
// CHECK-ASM:     st_l_v  [[REGS]], 0x0, [[REGV]]
