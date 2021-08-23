// RUN: %clang_cc1 -S -O0 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O0 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int dest, int x, int y) {
  bool256 __local *ptr = (bool256 __local*)dest;
  bool256 res = *ptr;
  *ptr = ~res;
}

// CHECK: ld_l_v  [[VPR1:%VP[0-9]+]], %S0,{{.*}} %SP0
// CHECK: not.b   [[VPR2:%VP[0-9]+]], [[VPR1]], %SP0
// CHECK: st_l_v  %S0,{{.*}} [[VPR2]], %SP0
