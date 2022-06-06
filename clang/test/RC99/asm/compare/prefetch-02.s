// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s

PREFETCH AD2; NOP; NOP; NOP 
PREFETCH AD2, SP1; NOP; NOP; NOP 
PREFETCH AD2, !SP1; NOP; NOP; NOP 

// CHECK: prefetch  AD2;     nop;    nop;    nop
// CHECK: prefetch  AD2, SP1;     nop;    nop;    nop
// CHECK: prefetch  AD2, !SP1;    nop;    nop;    nop