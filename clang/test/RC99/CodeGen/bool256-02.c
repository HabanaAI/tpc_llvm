// RUN: %clang_cc1 -S -O0 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O0 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest, int src) {
  bool256 __local *dptr = (bool256 __local*)dest;
  bool256 __local *sptr = (bool256 __local*)src;
  bool256 res;

  res = *sptr;
  *dptr = res;
}

// CHECK: ld_l_v %VP[[REG:[0-9]+]], %S1
// CHECK: st_l_v %S0,{{.*}} %VP[[REG]]
