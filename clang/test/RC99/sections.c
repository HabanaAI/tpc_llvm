// RUN: %tpc_clang -c %s -o %t.o
// RUN: llvm-objdump -triple=tpc -s %t.o | FileCheck %s

//GAUDI-1366
// XFAIL:*

void main(int dest, float val) {
  float __local *ptr = (float __local *)dest;
  *ptr = val + 2;
}

// CHECK: Contents of section .llvmir:
// CHECK: Contents of section .source:
// CHECK: void main
