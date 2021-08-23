// RUN: %clang_cc1 -S -O0 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O0 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int src, int dest) {
  bool256 __local *dptr = (bool256 __local*)dest;
  bool256 __local *ptr  = (bool256 __local*)src;
  bool256 x = *ptr;
  *dptr ^= x;
}

// CHECK-DAG: ld_l_v  {{%VP[0-9]+}}, %S0,{{.*}} %SP0
// CHECK-DAG: ld_l_v  {{%VP[0-9]+}}, %S1,{{.*}} %SP0
// CHECK:     xor.b   [[VPR3:%VP[0-9]+]], {{%VP[0-9]+}}, {{%VP[0-9]+}}, %SP0
// CHECK:     st_l_v  %S1,{{.*}} [[VPR3]], %SP0
