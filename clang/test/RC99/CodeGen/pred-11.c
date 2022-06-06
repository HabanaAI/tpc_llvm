// RUN: %codegen -triple tpc-none-none -std=rc99 -S -O1 -tpc-special %s -o - | FileCheck %s

void main(int dest, int src1, int src2) {
  int val = 0;
  val = s_i32_add(src1, src2, SW_NO_SAT, val, 1, 1);
  *(int __local *)dest = val;
}

// CHECK-NOT: add.i32
