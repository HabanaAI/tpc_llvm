// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(float x0, minifloat x1, int dest) {
  float __local *dest_ptr = (float  __local *)dest;
  float res = 0.0;

  res = s_f8_add(res, (minifloat)8.0, 0, res, 1, 0);
  *dest_ptr = res;
// CHECK: add.f8_143 [[RES:%S[0-9]+]], [[SRC:%S[0-9]+]], 0x50
// CHECK: st_l       [[DEST:%S[0-9]+]], [[RES]]
}
