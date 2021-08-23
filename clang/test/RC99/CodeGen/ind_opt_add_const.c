// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int src,int step) {
  int64 val = src;
  int5 storeCoord = { 0, 1, 2, 3, 4 };
  storeCoord[0]+=2;
  storeCoord[1]+=2;
  storeCoord[2]+=2;
  i32_st_tnsr_i_v_b(storeCoord, 1, val, 1, 0);
  storeCoord[3]+=2;
  storeCoord[4]+=2;
  i32_st_tnsr_i_v_b(storeCoord, 1, val, 1, 0);
}

// CHECK: add.i32  b00111 %I2, 0x2, %I2, %SP0
// CHECK: add.i32  b11000 %I2, 0x2, %I2, %SP0
