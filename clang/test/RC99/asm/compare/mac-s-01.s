// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s


NOP; MAC.F32 S5, S1, S2
NOP; MAC.F32 S5, S1, S2, SP3
NOP; MAC.F32  S5, S1, S2, !SP3

// CHECK: 	nop; 	mac.f32  S5, S1, S2, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.f32  S5, S1, S2, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.f32  S5, S1, S2, !SP3; 	nop; 	nop


NOP; MAC.F32 S5, S2, 0x3f800000
NOP; MAC.F32 S5, S2, 0x3f800000, SP3
NOP; MAC.F32 S5, S2, 0x3f800000, !SP3

// CHECK: 	nop; 	mac.f32  S5, S2, 0x3f800000, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.f32  S5, S2, 0x3f800000, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.f32  S5, S2, 0x3f800000, !SP3; 	nop; 	nop


NOP; MAC.F32.NEG S5, S1, S2
NOP; MAC.F32.NEG S5, S1, S2, SP3
NOP; MAC.F32.NEG  S5, S1, S2, !SP3

// CHECK: 	nop; 	mac.f32  neg S5, S1, S2, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.f32  neg S5, S1, S2, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.f32  neg S5, S1, S2, !SP3; 	nop; 	nop


NOP; MAC.F32.NEG S5, S2, 0x3f800000
NOP; MAC.F32.NEG S5, S2, 0x3f800000, SP3
NOP; MAC.F32.NEG S5, S2, 0x3f800000, !SP3

// CHECK: 	nop; 	mac.f32  neg S5, S2, 0x3f800000, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.f32  neg S5, S2, 0x3f800000, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.f32  neg S5, S2, 0x3f800000, !SP3; 	nop; 	nop


NOP; MAC.I8 S5, S1, S2
NOP; MAC.I8 S5, S1, S2, SP3
NOP; MAC.I8 S5, S1, S2, !SP3

// CHECK: 	nop; 	mac.i8  S5, S1, S2, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.i8  S5, S1, S2, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.i8  S5, S1, S2, !SP3; 	nop; 	nop


NOP; MAC.I8 S5, S2, 123
NOP; MAC.I8 S5, S2, 123, SP3
NOP; MAC.I8 S5, S2, 123, !SP3

// CHECK: 	nop; 	mac.i8  S5, S2, 0x7b, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.i8  S5, S2, 0x7b, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.i8  S5, S2, 0x7b, !SP3; 	nop; 	nop

NOP; MAC.U8 S5, S1, S2
NOP; MAC.U8 S5, S1, S2, SP3
NOP; MAC.U8 S5, S1, S2, !SP3

// CHECK: 	nop; 	mac.u8  S5, S1, S2, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.u8  S5, S1, S2, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.u8  S5, S1, S2, !SP3; 	nop; 	nop


NOP; MAC.U8 S5, S2, 123
NOP; MAC.U8 S5, S2, 123, SP3
NOP; MAC.U8 S5, S2, 123, !SP3

// CHECK: 	nop; 	mac.u8  S5, S2, 0x7b, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.u8  S5, S2, 0x7b, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.u8  S5, S2, 0x7b, !SP3; 	nop; 	nop

NOP; MAC.I16 S5, S1, S2
NOP; MAC.I16 S5, S1, S2, SP3
NOP; MAC.I16 S5, S1, S2, !SP3

// CHECK: 	nop; 	mac.i16  S5, S1, S2, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.i16  S5, S1, S2, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.i16  S5, S1, S2, !SP3; 	nop; 	nop


NOP; MAC.I16 S5, S2, 123
NOP; MAC.I16 S5, S2, 123, SP3
NOP; MAC.I16 S5, S2, 123, !SP3

// CHECK: 	nop; 	mac.i16  S5, S2, 0x7b, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.i16  S5, S2, 0x7b, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.i16  S5, S2, 0x7b, !SP3; 	nop; 	nop

NOP; MAC.U16 S5, S1, S2
NOP; MAC.U16 S5, S1, S2, SP3
NOP; MAC.U16 S5, S1, S2, !SP3

// CHECK: 	nop; 	mac.u16  S5, S1, S2, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.u16  S5, S1, S2, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.u16  S5, S1, S2, !SP3; 	nop; 	nop


NOP; MAC.U16 S5, S2, 123
NOP; MAC.U16 S5, S2, 123, SP3
NOP; MAC.U16 S5, S2, 123, !SP3

// CHECK: 	nop; 	mac.u16  S5, S2, 0x7b, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.u16  S5, S2, 0x7b, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.u16  S5, S2, 0x7b, !SP3; 	nop; 	nop


NOP; MAC.I8.SAT S5, S1, S2
NOP; MAC.I8.SAT S5, S1, S2, SP3
NOP; MAC.I8.SAT S5, S1, S2, !SP3

// CHECK: 	nop; 	mac.i8  st S5, S1, S2, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.i8  st S5, S1, S2, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.i8  st S5, S1, S2, !SP3; 	nop; 	nop


NOP; MAC.I8.SAT S5, S2, 123
NOP; MAC.I8.SAT S5, S2, 123, SP3
NOP; MAC.I8.SAT S5, S2, 123, !SP3

// CHECK: 	nop; 	mac.i8  st S5, S2, 0x7b, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.i8  st S5, S2, 0x7b, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.i8  st S5, S2, 0x7b, !SP3; 	nop; 	nop

NOP; MAC.U8.SAT S5, S1, S2
NOP; MAC.U8.SAT S5, S1, S2, SP3
NOP; MAC.U8.SAT S5, S1, S2, !SP3

// CHECK: 	nop; 	mac.u8  st S5, S1, S2, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.u8  st S5, S1, S2, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.u8  st S5, S1, S2, !SP3; 	nop; 	nop


NOP; MAC.U8.SAT S5, S2, 123
NOP; MAC.U8.SAT S5, S2, 123, SP3
NOP; MAC.U8.SAT S5, S2, 123, !SP3

// CHECK: 	nop; 	mac.u8  st S5, S2, 0x7b, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.u8  st S5, S2, 0x7b, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.u8  st S5, S2, 0x7b, !SP3; 	nop; 	nop

NOP; MAC.I16.SAT S5, S1, S2
NOP; MAC.I16.SAT S5, S1, S2, SP3
NOP; MAC.I16.SAT S5, S1, S2, !SP3

// CHECK: 	nop; 	mac.i16  st S5, S1, S2, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.i16  st S5, S1, S2, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.i16  st S5, S1, S2, !SP3; 	nop; 	nop


NOP; MAC.I16.SAT S5, S2, 123
NOP; MAC.I16.SAT S5, S2, 123, SP3
NOP; MAC.I16.SAT S5, S2, 123, !SP3

// CHECK: 	nop; 	mac.i16  st S5, S2, 0x7b, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.i16  st S5, S2, 0x7b, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.i16  st S5, S2, 0x7b, !SP3; 	nop; 	nop

NOP; MAC.U16.SAT S5, S1, S2
NOP; MAC.U16.SAT S5, S1, S2, SP3
NOP; MAC.U16.SAT S5, S1, S2, !SP3

// CHECK: 	nop; 	mac.u16  st S5, S1, S2, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.u16  st S5, S1, S2, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.u16  st S5, S1, S2, !SP3; 	nop; 	nop


NOP; MAC.U16.SAT S5, S2, 123
NOP; MAC.U16.SAT S5, S2, 123, SP3
NOP; MAC.U16.SAT S5, S2, 123, !SP3

// CHECK: 	nop; 	mac.u16  st S5, S2, 0x7b, SP0; 	nop; 	nop
// CHECK: 	nop; 	mac.u16  st S5, S2, 0x7b, SP3; 	nop; 	nop
// CHECK: 	nop; 	mac.u16  st S5, S2, 0x7b, !SP3; 	nop; 	nop
