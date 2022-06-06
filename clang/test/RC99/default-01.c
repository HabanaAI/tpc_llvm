// RUN: %clang_cc1 -S -O2  -emit-llvm -triple tpc -std=rc99 -target-cpu goya2 -mllvm -tpc-trans-intr=0 %s -o - | FileCheck %s

void main(int dest, int x, int y) {
  int __local *dptr = (int __local *)dest;
  *dptr++ = s_i32_abs(x);
  *dptr++ = s_i32_abs(x, SW_NO_SAT);
  *dptr++ = s_i32_abs(y, 0, 1);
  *dptr++ = s_i32_abs(x, 0, 2, 0);
  *dptr++ = s_i32_abs(x, 0, 2, 0, 1);
}

// CHECK: call i32 @llvm.tpc.abs.i32.i32.i1(i32 %x, i8 2, i32 0, i32 undef, i1 true, i1 false)
// CHECK: call i32 @llvm.tpc.abs.i32.i32.i1(i32 %x, i8 2, i32 1, i32 undef, i1 true, i1 false)
// CHECK: call i32 @llvm.tpc.abs.i32.i32.i1(i32 %y, i8 2, i32 0, i32 undef, i1 true, i1 false)
// CHECK: call i32 @llvm.tpc.abs.i32.i32.i1(i32 %x, i8 2, i32 0, i32 2, i1 false, i1 false)
// CHECK: call i32 @llvm.tpc.abs.i32.i32.i1(i32 %x, i8 2, i32 0, i32 undef, i1 false, i1 true)
