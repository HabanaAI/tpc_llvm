// RUN: not %clang -cc1as -triple tpc-none-none -filetype obj %s -o /dev/null 2>&1 >/dev/null | FileCheck %s

// Index 16 is one past the last tensor; the assembler used to index a
// 16-bit set with it and assert.
.tpc_metadata
scalarLd[16]:			DB	1
// CHECK: [[@LINE-1]]:10: error: Tensor index out of range, expected from 0 to 15.

NOP;
