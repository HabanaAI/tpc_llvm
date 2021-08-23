// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s

void main(int dest, int src, unsigned short tanhIntervalShift) {
  short128 absX = *(short128 __local *)src;
  short128 interval = absX >> tanhIntervalShift;
  *(short128 __local *)dest = interval;
}

// CHECK:      ld_l_v %V0, %S1, 0x0
// CHECK-NEXT: mov.i16	%S1, 0x0
// CHECK-NEXT: nop
// CHECK-NEXT: nop
// CHECK-NEXT: nop
// CHECK-NEXT: sub.i16  %S1, %S1, %S2
// CHECK-NEXT: ash.i16  %V0, %V0, %S1
// CHECK-NEXT: nop
// CHECK-NEXT: nop
// CHECK-NEXT: nop
// CHECK-NEXT: nop
// CHECK-NEXT: nop
// CHECK-NEXT: nop
// CHECK-NEXT: st_l_v %S0, 0x0, %V0
