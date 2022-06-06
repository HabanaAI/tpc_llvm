// RUN: %clang_cc1 -triple tpc-none-none -std=rc99  -target-cpu gaudi2 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99  -target-cpu doron1 -S -O1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int dest, minihalf x0, int x1) {
  minihalf __local *res = (minihalf __local *)dest;
  *res = s_h8_abs(x0, 0, 0, 1, 0);
}

//CHECK-ASM: .globl main
//CHECK-ASM: main:
//CHECK-ASM: and.f8_152 %S{{[0-9]+}}, %S{{[0-9]+}}, 0x7f
