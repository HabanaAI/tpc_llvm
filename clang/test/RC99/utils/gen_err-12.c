// RUN: not %intr-gen %s 2>&1 | FileCheck %s

int func_01(polarity);

// CHECK: line 3 error: missing parameter type
// CHECK: >  int func_01(polarity);
