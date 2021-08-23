// RUN: not %intr-gen %s 2>&1 | FileCheck %s

int func_01(bool predicate=0);

// CHECK: line 3 error: Default argument of 'predicate' must be 1
// CHECK: >  int func_01(bool predicate=0);
