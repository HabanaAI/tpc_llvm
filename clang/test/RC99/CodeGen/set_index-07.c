// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int src, int src2) {
  int64 val = src;
  int5 storeCoord = { src2, 11, src2, 11, src2 };
  v_i32_st_tnsr(storeCoord, 1, val, 0, 1, 0);
}

// CHECK: set_indx [[REGI:%I[0-9]+]], b10101, %S1
// CHECK: set_indx [[REGI]], b01010, 0xb
// CHECK: st_tnsr 0x1, [[REGI]], %V{{[0-9]+}}
