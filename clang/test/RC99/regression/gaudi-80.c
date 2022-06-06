// RUN: %clang_cc1 -S -emit-llvm -triple tpc-none-none -std=rc99 -O1 %s -o -
// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O1 %s -o - | FileCheck %s

volatile int a = 124089;
void main(tensor out) {
//for(int i=0;i<10;i++) {a=2*i*i;}
  int5 space = {0, 0, 0, 0, 0};
  volatile int t = a;//23534;
//  a = t;
//  v_i32_st_tnsr(out, space, a, 0, 1, 0);
}
// CHECK: mov.i32 [[A:%S[0-9]+]], 0x1e4b9
// CHECK: st_l  [[ADDR_A:0x[0-9]+]], [[A]]
// CHECK: ld_l  [[S:%S[0-9]+]], [[ADDR_A]]
// CHECK: st_l  [[ADDR_B:0x[0-9]+]], [[S]]
