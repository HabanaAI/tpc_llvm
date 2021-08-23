// RUN: not %intr-gen %s 2>&1 | FileCheck %s

int func_01(bool x=4);

// CHECK: line 3 error: Default argument of boolean parameter 'x' must be 1 or 0
// CHECK: >  int func_01(bool x=4);
