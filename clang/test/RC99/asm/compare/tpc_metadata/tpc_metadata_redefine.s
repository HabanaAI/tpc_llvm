// RUN: not %clang -cc1as -triple tpc-none-none -filetype obj %s -o /dev/null 2>&1 1>/dev/null | FileCheck %s

.TPC_METADATA
specialFunctionUsed:	DB	1
specialFunctionUsed:	DB	0
// CHECK: [[@LINE-1]]:1: error: Redefintion of .TPC_METADATA field: specialFunctionUsed

NOP;
