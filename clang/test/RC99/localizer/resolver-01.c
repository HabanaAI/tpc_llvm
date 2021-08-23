// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck %s 
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

// NB! Assume that globals are ordered as in source file.
int ggg[12];
int vvv[2];

void main(int x) {
  ggg[1] = 11;
  vvv[0] = 777;
}


// CHECK: store i32 11, i32 addrspace(1)* getelementptr inbounds ([12 x i32], [12 x i32] addrspace(1)* null, i32 0, i32 1)
// CHECK: store i32 777, i32 addrspace(1)* getelementptr inbounds ([2 x i32], [2 x i32] addrspace(1)* inttoptr (i32 48 to [2 x i32] addrspace(1)*), i32 0, i32 0)


// CHECK: !llvm.tpc.scalar_data = !{![[SSZ:[0-9]+]]}
// CHECK: !llvm.tpc.vector_data = !{![[VSZ:[0-9]+]]}
// CHECK: ![[SSZ]] = !{i32 56}
// CHECK: ![[VSZ]] = !{i32 0}


// CHECK-ASM-DAG: mov.i32 %S[[REG1:[0-9]+]], 0xb
// CHECK-ASM-DAG: st_l 0x4, %S[[REG1]]
// CHECK-ASM-DAG: mov.i32 %S[[REG2:[0-9]+]], 0x309
// CHECK-ASM-DAG: st_l 0x30, %S[[REG2]]
