// RUN: %codegen -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck %s

void main(int src, int pred) {
  int5 ndx = { 0, 0, 0, 0, 0 };
  int64 val = src;
  i32_st_tnsr_i_v_b(ndx, 0, val, pred, 0);
}

// CHECK: st_tnsr {{.*}}, %SP{{[0-9]+}}
