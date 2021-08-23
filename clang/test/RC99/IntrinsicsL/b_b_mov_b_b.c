// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck %s


void main(_Bool x0, _Bool x1, _Bool pred, int dest) {
  int __local *dptr = (int __local *) dest;
  _Bool res = pred;

  res = b_b_mov_b_b(x0, res, pred, 0);
  *dptr++ = res;
// CHECK: mov  [[RES:%SP[0-9]+]], %SP{{[0-9]+}}, %SP{{[0-9]+}}

  res = b_b_mov_b_b(1, res, pred, 0);
  *dptr++ = res;
// CHECK: mov  [[RES]], 0x1, %SP{{[0-9]+}}

  res = b_b_mov_b_b(0, res, pred, 0);
  *dptr++ = res;
// CHECK: mov  [[RES]], 0x0, %SP{{[0-9]+}}
}
