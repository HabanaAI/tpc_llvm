// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -S -O1 -emit-llvm %s -o - | FileCheck %s


const int coeff[] = { 11, 22, 33 };

void main(int dest, int src) {
  int __local *srcptr = (int __local *)src;
  int __local *destptr = (int __local *)dest;
  for (int i = 0; i < 3; ++i) {
    destptr[i] = srcptr[i] * coeff[i];
  }
}

// NOTE: the first line may absent from gerenated ll file, if debug info will be
// implemented differently and global symbol will not be needed.

// CHECK: @coeff = external dso_local local_unnamed_addr addrspace(1) constant [3 x i32]
// CHECK-NOT: @coeff