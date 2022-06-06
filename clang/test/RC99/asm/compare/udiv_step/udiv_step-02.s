// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s

nop; udiv_4step.u32 5 S4, S1; nop; nop
nop; udiv_4step.u16 6 S4, S1, SP1; nop; nop
nop; udiv_4step.u8 10 S4, S1, !SP1; nop; nop

//CHECK:nop; 	udiv_4step.u32  0x5 Z4, S1; 	nop; 	nop
//CHECK: nop; 	udiv_4step.u16  0x6 Z4, S1, SP1; 	nop; 	nop
//CHECK: nop; 	udiv_4step.u8  0xa Z4, S1, !SP1; 	nop; 	nop

nop; udiv_4step.u32 20 Z4, S1; nop; nop
nop; udiv_4step.u16 22 Z2, S1, SP1; nop; nop
nop; udiv_4step.u8 24 Z0, S1, !SP1; nop; nop

//CHECK: nop; 	udiv_4step.u32  0x14 Z4, S1; 	nop; 	nop
//CHECK: nop; 	udiv_4step.u16  0x16 Z2, S1, SP1; 	nop; 	nop
//CHECK: nop; 	udiv_4step.u8  0x18 Z0, S1, !SP1; 	nop; 	nop
