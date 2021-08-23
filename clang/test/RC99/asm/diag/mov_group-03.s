// RUN: not %clang -cc1as -triple tpc-none-none %s 2>&1 | FileCheck %s

nop; nop; mov_group V1, V2, group_en=4, dual_group_en=15, 32; nop
// CHECK: Unexpected group description.
