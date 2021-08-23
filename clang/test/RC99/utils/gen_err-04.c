// RUN: not %intr-gen %s 2>&1 | FileCheck %s

int func_01(int x=1);

// CHECK: line 3 error: Default argument of integer parameter 'x' must be 0
// CHECK: >  int func_01(int x=1);
