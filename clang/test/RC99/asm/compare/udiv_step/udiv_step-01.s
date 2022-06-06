// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

nop; udiv_step.u32 0 S4, S1; nop; nop
nop; udiv_step.u16 1 S4, S1, SP1; nop; nop
nop; udiv_step.u8 10 S4, S1, !SP1; nop; nop

//CHECK: nop; 	udiv_step.u32 0x0 Z4, S1; 	nop; 	nop
//CHECK: nop; 	udiv_step.u16 0x1 Z4, S1, SP1; 	nop; 	nop
//CHECK: nop; 	udiv_step.u8 0xa Z4, S1, !SP1; 	nop; 	nop

nop; udiv_step.u32 20 Z4, S1; nop; nop
nop; udiv_step.u16 22 Z2, S1, SP1; nop; nop
nop; udiv_step.u8 24 Z0, S1, !SP1; nop; nop

//CHECK: nop; 	udiv_step.u32 0x14 Z4, S1; 	nop; 	nop
//CHECK: nop; 	udiv_step.u16 0x16 Z2, S1, SP1; 	nop; 	nop
//CHECK: nop; 	udiv_step.u8 0x18 Z0, S1, !SP1; 	nop; 	nop
