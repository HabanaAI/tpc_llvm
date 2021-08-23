// RUN: %intr-gen %s | FileCheck %s

int func_00(int x);

#if defined(__dali__)
int func_01(int x);
#endif

#if defined(__gaudi__)
int func_02(int x);
#endif



// CHECK: TARGET_BUILTIN( func_00, "ii", "nc", "dali|gaudi" )
// CHECK: TARGET_BUILTIN( func_01, "ii", "nc", "dali" )
// CHECK: TARGET_BUILTIN( func_02, "ii", "nc", "gaudi" )
