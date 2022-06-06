// RUN: %clang_cc1 -S -O0 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O0 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest, int x, int y) {
  bool256 __local *dptr = (bool256 __local*)dest;
  bool256 res = x < y;

  *dptr = res;
}

// CHECK: cmp_less.i32  [[SPREG:%SP[0-9]+]], %S1, %S2
// CHECK: mov           [[VPREG:%VP[0-9]+]], [[SPREG]]
// CHECK: st_l_v        %S0,{{.*}} [[VPREG]]
