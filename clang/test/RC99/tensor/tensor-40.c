// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -emit-llvm -O1 -tpc-special %s -o - | FileCheck %s

void main(int dest) {
  unsigned __local *ptr = (unsigned __local *)dest;
  *ptr = get_dim_stride(15, 0);
}
// 0x888 == 2184 
// CHECK: %{{[0-9]+}} = {{.*}}call i32 @llvm.tpc.ld.l.i32(i32 2184, i32 1, i32 {{.*}}, i1 true, i1 false)
