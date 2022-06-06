// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int src,int step) {
  int64 val = src;
  int5 storeCoord = { 0, 1, 2, 3, 4 };
  storeCoord[0]+=step;
  storeCoord[1]+=step;
  storeCoord[2]+=step;
  v_i32_st_tnsr(storeCoord, 1, val, 0, 1, 0);
  storeCoord[3]+=step;
  storeCoord[4]+=step;
  v_i32_st_tnsr(storeCoord, 1, val, 0, 1, 0);
}

// CHECK: add.i32  b00111 [[NDX1:%I[0-9]+]], %S1, %I{{[0-9]+}}
// CHECK: add.i32  b11000 %I{{[0-9]+}}, %S1, [[NDX1]]
