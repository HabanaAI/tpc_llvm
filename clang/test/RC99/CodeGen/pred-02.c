// RUN: %codegen -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck %s

void main(int src) {
  int5 ndx = { 0, 0, 0, 0, 0 };
  int64 val = src;
  val = v_i32_add_b(val, 22, 0, val, 1, 1);
  v_i32_st_tnsr(ndx, 0, val, 0, 1, 0);
}

// CHECK-NOT: add.i32
