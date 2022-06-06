// RUN: not %clang -cc1as -triple tpc-none-none -filetype obj %s -o /dev/null 2>&1 1>/dev/null | FileCheck %s

.tpc_metadata
scalarld:				DH	0101001011100011
scalarLd[0]:			DB	1
// CHECK: [[@LINE-1]]:10: error: Redefintion of .tpc_metadata field.

NOP;
