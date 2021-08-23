// RUN:%intr-gen %s | FileCheck %s

#if defined(__dali__) || defined(__gaudi__)
int func_01(int x);
#endif

#if !defined(__dali__)
int func_02(int x);
#endif


// CHECK: TARGET_BUILTIN( func_01, "ii", "nc", "dali|gaudi" )
// CHECK: TARGET_BUILTIN( func_02, "ii", "nc", "goya" )
