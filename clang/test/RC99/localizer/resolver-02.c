// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -emit-llvm -O1 %s -o - | FileCheck %s 
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

struct ABC {
  float field1;
  int field2[12];
};

struct ABC vvv;

void main(int x) {
  vvv.field1 = 1.0;
  vvv.field2[1] = 777;
}


// CHECK: store float 1.000000e+00, float addrspace(1)* null
// CHECK: store i32 777, i32 addrspace(1)* getelementptr inbounds (%struct.ABC, %struct.ABC addrspace(1)* null, i32 0, i32 1, i32 1)


// CHECK: !llvm.tpc.scalar_data = !{![[SSZ:[0-9]+]]}
// CHECK: !llvm.tpc.vector_data = !{![[VSZ:[0-9]+]]}
// CHECK: ![[SSZ]] = !{i32 52}
// CHECK: ![[VSZ]] = !{i32 0}


// CHECK-ASM-DAG: mov.i32 %S[[REG1:[0-9]+]], 0x3f800000
// CHECK-ASM-DAG: st_l 0x0, %S[[REG1]]
// CHECK-ASM-DAG: mov.i32 %S[[REG2:[0-9]+]], 0x309
// CHECK-ASM-DAG: st_l 0x8, %S[[REG2]]
