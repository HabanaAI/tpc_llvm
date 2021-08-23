// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -emit-llvm -O1 -tpc-special %s -o - | FileCheck %s

void main(int dest) {
  unsigned __local *ptr = (unsigned __local *)dest;
  set_dim_size(1, 2, *ptr);
}
// 0x474 == 1140
// CHECK: call void @llvm.tpc.st.l.i32(i32 1140, i32 %{{[0-9]+}}, i32 1, i1 true, i1 false)