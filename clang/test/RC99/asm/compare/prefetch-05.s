// RUN: %clang -cc1as -triple tpc-none-none -target-cpu doron1 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu doron1 %t.o | FileCheck %s

PREFETCH.L2 AD2; NOP; NOP; NOP 
PREFETCH.L2 AD2, SP1; NOP; NOP; NOP 
PREFETCH.L2 AD2, !SP1; NOP; NOP; NOP 

// CHECK: prefetch l2 AD2;        nop;    nop;    nop
// CHECK: prefetch l2 AD2, SP1;   nop;    nop;    nop
// CHECK: prefetch l2 AD2, !SP1;  nop;    nop;    nop

PREFETCH AD1, INC_0; NOP; NOP; NOP 
PREFETCH AD2, INC_1; NOP; NOP; NOP 
PREFETCH AD3, INC_2, SP1; NOP; NOP; NOP 
PREFETCH AD4, INC_4, !SP1; NOP; NOP; NOP 
PREFETCH AD5, INC_8; NOP; NOP; NOP 

// CHECK: prefetch  AD1;  nop;    nop;    nop
// CHECK: prefetch inc_1 AD2;     nop;    nop;    nop
// CHECK: prefetch inc_2 AD3, SP1;        nop;    nop;    nop
// CHECK: prefetch inc_4 AD4, !SP1;       nop;    nop;    nop
// CHECK: prefetch inc_8 AD5;     nop;    nop;    nop

