// RUN: not %intr-gen %s 2>&1 | FileCheck %s

int func_01(int switches=0, bool polarity);

// CHECK: line 3 error: Missed default argument in parameter 'polarity'
// CHECK: >  int func_01(int switches=0, bool polarity);
