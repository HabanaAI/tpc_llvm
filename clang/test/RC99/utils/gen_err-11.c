// RUN: not %intr-gen %s 2>&1 | FileCheck %s

int func_01(bool);

// CHECK: line 3 error: missing parameter name
// CHECK: >  int func_01(bool);
