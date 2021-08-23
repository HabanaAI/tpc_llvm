// RUN: %tpc_clang -S -O0 -o - %s | FileCheck %s

void main(int dest, int src1, int src2) {
  int __local *ptr = (int __local *)dest;
  *ptr = src1 + src2;
}

// CHECK:      add.i32 %S
// CHECK-NEXT: nop
// CHECK-NEXT: nop
// CHECK-NEXT: nop
// CHECK-NEXT: nop
// CHECK-NEXT: nop
// CHECK-NEXT: nop
// CHECK-NEXT: nop
// CHECK-NEXT: nop

