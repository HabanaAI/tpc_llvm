// RUN: %codegen -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck %s

void main(int src) {
  int5 ndx = { 0, 0, 0, 0, 0 };
  int64 val = src;
  val = v_i32_add_v_s_b(val, 22, val, 0, 0, 0);
  i32_st_tnsr_i_v_b(ndx, 0, val, 1, 0);
}

// CHECK-NOT: add.i32
