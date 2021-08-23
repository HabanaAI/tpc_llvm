// RUN: not %clang -cc1as -triple tpc-none-none %s 2>&1 | FileCheck %s

nop; nop; ADD.CARRY S1, S2, S3
// CHECK: error: Suffix is not valid for the current architecture
