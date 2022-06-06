// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s

nop; udiv_4step.u32 DIV_STEP S4, S1; nop; nop
nop; udiv_4step.u16 DIV_STEP S4, S1, SP3; nop; nop
nop; udiv_4step.u8  DIV_STEP  S4, S1, !SP7; nop; nop

//CHECK: nop; 	udiv_4step.u32 DIV_STEP Z4, S1; 	nop; 	nop
//CHECK: nop; 	udiv_4step.u16 DIV_STEP Z4, S1, SP3; 	nop; 	nop
//CHECK: nop; 	udiv_4step.u8 DIV_STEP Z4, S1, !SP7; 	nop; 	nop

nop; udiv_4step.u32 DIV_STEP X2 S4, S1; nop; nop
nop; udiv_4step.u16 X2 DIV_STEP S4, S1, SP3; nop; nop
nop; udiv_4step.u8 X2 DIV_STEP S4, S1, !SP7; nop; nop

//CHECK: nop; 	udiv_4step.u32 x2 DIV_STEP Z4, S1; 	nop; 	nop
//CHECK: nop; 	udiv_4step.u16 x2 DIV_STEP Z4, S1, SP3; 	nop; 	nop
//CHECK: nop; 	udiv_4step.u8 x2 DIV_STEP Z4, S1, !SP7; 	nop; 	nop

nop; udiv_4step.u32 X2 10 Z4, S1; nop; nop
nop; udiv_4step.u16 x2 6 Z4, S1, SP3; nop; nop
nop; udiv_4step.u8 x2 25 Z4, S1, !SP7; nop; nop

//CHECK: nop; 	udiv_4step.u32 x2 0xa Z4, S1; 	nop; 	nop
//CHECK: nop; 	udiv_4step.u16 x2 0x6 Z4, S1, SP3; 	nop; 	nop
//CHECK: nop; 	udiv_4step.u8 x2 0x19 Z4, S1, !SP7; 	nop; 	nop

