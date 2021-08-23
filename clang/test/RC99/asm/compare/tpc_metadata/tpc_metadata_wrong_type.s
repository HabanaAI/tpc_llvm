// RUN: not %clang -cc1as -triple tpc-none-none -filetype obj %s -o /dev/null 2>&1 1>/dev/null | FileCheck %s

.TPC_METADATA
specialFunctionUsed:	WRONG_TYPE	3
// CHECK: [[@LINE-1]]:22: error: Unexpected token during parse tpc metadata: 'WRONG_TYPE'. Expected one of 'DT', 'DD', 'DH', 'DW', 'DQ', 'DB'.

NOP;
