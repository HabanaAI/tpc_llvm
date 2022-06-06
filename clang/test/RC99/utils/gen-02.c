// RUN:%intr-gen %s | FileCheck %s

#if defined(__dali__) || defined(__gaudi__)
int func_01(int x);
#endif

#if !defined(__dali__)
int func_02(int x);
#endif

#if defined(__gaudi_plus__) && !defined(__gaudi2__)
int func_03(int x);
#endif

#if !(defined(__gaudi__) || defined(__goya2__))
int func_04(int x);
#endif

// CHECK: TARGET_BUILTIN( func_01, "ii", "nc", "dali|gaudi" )
// CHECK: TARGET_BUILTIN( func_02, "ii", "nc", "gaudi|gaudib|goya2|gaudi2" )
// CHECK: TARGET_BUILTIN( func_03, "ii", "nc", "gaudi|gaudib|goya2" )
// CHECK: TARGET_BUILTIN( func_04, "ii", "nc", "dali|gaudib|gaudi2" )
