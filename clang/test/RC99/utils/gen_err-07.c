// RUN: not %intr-gen %s 2>&1 | FileCheck %s

int func_01(bool polarity=1);

// CHECK: line 3 error: Default argument of 'polarity' must be 0
// CHECK: >  int func_01(bool polarity=1);
