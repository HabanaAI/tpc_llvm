// RUN: not %intr-gen %s 2>&1 | FileCheck %s

#if defined(__dali__)
#if defined(__gaudi__)
int func_01(int x);
#endif
#endif

// CHECK: line 4 column 3 error: nested 'if' directives are not supported
// CHECK: >  #if defined(__gaudi__)
// CHECK:     ^
// CHECK: previous construct is at line 2
// CHECK: >  #if defined(__dali__)