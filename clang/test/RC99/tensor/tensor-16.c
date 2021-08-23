// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -emit-llvm -O1 -tpc-special %s -o - | FileCheck %s

void main(int dest) {
  unsigned __local *ptr = (unsigned __local *)dest;
  set_pad_value_uint(0, *ptr);
}
// 0x408 == 1032
// CHECK: call void @llvm.tpc.st.l.i32(i32 1032, i32 %{{[0-9]+}}, i32 1, i1 true, i1 false)