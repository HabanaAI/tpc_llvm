// RUN: not %intr-gen %s 2>&1 | FileCheck %s

#ifdef __dali__
int func_01(int x);
#endif

// CHECK: line 3 column 3 error: unsupported preprocessor directive
// CHECK: >  #ifdef __dali__
// CHECK:     ^
