// RUN: %clang_cc1 -triple tpc-none-none -std=rc99  -target-cpu gaudi2 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99  -target-cpu doron1 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int dest, minifloat x0, int x1) {
  minifloat __local *res = (minifloat __local *)dest;
  *res = s_f8_abs(x0, 0, 0, 1, 0);
}

//CHECK-ASM: .globl main
//CHECK-ASM: main:
//CHECK-ASM: and.f8_143 %S{{[0-9]+}}, %S{{[0-9]+}}, 0x7f
