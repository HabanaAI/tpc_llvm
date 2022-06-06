// RUN: not %clang -cc1as -triple tpc-none-none -filetype obj %s -o /dev/null 2>&1 1>/dev/null | FileCheck %s

.tpc_metadata
version:				DD	5
march:					DW	2
// CHECK: [[@LINE-1]]:15: error: Specified TPC_METADATA architecture is different from actual target: goya

NOP;
