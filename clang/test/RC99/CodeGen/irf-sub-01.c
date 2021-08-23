// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int src, int src1, int src2) {
  int64 val = src;
  int cnt = 0;
  int5 ndx1 = { src, src, src, src, src };

  while (cnt > src2) {
    i32_st_tnsr_i_v_b(ndx1, 1, val, 1, 0);

    ndx1[0] -= src;
  }
}

// CHECK: st_tnsr 0x1, [[NDX1:%I[0-9]+]], [[VREG:%V[0-9]+]]
// CHECK: sub.i32 b00001 [[NDX2:%I[0-9]+]], {{.*}}, [[NDX1]]
// CHECK: sub.i32 b00001 %I{{[0-9]+}}, 0x0, [[NDX2]]
