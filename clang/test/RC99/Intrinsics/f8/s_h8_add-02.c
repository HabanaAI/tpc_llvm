// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -float8 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -float8 -target-cpu doron1 %s -o - | FileCheck %s

void main(float x0, minihalf x1, int dest) {
  float __local *dest_ptr = (float  __local *)dest;
  float res = 0.0;

  res = s_h8_add(res, (minihalf)8.0, 0, res, 1, 0);
  *dest_ptr = res;
// CHECK: add.f8_152 [[RES:%S[0-9]+]], [[SRC:%S[0-9]+]], 0x48
// CHECK: st_l       [[DEST:%S[0-9]+]], [[RES]]
}
