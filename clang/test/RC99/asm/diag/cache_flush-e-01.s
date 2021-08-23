// RUN: not %clang -cc1as -triple tpc-none-none -filetype obj %s 2>&1 | FileCheck %s

nop; nop; nop; CACHE_FLUSH VP0
// CHECK: error: Invalid operand in the instruction 'cache_flush'
