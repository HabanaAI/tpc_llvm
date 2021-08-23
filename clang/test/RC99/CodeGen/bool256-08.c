// R U N: %clang_cc1 -S -O0 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// R U N: %clang_cc1 -S -O0 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest) {
  bool256 __local *dptr = (bool256 __local*)dest;
  bool256 res = 1;

  *dptr = res;
}

// CHECK: st_l_v %S0,{{.*}} %VP0
