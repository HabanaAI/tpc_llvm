// RUN: not %intr-gen %s 2>&1 | FileCheck %s

#if defined(__qqq__)
int func_01(int x);
#endif

// CHECK: line 3 column 14 error: Unknown define: __qqq__
// CHECK: >  #if defined(__qqq__)
// CHECK:                ^
