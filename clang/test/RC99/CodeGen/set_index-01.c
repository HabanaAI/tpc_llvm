// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O0 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int src) {
  int64 val = src;
  int5 storeCoord = { 0, 1, 2, 3, 4 };
  v_i32_st_tnsr(storeCoord, 1, val, 0, 1, 0);
}

// CHECK: set_indx [[REGI:%I[0-9]+]], b00001, 0x0
// CHECK: set_indx [[REGI]], b00010, 0x1
// CHECK: set_indx [[REGI]], b00100, 0x2
// CHECK: set_indx [[REGI]], b01000, 0x3
// CHECK: set_indx [[REGI]], b10000, 0x4
// CHECK: st_tnsr  0x1, [[REGI]], %V{{[0-9]+}}
