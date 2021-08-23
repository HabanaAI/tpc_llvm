// RUN: not %intr-gen %s 2>&1 | FileCheck %s

int func_01(int a, int b,,bool polarity);

// CHECK: line 3 error: Invalid parameter declaration
// CHECK: >  int func_01(int a, int b,,bool polarity);
