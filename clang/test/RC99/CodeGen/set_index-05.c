// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int src, int src2) {
  int64 val = src;
  int5 storeCoord = { src2, src2, src2, 11, 11 };
  i32_st_tnsr_i_v_b(storeCoord, 1, val, 1, 0);
}

// CHECK: set_indx [[REGI:%I[0-9]+]], b00111, %S1, %SP0
// CHECK: set_indx [[REGI]], b11000, 0xb, %SP0
// CHECK: st_tnsr 0x1, [[REGI]], %V{{[0-9]+}}
