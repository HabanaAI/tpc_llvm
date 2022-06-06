// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int src, int src1, int src2) {
  int64 val = src;
  int cnt = 0;
  int5 ndx1 = { src, src, src, src, src };

  while (cnt < src2) {
    v_i32_st_tnsr(ndx1, 1, val, 0, 1, 0);
    ndx1[0] &= src;
    cnt++;
  }
}

// CHECK: st_tnsr 0x1, [[NDX:%I[0-9]+]], %V{{[0-9]+}}
// CHECK: and.i32 b00001 [[NDX]], %S0, [[NDX]]
