// RUN: %intr-gen --def %s | FileCheck %s

int func_01(int x, bool polarity);
int func_02(int x, bool polarity=0);
int func_03(int x, bool predicate, bool polarity=0);
int func_04(int x, bool predicate=1, bool polarity=0);

// CHECK: TPC_BUILTIN( func_01, "iib", "nc", "dali|gaudi|gaudib|goya2|gaudi2", -1 )
// CHECK: TPC_BUILTIN( func_02, "iib", "nc", "dali|gaudi|gaudib|goya2|gaudi2", 1 )
// CHECK: TPC_BUILTIN( func_03, "iibb", "nc", "dali|gaudi|gaudib|goya2|gaudi2", 2 )
// CHECK: TPC_BUILTIN( func_04, "iibb", "nc", "dali|gaudi|gaudib|goya2|gaudi2", 1 )
