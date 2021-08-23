// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

// GAUDI-1064 : LLVM-284 Not accept small character in Reg name & detales for error: Invalid operand in the instruction 'ANY_instruction with reg name in small character'

mov V26, V19, SP0; nop; add.i32 st v18, V32, V27, SP0; nop
// CHECK: mov  V26, V19, SP0; nop; add.i32 st V18, V32, V27, SP0;  nop
