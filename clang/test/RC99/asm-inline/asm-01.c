// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O1 %s -o - | FileCheck %s
// RUN: %clang_cc1 -emit-obj -triple tpc-none-none -std=rc99 -O1 %s -o /dev/null

void main(int x) {
  register int result __asm__("s11");
  __asm volatile ("nop; add.f32 %0, S2, S3" :"=s" (result)::);
}
// CHECK-LABEL: main:
// CHECK:    //APP
// CHECK-NEXT:    nop; add.f32 S11, S2, S3
// CHECK-NEXT:    //NO_APP
