// RUN: %codegen -S -O0 -triple tpc-none-none -std=rc99 -mllvm -tpc-nearbyint-workaround=0  -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s
// RUN: %codegen -S -O0 -triple tpc-none-none -std=rc99 -mllvm -tpc-nearbyint-workaround=0  -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s

void main(int dest, int x, int vpredp, int pred) {
  volatile int __local *vptr = (int __local *)dest;
  minifloat mf = (minifloat)1;
  *vptr = sizeof(mf);
}
// CHECK-ASM: main:
// CHECK-ASM: mov.i32 [[REGS:%S[0-9]+]], 0x1
