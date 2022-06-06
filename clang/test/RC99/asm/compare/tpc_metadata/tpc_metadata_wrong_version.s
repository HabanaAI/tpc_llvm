// RUN: not %clang -cc1as -triple tpc-none-none -filetype obj %s -o /dev/null 2>&1 1>/dev/null | FileCheck %s

.tpc_metadata
version:				DD	10000
// CHECK: [[@LINE-1]]:16: error: Incorrect value. Expected value in range [0, 

NOP;
