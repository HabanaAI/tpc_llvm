// RUN: not %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o /dev/null 2>&1 | FileCheck %s

ld_l_v V1, S2, S3; nop; nop; nop
// CHECK: Missing feature
