// RUN: %clang_cc1 -S -O0 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O0 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %clang_cc1 -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s

void main(int src1, int src2, int dest) {
  bool256 __local *dptr = (bool256 __local*)dest;
  bool256 __local *ptr1 = (bool256 __local*)src1;
  bool256 __local *ptr2 = (bool256 __local*)src2;
  bool256 x1 = *ptr1;
  bool256 x2 = *ptr2;
  *dptr = x1 ^ x2;
}

// CHECK-DAG: ld_l_v  {{%VP[0-9]+}}, %S0{{.*}}
// CHECK-DAG: ld_l_v  {{%VP[0-9]+}}, %S1{{.*}}
// CHECK:     xor.b   [[VPR3:%VP[0-9]+]], {{%VP[0-9]+}}, {{%VP[0-9]+}}
// CHECK:     st_l_v  %S2,{{.*}} [[VPR3]]
