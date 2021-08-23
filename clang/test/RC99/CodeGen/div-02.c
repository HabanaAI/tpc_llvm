// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int dest) {
  volatile unsigned __local *ptr = (unsigned __local *)dest;

  *ptr++ = *ptr / 4;
// CHECK: ld_l     [[SRC1:%S[0-9]+]], %S0
// CHECK: shr.i32  [[VAL1:%S[0-9]+]], [[SRC1]], 0x2
// CHECK: st_l     %S{{[0-9]+}}, [[VAL1]]

  *ptr++ = *ptr % 64;
// CHECK: and.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x3f
// CHECK: st_l

  volatile int __local *iptr = (int __local *)ptr;
  *iptr++ = *iptr / 4;
// CHECK: ash.i32
// CHECK: shr.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x1e
// CHECK: add.i32
// CHECK: ash.i32
// CHECK: st_l

  *iptr++ = *iptr % 64;
// CHECK: ash.i32
// CHECK: shr.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x1a
// CHECK: add.i32
// CHECK: and.i32  %S{{[0-9]+}}, %S{{[0-9]+}}, -0x40
// CHECK: sub.i32
// CHECK: st_l

}
