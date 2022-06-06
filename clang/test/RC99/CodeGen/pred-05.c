// RUN: %codegen -triple tpc-none-none -std=rc99 -S -O1 %s -o - | FileCheck %s

void main(int src, int pred) {
  int5 ndx = { 0, 0, 0, 0, 0 };
  int64 val = src;
  v_i32_st_tnsr(ndx, 0, val, 0, pred, 0);
}

// CHECK: st_tnsr {{.*}}, %SP{{[0-9]+}}
