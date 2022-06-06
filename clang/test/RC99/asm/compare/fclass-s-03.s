// RUN: %clang -cc1as -triple tpc-none-none -target-cpu greco -filetype obj %s -o %t.o
// RUN: %disasm --mcpu greco %t.o | FileCheck %s

NOP; FCLASS.F16 S6, S7

// CHECK: nop; fclass.f16  S6, S7; nop; nop
