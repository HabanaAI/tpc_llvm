// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int dest, int x0, int x1) {
  minifloat256 __local *res = (minifloat256 __local *)dest;
  minifloat256 __local *ptr_x0 = (minifloat256 __local *)x0;

  *res = v_f8_abs_b(*ptr_x0, 0,0,1,0);
}

//CHECK-ASM: .globl main
//CHECK-ASM: main:
//CHECK-ASM: form_fp_num.f8_143 force_sign0 %V{{[0-9]+}}, %[[SRC:V[0-9]+]], %[[SRC]], %[[SRC]]
