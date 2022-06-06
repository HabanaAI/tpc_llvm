// RUN: %clang -cc1as -triple tpc-none-none -target-cpu greco -filetype obj %s -o %t.o
// RUN: %disasm --mcpu greco %t.o | FileCheck %s


NOP; NOP; FCLASS.F16 V6, V7;       NOP;

// CHECK: nop; nop; fclass.f16  V6, V7; nop
