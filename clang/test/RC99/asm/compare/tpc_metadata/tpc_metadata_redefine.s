// RUN: not %clang -cc1as -triple tpc-none-none -filetype obj %s -o /dev/null 2>&1 1>/dev/null | FileCheck %s

.tpc_metadata
specialFunctionUsed:	DB	1
specialFunctionUsed:	DB	0
// CHECK: [[@LINE-1]]:1: error: Redefintion of .tpc_metadata field: specialFunctionUsed

NOP;
