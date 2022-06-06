// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int src) {
  int64 val = src;
  int5 storeCoord = { 11, 11, 22, 22, 22 };
  v_i32_st_tnsr(storeCoord, 1, val, 0, 1, 0);
}

// CHECK: set_indx [[REGI:%I[0-9]+]], b00011, 0xb
// CHECK: set_indx [[REGI]], b11100, 0x16
// CHECK: st_tnsr 0x1, [[REGI]], %V{{[0-9]+}}
