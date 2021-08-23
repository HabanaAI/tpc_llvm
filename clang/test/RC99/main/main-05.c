// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -emit-llvm -O1 %s -o - | FileCheck %s

void main(int dest, tensor t0, float arg2, tensor t1, tensor t2) {
  *(int volatile __local *)dest = t0;
  *(int volatile __local *)dest = t1;
  *(int volatile __local *)dest = t2;
}

// CHECK: define void @main(i32 %dest, float %arg2) local_unnamed_addr #0 {
// CHECK: store volatile i32 0, i32 addrspace(1)* %{{[0-9]+}}
// CHECK: store volatile i32 1, i32 addrspace(1)* %{{[0-9]+}}
// CHECK: store volatile i32 2, i32 addrspace(1)* %{{[0-9]+}}
