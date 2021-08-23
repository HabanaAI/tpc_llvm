// RUN: not %intr-gen %s 2>&1 | FileCheck %s

int func_01(float128 x=4);

// CHECK: line 3 error: Invalid default argument of parameter 'x'
// CHECK: >  int func_01(float128 x=4);
