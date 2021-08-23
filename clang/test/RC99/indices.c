// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -emit-llvm -O1 %s -o - | FileCheck %s

void main(int dest) {
  int5 ndx1;
  ndx1 = 0;
  *(volatile __local int5*)dest = ndx1;
// CHECK: store volatile <5 x i32> zeroinitializer, <5 x i32> addrspace(1)* %

  int5 i1 = {1,2,3,9,8};
  *(volatile __local int5*)dest = i1;
// CHECK: store volatile <5 x i32> <i32 1, i32 2, i32 3, i32 9, i32 8>, <5 x i32> addrspace(1)* %

  int5 i2 = (int5){11, 22, 33, 99, 88};
  *(volatile __local int5*)dest = i2;
// CHECK: store volatile <5 x i32> <i32 11, i32 22, i32 33, i32 99, i32 88>, <5 x i32> addrspace(1)* %

  int5 ndx2;
  ndx2.x = 1;
  ndx2.y = 2;
  ndx2.z = 3;
  ndx2.w = 4;
  ndx2.q = 5;
  *(volatile __local int5*)dest = ndx2;
// CHECK: store volatile <5 x i32> <i32 1, i32 2, i32 3, i32 4, i32 5>, <5 x i32> addrspace(1)* %

  int5 ndx3;
  ndx3.xyz = 11;
  ndx3.s3 = 22;
  ndx3.s4 = 33;
  *(volatile __local int5*)dest = ndx3;
// CHECK: store volatile <5 x i32> <i32 11, i32 11, i32 11, i32 22, i32 33>, <5 x i32> addrspace(1)* %

  int5 ndx4;
  ndx4.xyzwq = ndx3.qwzyx;
  *(volatile __local int5*)dest = ndx4;
// CHECK: store volatile <5 x i32> <i32 33, i32 22, i32 11, i32 11, i32 11>, <5 x i32> addrspace(1)* %

  int5 ndx5;
  ndx5.xyzwq = ndx3.qqwqx;
  *(volatile __local int5*)dest = ndx5;
// CHECK: store volatile <5 x i32> <i32 33, i32 33, i32 22, i32 33, i32 11>, <5 x i32> addrspace(1)* %
}
