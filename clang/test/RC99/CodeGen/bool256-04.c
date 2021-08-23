// RUN: %clang_cc1 -S -O0 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O0 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest) {
  bool256 __local *dptr = (bool256 __local*)dest;
  bool256 res = 0;

  *dptr = res;
}

// CHECK: mov %VP[[REG:[0-9]+]], 0x0
// CHECK: st_l_v %S0,{{.*}} %VP[[REG]]
