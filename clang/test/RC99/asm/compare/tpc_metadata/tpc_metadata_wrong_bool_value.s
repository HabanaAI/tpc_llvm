// RUN: not %clang -cc1as -triple tpc-none-none -filetype obj %s -o /dev/null 2>&1 1>/dev/null | FileCheck %s

.tpc_metadata
specialFunctionUsed:	DB	3
// CHECK: [[@LINE-1]]:25: error: Incorrect value. Expected value in range [0, 1].

NOP;
