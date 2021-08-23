// RUN: not %clang -cc1as -triple tpc-none-none %s 2>&1 | FileCheck %s

nop; nop; mov_g V1, V2, g_en=4, dg_en=15, 32; nop
// CHECK: Unexpected group description.
 
