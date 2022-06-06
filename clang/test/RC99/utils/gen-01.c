// RUN: %intr-gen %s | FileCheck %s

int func_00(int x);

#if defined(__dali__)
int func_01(int x);
#endif

#if defined(__gaudi__)
int func_02(int x);
#endif

#if defined(__gaudib__)
int func_03(int x);
#endif

#if defined(__goya2__)
int func_04(int x);
#endif

#if defined(__gaudi2__)
int func_05(int x);
#endif

#if defined(__gaudi_plus__)
int func_10(int x);
#endif

#if defined(__goya2_plus__)
int func_11(int x);
#endif

#if defined(__gaudi2_plus__)
int func_12(int x);
#endif


// CHECK: TARGET_BUILTIN( func_00, "ii", "nc", "dali|gaudi|gaudib|goya2|gaudi2" )
// CHECK: TARGET_BUILTIN( func_01, "ii", "nc", "dali" )
// CHECK: TARGET_BUILTIN( func_02, "ii", "nc", "gaudi" )
// CHECK: TARGET_BUILTIN( func_03, "ii", "nc", "gaudib" )
// CHECK: TARGET_BUILTIN( func_04, "ii", "nc", "goya2" )
// CHECK: TARGET_BUILTIN( func_05, "ii", "nc", "gaudi2" )

// CHECK: TARGET_BUILTIN( func_10, "ii", "nc", "gaudi|gaudib|goya2|gaudi2" )
// CHECK: TARGET_BUILTIN( func_11, "ii", "nc", "goya2|gaudi2" )
// CHECK: TARGET_BUILTIN( func_12, "ii", "nc", "gaudi2" )
