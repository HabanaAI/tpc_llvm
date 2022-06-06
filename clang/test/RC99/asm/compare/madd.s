// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s

NOP; NOP; MADD.F32 V5, V1, V2, V4
NOP; NOP; MADD.F32 V5, V1, V2, V4, SP3
NOP; NOP; MADD.F32 V5, V1, V2, V4, !SP3
NOP; NOP; MADD.F32 V5, V1, V2, V4, VP3
NOP; NOP; MADD.F32 V5, V1, V2, V4, !VP3

// CHECK: 	nop; 	nop; 	madd.f32  V5, V1, V2, V4; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  V5, V1, V2, V4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  V5, V1, V2, V4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  V5, V1, V2, V4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  V5, V1, V2, V4, !VP3; 	nop

NOP; NOP; MADD.F32 V5, V1, S2, V4
NOP; NOP; MADD.F32 V5, V1, S2, V4, SP3
NOP; NOP; MADD.F32 V5, V1, S2, V4, !SP3
NOP; NOP; MADD.F32 V5, V1, S2, V4, VP3
NOP; NOP; MADD.F32 V5, V1, S2, V4, !VP3

// CHECK: 	nop; 	nop; 	madd.f32  V5, V1, S2, V4; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  V5, V1, S2, V4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  V5, V1, S2, V4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  V5, V1, S2, V4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  V5, V1, S2, V4, !VP3; 	nop

NOP; NOP; MADD.F32 V5, V1, 0X3F800000, V4
NOP; NOP; MADD.F32 V5, V1, 0X3F800000, V4, SP3
NOP; NOP; MADD.F32 V5, V1, 0X3F800000, V4, !SP3
NOP; NOP; MADD.F32 V5, V1, 0X3F800000, V4, VP3
NOP; NOP; MADD.F32 V5, V1, 0X3F800000, V4, !VP3

// CHECK: 	nop; 	nop; 	madd.f32  V5, V1, 0x3f800000, V4; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  V5, V1, 0x3f800000, V4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  V5, V1, 0x3f800000, V4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  V5, V1, 0x3f800000, V4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  V5, V1, 0x3f800000, V4, !VP3; 	nop


NOP; NOP; MADD.F32.NEG V5, V1, V2, V4
NOP; NOP; MADD.F32.NEG V5, V1, V2, V4, SP3
NOP; NOP; MADD.F32.NEG V5, V1, V2, V4, !SP3
NOP; NOP; MADD.F32.NEG V5, V1, V2, V4, VP3
NOP; NOP; MADD.F32.NEG V5, V1, V2, V4, !VP3

// CHECK: 	nop; 	nop; 	madd.f32  neg V5, V1, V2, V4; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  neg V5, V1, V2, V4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  neg V5, V1, V2, V4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  neg V5, V1, V2, V4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  neg V5, V1, V2, V4, !VP3; 	nop

NOP; NOP; MADD.F32.NEG V5, V1, S2, V4
NOP; NOP; MADD.F32.NEG V5, V1, S2, V4, SP3
NOP; NOP; MADD.F32.NEG V5, V1, S2, V4, !SP3
NOP; NOP; MADD.F32.NEG V5, V1, S2, V4, VP3
NOP; NOP; MADD.F32.NEG V5, V1, S2, V4, !VP3

// CHECK: 	nop; 	nop; 	madd.f32  neg V5, V1, S2, V4; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  neg V5, V1, S2, V4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  neg V5, V1, S2, V4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  neg V5, V1, S2, V4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  neg V5, V1, S2, V4, !VP3; 	nop

NOP; NOP; MADD.F32.NEG V5, V1, 0X3F800000, V4
NOP; NOP; MADD.F32.NEG V5, V1, 0X3F800000, V4, SP3
NOP; NOP; MADD.F32.NEG V5, V1, 0X3F800000, V4, !SP3
NOP; NOP; MADD.F32.NEG V5, V1, 0X3F800000, V4, VP3
NOP; NOP; MADD.F32.NEG V5, V1, 0X3F800000, V4, !VP3

// CHECK: 	nop; 	nop; 	madd.f32  neg V5, V1, 0x3f800000, V4; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  neg V5, V1, 0x3f800000, V4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  neg V5, V1, 0x3f800000, V4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  neg V5, V1, 0x3f800000, V4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f32  neg V5, V1, 0x3f800000, V4, !VP3; 	nop


NOP; NOP; MADD.I16 D8, V1, V2, D4
NOP; NOP; MADD.I16 D8, V1, V2, D4, SP3
NOP; NOP; MADD.I16 D8, V1, V2, D4, !SP3
NOP; NOP; MADD.I16 D8, V1, V2, D4, VP3
NOP; NOP; MADD.I16 D8, V1, V2, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i16  D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  D8, V1, V2, D4, !VP3; 	nop

NOP; NOP; MADD.I16 D8, V1, S2, D4
NOP; NOP; MADD.I16 D8, V1, S2, D4, SP3
NOP; NOP; MADD.I16 D8, V1, S2, D4, !SP3
NOP; NOP; MADD.I16 D8, V1, S2, D4, VP3
NOP; NOP; MADD.I16 D8, V1, S2, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i16  D8, V1, S2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  D8, V1, S2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  D8, V1, S2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  D8, V1, S2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  D8, V1, S2, D4, !VP3; 	nop

NOP; NOP; MADD.I16 D8, V1, 123, D4
NOP; NOP; MADD.I16 D8, V1, 123, D4, SP3
NOP; NOP; MADD.I16 D8, V1, 123, D4, !SP3
NOP; NOP; MADD.I16 D8, V1, 123, D4, VP3
NOP; NOP; MADD.I16 D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i16  D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.I16.SAT D8, V1, V2, D4
NOP; NOP; MADD.I16.SAT D8, V1, V2, D4, SP3
NOP; NOP; MADD.I16.SAT D8, V1, V2, D4, !SP3
NOP; NOP; MADD.I16.SAT D8, V1, V2, D4, VP3
NOP; NOP; MADD.I16.SAT D8, V1, V2, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i16  st D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st D8, V1, V2, D4, !VP3; 	nop

NOP; NOP; MADD.I16.SAT D8, V1, S2, D4
NOP; NOP; MADD.I16.SAT D8, V1, S2, D4, SP3
NOP; NOP; MADD.I16.SAT D8, V1, S2, D4, !SP3
NOP; NOP; MADD.I16.SAT D8, V1, S2, D4, VP3
NOP; NOP; MADD.I16.SAT D8, V1, S2, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i16  st D8, V1, S2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st D8, V1, S2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st D8, V1, S2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st D8, V1, S2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st D8, V1, S2, D4, !VP3; 	nop


NOP; NOP; MADD.I16.SAT D8, V1, 123, D4
NOP; NOP; MADD.I16.SAT D8, V1, 123, D4, SP3
NOP; NOP; MADD.I16.SAT D8, V1, 123, D4, !SP3
NOP; NOP; MADD.I16.SAT D8, V1, 123, D4, VP3
NOP; NOP; MADD.I16.SAT D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i16  st D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st D8, V1, 0x7b, D4, !VP3; 	nop

NOP; NOP; MADD.U16 D8, V1, V2, D4
NOP; NOP; MADD.U16 D8, V1, V2, D4, SP3
NOP; NOP; MADD.U16 D8, V1, V2, D4, !SP3
NOP; NOP; MADD.U16 D8, V1, V2, D4, VP3
NOP; NOP; MADD.U16 D8, V1, V2, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.u16  D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  D8, V1, V2, D4, !VP3; 	nop


NOP; NOP; MADD.U16 D8, V1, 123, D4
NOP; NOP; MADD.U16 D8, V1, 123, D4, SP3
NOP; NOP; MADD.U16 D8, V1, 123, D4, !SP3
NOP; NOP; MADD.U16 D8, V1, 123, D4, VP3
NOP; NOP; MADD.U16 D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.u16  D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.U16.SAT D8, V1, V2, D4
NOP; NOP; MADD.U16.SAT D8, V1, V2, D4, SP3
NOP; NOP; MADD.U16.SAT D8, V1, V2, D4, !SP3
NOP; NOP; MADD.U16.SAT D8, V1, V2, D4, VP3
NOP; NOP; MADD.U16.SAT D8, V1, V2, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.u16  st D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st D8, V1, V2, D4, !VP3; 	nop


NOP; NOP; MADD.U16.SAT D8, V1, 123, D4
NOP; NOP; MADD.U16.SAT D8, V1, 123, D4, SP3
NOP; NOP; MADD.U16.SAT D8, V1, 123, D4, !SP3
NOP; NOP; MADD.U16.SAT D8, V1, 123, D4, VP3
NOP; NOP; MADD.U16.SAT D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.u16  st D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st D8, V1, 0x7b, D4, !VP3; 	nop

NOP; NOP; MADD.I8 A8, V1, V2, A4
NOP; NOP; MADD.I8 A8, V1, V2, A4, SP3
NOP; NOP; MADD.I8 A8, V1, V2, A4, !SP3
NOP; NOP; MADD.I8 A8, V1, V2, A4, VP3
NOP; NOP; MADD.I8 A8, V1, V2, A4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8  A8, V1, V2, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  A8, V1, V2, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  A8, V1, V2, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  A8, V1, V2, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  A8, V1, V2, A4, !VP3; 	nop


NOP; NOP; MADD.I8 A8, V1, 123, A4
NOP; NOP; MADD.I8 A8, V1, 123, A4, SP3
NOP; NOP; MADD.I8 A8, V1, 123, A4, !SP3
NOP; NOP; MADD.I8 A8, V1, 123, A4, VP3
NOP; NOP; MADD.I8 A8, V1, 123, A4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8  A8, V1, 0x7b, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  A8, V1, 0x7b, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  A8, V1, 0x7b, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  A8, V1, 0x7b, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  A8, V1, 0x7b, A4, !VP3; 	nop


NOP; NOP; MADD.I8.SAT A8, V1, V2, A4
NOP; NOP; MADD.I8.SAT A8, V1, V2, A4, SP3
NOP; NOP; MADD.I8.SAT A8, V1, V2, A4, !SP3
NOP; NOP; MADD.I8.SAT A8, V1, V2, A4, VP3
NOP; NOP; MADD.I8.SAT A8, V1, V2, A4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8  st A8, V1, V2, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st A8, V1, V2, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st A8, V1, V2, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st A8, V1, V2, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st A8, V1, V2, A4, !VP3; 	nop


NOP; NOP; MADD.I8.SAT A8, V1, 123, A4
NOP; NOP; MADD.I8.SAT A8, V1, 123, A4, SP3
NOP; NOP; MADD.I8.SAT A8, V1, 123, A4, !SP3
NOP; NOP; MADD.I8.SAT A8, V1, 123, A4, VP3
NOP; NOP; MADD.I8.SAT A8, V1, 123, A4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8  st A8, V1, 0x7b, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st A8, V1, 0x7b, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st A8, V1, 0x7b, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st A8, V1, 0x7b, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st A8, V1, 0x7b, A4, !VP3; 	nop

NOP; NOP; MADD.U8 A8, V1, V2, A4
NOP; NOP; MADD.U8 A8, V1, V2, A4, SP3
NOP; NOP; MADD.U8 A8, V1, V2, A4, !SP3
NOP; NOP; MADD.U8 A8, V1, V2, A4, VP3
NOP; NOP; MADD.U8 A8, V1, V2, A4, !VP3

// CHECK: 	nop; 	nop; 	madd.u8  A8, V1, V2, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  A8, V1, V2, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  A8, V1, V2, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  A8, V1, V2, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  A8, V1, V2, A4, !VP3; 	nop


NOP; NOP; MADD.U8 A8, V1, 123, A4
NOP; NOP; MADD.U8 A8, V1, 123, A4, SP3
NOP; NOP; MADD.U8 A8, V1, 123, A4, !SP3
NOP; NOP; MADD.U8 A8, V1, 123, A4, VP3
NOP; NOP; MADD.U8 A8, V1, 123, A4, !VP3

// CHECK: 	nop; 	nop; 	madd.u8  A8, V1, 0x7b, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  A8, V1, 0x7b, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  A8, V1, 0x7b, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  A8, V1, 0x7b, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  A8, V1, 0x7b, A4, !VP3; 	nop


NOP; NOP; MADD.U8.SAT A8, V1, V2, A4
NOP; NOP; MADD.U8.SAT A8, V1, V2, A4, SP3
NOP; NOP; MADD.U8.SAT A8, V1, V2, A4, !SP3
NOP; NOP; MADD.U8.SAT A8, V1, V2, A4, VP3
NOP; NOP; MADD.U8.SAT A8, V1, V2, A4, !VP3

// CHECK: 	nop; 	nop; 	madd.u8  st A8, V1, V2, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st A8, V1, V2, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st A8, V1, V2, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st A8, V1, V2, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st A8, V1, V2, A4, !VP3; 	nop


NOP; NOP; MADD.U8.SAT A8, V1, 123, A4
NOP; NOP; MADD.U8.SAT A8, V1, 123, A4, SP3
NOP; NOP; MADD.U8.SAT A8, V1, 123, A4, !SP3
NOP; NOP; MADD.U8.SAT A8, V1, 123, A4, VP3
NOP; NOP; MADD.U8.SAT A8, V1, 123, A4, !VP3

// CHECK: 	nop; 	nop; 	madd.u8  st A8, V1, 0x7b, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st A8, V1, 0x7b, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st A8, V1, 0x7b, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st A8, V1, 0x7b, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st A8, V1, 0x7b, A4, !VP3; 	nop


NOP; NOP; MADD.BF16 V5, V1, V2, V4
NOP; NOP; MADD.BF16 V5, V1, V2, V4, SP3
NOP; NOP; MADD.BF16 V5, V1, V2, V4, !SP3
NOP; NOP; MADD.BF16 V5, V1, V2, V4, VP3
NOP; NOP; MADD.BF16 V5, V1, V2, V4, !VP3
NOP; NOP; MADD.BF16 V5, V1, 0X3F800000, V4
NOP; NOP; MADD.BF16 V5, V1, 0X3F800000, V4, SP3
NOP; NOP; MADD.BF16 V5, V1, 0X3F800000, V4, !SP3
NOP; NOP; MADD.BF16 V5, V1, 0X3F800000, V4, VP3
NOP; NOP; MADD.BF16 V5, V1, 0X3F800000, V4, !VP3

// CHECK: 	nop; 	nop; 	madd.bf16  V5, V1, V2, V4; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  V5, V1, V2, V4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  V5, V1, V2, V4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  V5, V1, V2, V4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  V5, V1, V2, V4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  V5, V1, 0x3f800000, V4; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  V5, V1, 0x3f800000, V4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  V5, V1, 0x3f800000, V4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  V5, V1, 0x3f800000, V4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  V5, V1, 0x3f800000, V4, !VP3; 	nop


NOP; NOP; MADD.BF16.NEG V5, V1, V2, V4
NOP; NOP; MADD.BF16.NEG V5, V1, V2, V4, SP3
NOP; NOP; MADD.BF16.NEG V5, V1, V2, V4, !SP3
NOP; NOP; MADD.BF16.NEG V5, V1, V2, V4, VP3
NOP; NOP; MADD.BF16.NEG V5, V1, V2, V4, !VP3
NOP; NOP; MADD.BF16.NEG V5, V1, 0X3F800000, V4
NOP; NOP; MADD.BF16.NEG V5, V1, 0X3F800000, V4, SP3
NOP; NOP; MADD.BF16.NEG V5, V1, 0X3F800000, V4, !SP3
NOP; NOP; MADD.BF16.NEG V5, V1, 0X3F800000, V4, VP3
NOP; NOP; MADD.BF16.NEG V5, V1, 0X3F800000, V4, !VP3

// CHECK: 	nop; 	nop; 	madd.bf16  neg V5, V1, V2, V4; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg V5, V1, V2, V4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg V5, V1, V2, V4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg V5, V1, V2, V4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg V5, V1, V2, V4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg V5, V1, 0x3f800000, V4; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg V5, V1, 0x3f800000, V4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg V5, V1, 0x3f800000, V4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg V5, V1, 0x3f800000, V4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg V5, V1, 0x3f800000, V4, !VP3; 	nop


NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, V2, D4
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, V2, D4, SP3
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, V2, D4, !SP3
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, V2, D4, VP3
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, V2, D4, !VP3
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, 0X3F800000, D4
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, 0X3F800000, D4, SP3
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, 0X3F800000, D4, !SP3
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, 0X3F800000, D4, VP3
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, 0X3F800000, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, 0x3f800000, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, 0x3f800000, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, 0x3f800000, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, 0x3f800000, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, 0x3f800000, D4, !VP3; 	nop


NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, V2, D4
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, V2, D4, SP3
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, V2, D4, !SP3
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, V2, D4, VP3
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, V2, D4, !VP3
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, SP3
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, !SP3
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, VP3
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, 0x3f800000, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, 0x3f800000, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, 0x3f800000, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, 0x3f800000, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, 0x3f800000, D4, !VP3; 	nop


NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, V2, D4
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, V2, D4, SP3
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, V2, D4, !SP3
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, V2, D4, VP3
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, V2, D4, !VP3
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, 0X3F800000, D4
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, 0X3F800000, D4, SP3
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, 0X3F800000, D4, !SP3
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, 0X3F800000, D4, VP3
NOP; NOP; MADD.BF16.ACC_FP32 D6, V1, 0X3F800000, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, 0x3f800000, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, 0x3f800000, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, 0x3f800000, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, 0x3f800000, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  acc_fp32 D6, V1, 0x3f800000, D4, !VP3; 	nop


NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, V2, D4
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, V2, D4, SP3
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, V2, D4, !SP3
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, V2, D4, VP3
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, V2, D4, !VP3
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, SP3
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, !SP3
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, VP3
NOP; NOP; MADD.BF16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, 0x3f800000, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, 0x3f800000, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, 0x3f800000, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, 0x3f800000, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.bf16  neg acc_fp32 D6, V1, 0x3f800000, D4, !VP3; 	nop


NOP; NOP; MADD.F16 V5, V1, V2, V4
NOP; NOP; MADD.F16 V5, V1, V2, V4, SP3
NOP; NOP; MADD.F16 V5, V1, V2, V4, !SP3
NOP; NOP; MADD.F16 V5, V1, V2, V4, VP3
NOP; NOP; MADD.F16 V5, V1, V2, V4, !VP3
NOP; NOP; MADD.F16 V5, V1, 0X3F800000, V4
NOP; NOP; MADD.F16 V5, V1, 0X3F800000, V4, SP3
NOP; NOP; MADD.F16 V5, V1, 0X3F800000, V4, !SP3
NOP; NOP; MADD.F16 V5, V1, 0X3F800000, V4, VP3
NOP; NOP; MADD.F16 V5, V1, 0X3F800000, V4, !VP3

// CHECK: 	nop; 	nop; 	madd.f16  V5, V1, V2, V4; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  V5, V1, V2, V4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  V5, V1, V2, V4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  V5, V1, V2, V4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  V5, V1, V2, V4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  V5, V1, 0x3f800000, V4; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  V5, V1, 0x3f800000, V4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  V5, V1, 0x3f800000, V4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  V5, V1, 0x3f800000, V4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  V5, V1, 0x3f800000, V4, !VP3; 	nop


NOP; NOP; MADD.F16.NEG V5, V1, V2, V4
NOP; NOP; MADD.F16.NEG V5, V1, V2, V4, SP3
NOP; NOP; MADD.F16.NEG V5, V1, V2, V4, !SP3
NOP; NOP; MADD.F16.NEG V5, V1, V2, V4, VP3
NOP; NOP; MADD.F16.NEG V5, V1, V2, V4, !VP3
NOP; NOP; MADD.F16.NEG V5, V1, 0X3F800000, V4
NOP; NOP; MADD.F16.NEG V5, V1, 0X3F800000, V4, SP3
NOP; NOP; MADD.F16.NEG V5, V1, 0X3F800000, V4, !SP3
NOP; NOP; MADD.F16.NEG V5, V1, 0X3F800000, V4, VP3
NOP; NOP; MADD.F16.NEG V5, V1, 0X3F800000, V4, !VP3

// CHECK: 	nop; 	nop; 	madd.f16  neg V5, V1, V2, V4; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg V5, V1, V2, V4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg V5, V1, V2, V4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg V5, V1, V2, V4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg V5, V1, V2, V4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg V5, V1, 0x3f800000, V4; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg V5, V1, 0x3f800000, V4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg V5, V1, 0x3f800000, V4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg V5, V1, 0x3f800000, V4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg V5, V1, 0x3f800000, V4, !VP3; 	nop


NOP; NOP; MADD.F16.ACC_FP32 D6, V1, V2, D4
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, V2, D4, SP3
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, V2, D4, !SP3
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, V2, D4, VP3
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, V2, D4, !VP3
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, 0X3F800000, D4
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, 0X3F800000, D4, SP3
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, 0X3F800000, D4, !SP3
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, 0X3F800000, D4, VP3
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, 0X3F800000, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, 0x3f800000, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, 0x3f800000, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, 0x3f800000, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, 0x3f800000, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, 0x3f800000, D4, !VP3; 	nop


NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, V2, D4
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, V2, D4, SP3
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, V2, D4, !SP3
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, V2, D4, VP3
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, V2, D4, !VP3
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, SP3
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, !SP3
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, VP3
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, 0x3f800000, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, 0x3f800000, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, 0x3f800000, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, 0x3f800000, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, 0x3f800000, D4, !VP3; 	nop


NOP; NOP; MADD.F16.ACC_FP32 D6, V1, V2, D4
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, V2, D4, SP3
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, V2, D4, !SP3
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, V2, D4, VP3
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, V2, D4, !VP3
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, 0X3F800000, D4
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, 0X3F800000, D4, SP3
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, 0X3F800000, D4, !SP3
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, 0X3F800000, D4, VP3
NOP; NOP; MADD.F16.ACC_FP32 D6, V1, 0X3F800000, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, 0x3f800000, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, 0x3f800000, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, 0x3f800000, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, 0x3f800000, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  acc_fp32 D6, V1, 0x3f800000, D4, !VP3; 	nop


NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, V2, D4
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, V2, D4, SP3
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, V2, D4, !SP3
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, V2, D4, VP3
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, V2, D4, !VP3
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, SP3
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, !SP3
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, VP3
NOP; NOP; MADD.F16.NEG.ACC_FP32 D6, V1, 0X3F800000, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, 0x3f800000, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, 0x3f800000, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, 0x3f800000, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, 0x3f800000, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.f16  neg acc_fp32 D6, V1, 0x3f800000, D4, !VP3; 	nop

NOP; NOP; MADD.I8.NEG A8, V1, V2, A4
NOP; NOP; MADD.I8.NEG A8, V1, V2, A4, SP3
NOP; NOP; MADD.I8.NEG A8, V1, V2, A4, !SP3
NOP; NOP; MADD.I8.NEG A8, V1, V2, A4, VP3
NOP; NOP; MADD.I8.NEG A8, V1, V2, A4, !VP3

NOP; NOP; MADD.I8.NEG A8, V1, S2, A4
NOP; NOP; MADD.I8.NEG A8, V1, S2, A4, SP3
NOP; NOP; MADD.I8.NEG A8, V1, S2, A4, !SP3
NOP; NOP; MADD.I8.NEG A8, V1, S2, A4, VP3
NOP; NOP; MADD.I8.NEG A8, V1, S2, A4, !VP3

NOP; NOP; MADD.I8.NEG A8, V1, 123, A4
NOP; NOP; MADD.I8.NEG A8, V1, 123, A4, SP3
NOP; NOP; MADD.I8.NEG A8, V1, 123, A4, !SP3
NOP; NOP; MADD.I8.NEG A8, V1, 123, A4, VP3
NOP; NOP; MADD.I8.NEG A8, V1, 123, A4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8  neg A8, V1, V2, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg A8, V1, V2, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg A8, V1, V2, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg A8, V1, V2, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg A8, V1, V2, A4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg A8, V1, S2, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg A8, V1, S2, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg A8, V1, S2, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg A8, V1, S2, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg A8, V1, S2, A4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg A8, V1, 0x7b, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg A8, V1, 0x7b, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg A8, V1, 0x7b, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg A8, V1, 0x7b, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg A8, V1, 0x7b, A4, !VP3; 	nop


NOP; NOP; MADD.I8.SAT.NEG A8, V1, V2, A4
NOP; NOP; MADD.I8.SAT.NEG A8, V1, V2, A4, SP3
NOP; NOP; MADD.I8.SAT.NEG A8, V1, V2, A4, !SP3
NOP; NOP; MADD.I8.SAT.NEG A8, V1, V2, A4, VP3
NOP; NOP; MADD.I8.SAT.NEG A8, V1, V2, A4, !VP3

NOP; NOP; MADD.I8.SAT.NEG A8, V1, 123, A4
NOP; NOP; MADD.I8.SAT.NEG A8, V1, 123, A4, SP3
NOP; NOP; MADD.I8.SAT.NEG A8, V1, 123, A4, !SP3
NOP; NOP; MADD.I8.SAT.NEG A8, V1, 123, A4, VP3
NOP; NOP; MADD.I8.SAT.NEG A8, V1, 123, A4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8  st neg A8, V1, V2, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg A8, V1, V2, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg A8, V1, V2, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg A8, V1, V2, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg A8, V1, V2, A4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg A8, V1, 0x7b, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg A8, V1, 0x7b, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg A8, V1, 0x7b, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg A8, V1, 0x7b, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg A8, V1, 0x7b, A4, !VP3; 	nop


NOP; NOP; MADD.U8.NEG A8, V1, V2, A4
NOP; NOP; MADD.U8.NEG A8, V1, V2, A4, SP3
NOP; NOP; MADD.U8.NEG A8, V1, V2, A4, !SP3
NOP; NOP; MADD.U8.NEG A8, V1, V2, A4, VP3
NOP; NOP; MADD.U8.NEG A8, V1, V2, A4, !VP3

NOP; NOP; MADD.U8.NEG A8, V1, 123, A4
NOP; NOP; MADD.U8.NEG A8, V1, 123, A4, SP3
NOP; NOP; MADD.U8.NEG A8, V1, 123, A4, !SP3
NOP; NOP; MADD.U8.NEG A8, V1, 123, A4, VP3
NOP; NOP; MADD.U8.NEG A8, V1, 123, A4, !VP3

// CHECK: 	nop; 	nop; 	madd.u8  neg A8, V1, V2, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg A8, V1, V2, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg A8, V1, V2, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg A8, V1, V2, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg A8, V1, V2, A4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg A8, V1, 0x7b, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg A8, V1, 0x7b, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg A8, V1, 0x7b, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg A8, V1, 0x7b, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg A8, V1, 0x7b, A4, !VP3; 	nop


NOP; NOP; MADD.U8.SAT.NEG A8, V1, V2, A4
NOP; NOP; MADD.U8.SAT.NEG A8, V1, V2, A4, SP3
NOP; NOP; MADD.U8.SAT.NEG A8, V1, V2, A4, !SP3
NOP; NOP; MADD.U8.SAT.NEG A8, V1, V2, A4, VP3
NOP; NOP; MADD.U8.SAT.NEG A8, V1, V2, A4, !VP3

NOP; NOP; MADD.U8.SAT.NEG A8, V1, 123, A4
NOP; NOP; MADD.U8.SAT.NEG A8, V1, 123, A4, SP3
NOP; NOP; MADD.U8.SAT.NEG A8, V1, 123, A4, !SP3
NOP; NOP; MADD.U8.SAT.NEG A8, V1, 123, A4, VP3
NOP; NOP; MADD.U8.SAT.NEG A8, V1, 123, A4, !VP3

// CHECK: 	nop; 	nop; 	madd.u8  st neg A8, V1, V2, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg A8, V1, V2, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg A8, V1, V2, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg A8, V1, V2, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg A8, V1, V2, A4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg A8, V1, 0x7b, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg A8, V1, 0x7b, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg A8, V1, 0x7b, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg A8, V1, 0x7b, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg A8, V1, 0x7b, A4, !VP3; 	nop


NOP; NOP; MADD.I16.NEG D8, V1, V2, D4
NOP; NOP; MADD.I16.NEG D8, V1, V2, D4, SP3
NOP; NOP; MADD.I16.NEG D8, V1, V2, D4, !SP3
NOP; NOP; MADD.I16.NEG D8, V1, V2, D4, VP3
NOP; NOP; MADD.I16.NEG D8, V1, V2, D4, !VP3

NOP; NOP; MADD.I16.NEG D8, V1, 123, D4
NOP; NOP; MADD.I16.NEG D8, V1, 123, D4, SP3
NOP; NOP; MADD.I16.NEG D8, V1, 123, D4, !SP3
NOP; NOP; MADD.I16.NEG D8, V1, 123, D4, VP3
NOP; NOP; MADD.I16.NEG D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.I16.SAT.NEG D8, V1, V2, D4
NOP; NOP; MADD.I16.SAT.NEG D8, V1, V2, D4, SP3
NOP; NOP; MADD.I16.SAT.NEG D8, V1, V2, D4, !SP3
NOP; NOP; MADD.I16.SAT.NEG D8, V1, V2, D4, VP3
NOP; NOP; MADD.I16.SAT.NEG D8, V1, V2, D4, !VP3

NOP; NOP; MADD.I16.SAT.NEG D8, V1, 123, D4
NOP; NOP; MADD.I16.SAT.NEG D8, V1, 123, D4, SP3
NOP; NOP; MADD.I16.SAT.NEG D8, V1, 123, D4, !SP3
NOP; NOP; MADD.I16.SAT.NEG D8, V1, 123, D4, VP3
NOP; NOP; MADD.I16.SAT.NEG D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.U16.NEG D8, V1, V2, D4
NOP; NOP; MADD.U16.NEG D8, V1, V2, D4, SP3
NOP; NOP; MADD.U16.NEG D8, V1, V2, D4, !SP3
NOP; NOP; MADD.U16.NEG D8, V1, V2, D4, VP3
NOP; NOP; MADD.U16.NEG D8, V1, V2, D4, !VP3

NOP; NOP; MADD.U16.NEG D8, V1, 123, D4
NOP; NOP; MADD.U16.NEG D8, V1, 123, D4, SP3
NOP; NOP; MADD.U16.NEG D8, V1, 123, D4, !SP3
NOP; NOP; MADD.U16.NEG D8, V1, 123, D4, VP3
NOP; NOP; MADD.U16.NEG D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.u16  neg D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.U16.SAT.NEG D8, V1, V2, D4
NOP; NOP; MADD.U16.SAT.NEG D8, V1, V2, D4, SP3
NOP; NOP; MADD.U16.SAT.NEG D8, V1, V2, D4, !SP3
NOP; NOP; MADD.U16.SAT.NEG D8, V1, V2, D4, VP3
NOP; NOP; MADD.U16.SAT.NEG D8, V1, V2, D4, !VP3

NOP; NOP; MADD.U16.SAT.NEG D8, V1, 123, D4
NOP; NOP; MADD.U16.SAT.NEG D8, V1, 123, D4, SP3
NOP; NOP; MADD.U16.SAT.NEG D8, V1, 123, D4, !SP3
NOP; NOP; MADD.U16.SAT.NEG D8, V1, 123, D4, VP3
NOP; NOP; MADD.U16.SAT.NEG D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.I8.ACC_I16 D8, V1, V2, D4
NOP; NOP; MADD.I8.ACC_I16 D8, V1, V2, D4, SP3
NOP; NOP; MADD.I8.ACC_I16 D8, V1, V2, D4, !SP3
NOP; NOP; MADD.I8.ACC_I16 D8, V1, V2, D4, VP3
NOP; NOP; MADD.I8.ACC_I16 D8, V1, V2, D4, !VP3

NOP; NOP; MADD.I8.ACC_I16 D8, V1, 123, D4
NOP; NOP; MADD.I8.ACC_I16 D8, V1, 123, D4, SP3
NOP; NOP; MADD.I8.ACC_I16 D8, V1, 123, D4, !SP3
NOP; NOP; MADD.I8.ACC_I16 D8, V1, 123, D4, VP3
NOP; NOP; MADD.I8.ACC_I16 D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, V2, D4
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, V2, D4, SP3
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, V2, D4, !SP3
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, V2, D4, VP3
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, V2, D4, !VP3

NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, 123, D4
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, 123, D4, SP3
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, 123, D4, !SP3
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, 123, D4, VP3
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, V2, D4
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, V2, D4, SP3
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, V2, D4, !SP3
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, V2, D4, VP3
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, V2, D4, !VP3

NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, 123, D4
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, 123, D4, SP3
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, 123, D4, !SP3
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, 123, D4, VP3
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, V2, D4
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, V2, D4, SP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, V2, D4, !SP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, V2, D4, VP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, V2, D4, !VP3

NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, 123, D4
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, 123, D4, SP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, 123, D4, !SP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, 123, D4, VP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.I16.NEG D8, V1, V2, D4
NOP; NOP; MADD.I16.NEG D8, V1, V2, D4, SP3
NOP; NOP; MADD.I16.NEG D8, V1, V2, D4, !SP3
NOP; NOP; MADD.I16.NEG D8, V1, V2, D4, VP3
NOP; NOP; MADD.I16.NEG D8, V1, V2, D4, !VP3

NOP; NOP; MADD.I16.NEG D8, V1, 123, D4
NOP; NOP; MADD.I16.NEG D8, V1, 123, D4, SP3
NOP; NOP; MADD.I16.NEG D8, V1, 123, D4, !SP3
NOP; NOP; MADD.I16.NEG D8, V1, 123, D4, VP3
NOP; NOP; MADD.I16.NEG D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  neg D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.I16.SAT.NEG D8, V1, V2, D4
NOP; NOP; MADD.I16.SAT.NEG D8, V1, V2, D4, SP3
NOP; NOP; MADD.I16.SAT.NEG D8, V1, V2, D4, !SP3
NOP; NOP; MADD.I16.SAT.NEG D8, V1, V2, D4, VP3
NOP; NOP; MADD.I16.SAT.NEG D8, V1, V2, D4, !VP3

NOP; NOP; MADD.I16.SAT.NEG D8, V1, 123, D4
NOP; NOP; MADD.I16.SAT.NEG D8, V1, 123, D4, SP3
NOP; NOP; MADD.I16.SAT.NEG D8, V1, 123, D4, !SP3
NOP; NOP; MADD.I16.SAT.NEG D8, V1, 123, D4, VP3
NOP; NOP; MADD.I16.SAT.NEG D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i16  st neg D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.U16.NEG ACC_I32 D8, V1, V2, D4
NOP; NOP; MADD.U16.NEG ACC_I32 D8, V1, V2, D4, SP3
NOP; NOP; MADD.U16.NEG ACC_I32 D8, V1, V2, D4, !SP3
NOP; NOP; MADD.U16.NEG ACC_I32 D8, V1, V2, D4, VP3
NOP; NOP; MADD.U16.NEG ACC_I32 D8, V1, V2, D4, !VP3

NOP; NOP; MADD.U16.NEG ACC_I32 D8, V1, 123, D4
NOP; NOP; MADD.U16.NEG ACC_I32 D8, V1, 123, D4, SP3
NOP; NOP; MADD.U16.NEG ACC_I32 D8, V1, 123, D4, !SP3
NOP; NOP; MADD.U16.NEG ACC_I32 D8, V1, 123, D4, VP3
NOP; NOP; MADD.U16.NEG ACC_I32 D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.u16  neg acc_i32 D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg acc_i32 D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg acc_i32 D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg acc_i32 D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg acc_i32 D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg acc_i32 D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg acc_i32 D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg acc_i32 D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg acc_i32 D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  neg acc_i32 D8, V1, 0x7b, D4, !VP3; 	nop

NOP; NOP; MADD.U16.SAT.NEG D8, V1, V2, D4
NOP; NOP; MADD.U16.SAT.NEG D8, V1, V2, D4, SP3
NOP; NOP; MADD.U16.SAT.NEG D8, V1, V2, D4, !SP3
NOP; NOP; MADD.U16.SAT.NEG D8, V1, V2, D4, VP3
NOP; NOP; MADD.U16.SAT.NEG D8, V1, V2, D4, !VP3

NOP; NOP; MADD.U16.SAT.NEG D8, V1, 123, D4
NOP; NOP; MADD.U16.SAT.NEG D8, V1, 123, D4, SP3
NOP; NOP; MADD.U16.SAT.NEG D8, V1, 123, D4, !SP3
NOP; NOP; MADD.U16.SAT.NEG D8, V1, 123, D4, VP3
NOP; NOP; MADD.U16.SAT.NEG D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u16  st neg D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.I8.ACC_I16 D8, V1, V2, D4
NOP; NOP; MADD.I8.ACC_I16 D8, V1, V2, D4, SP3
NOP; NOP; MADD.I8.ACC_I16 D8, V1, V2, D4, !SP3
NOP; NOP; MADD.I8.ACC_I16 D8, V1, V2, D4, VP3
NOP; NOP; MADD.I8.ACC_I16 D8, V1, V2, D4, !VP3

NOP; NOP; MADD.I8.ACC_I16 D8, V1, 123, D4
NOP; NOP; MADD.I8.ACC_I16 D8, V1, 123, D4, SP3
NOP; NOP; MADD.I8.ACC_I16 D8, V1, 123, D4, !SP3
NOP; NOP; MADD.I8.ACC_I16 D8, V1, 123, D4, VP3
NOP; NOP; MADD.I8.ACC_I16 D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  acc_i16 D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, V2, D4
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, V2, D4, SP3
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, V2, D4, !SP3
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, V2, D4, VP3
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, V2, D4, !VP3

NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, 123, D4
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, 123, D4, SP3
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, 123, D4, !SP3
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, 123, D4, VP3
NOP; NOP; MADD.I8.SAT.ACC_I16 D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st acc_i16 D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, V2, D4
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, V2, D4, SP3
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, V2, D4, !SP3
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, V2, D4, VP3
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, V2, D4, !VP3

NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, 123, D4
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, 123, D4, SP3
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, 123, D4, !SP3
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, 123, D4, VP3
NOP; NOP; MADD.I8.NEG.ACC_I16 D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  neg acc_i16 D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, V2, D4
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, V2, D4, SP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, V2, D4, !SP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, V2, D4, VP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, V2, D4, !VP3

NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, 123, D4
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, 123, D4, SP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, 123, D4, !SP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, 123, D4, VP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8  st neg acc_i16 D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.U8.ACC_I16 D8, V1, V2, D4
NOP; NOP; MADD.U8.ACC_I16 D8, V1, V2, D4, SP3
NOP; NOP; MADD.U8.ACC_I16 D8, V1, V2, D4, !SP3
NOP; NOP; MADD.U8.ACC_I16 D8, V1, V2, D4, VP3
NOP; NOP; MADD.U8.ACC_I16 D8, V1, V2, D4, !VP3

NOP; NOP; MADD.U8.ACC_I16 D8, V1, 123, D4
NOP; NOP; MADD.U8.ACC_I16 D8, V1, 123, D4, SP3
NOP; NOP; MADD.U8.ACC_I16 D8, V1, 123, D4, !SP3
NOP; NOP; MADD.U8.ACC_I16 D8, V1, 123, D4, VP3
NOP; NOP; MADD.U8.ACC_I16 D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.u8  acc_i16 D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  acc_i16 D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  acc_i16 D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  acc_i16 D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  acc_i16 D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  acc_i16 D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  acc_i16 D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  acc_i16 D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  acc_i16 D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  acc_i16 D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.U8.SAT.ACC_I16 D8, V1, V2, D4
NOP; NOP; MADD.U8.SAT.ACC_I16 D8, V1, V2, D4, SP3
NOP; NOP; MADD.U8.SAT.ACC_I16 D8, V1, V2, D4, !SP3
NOP; NOP; MADD.U8.SAT.ACC_I16 D8, V1, V2, D4, VP3
NOP; NOP; MADD.U8.SAT.ACC_I16 D8, V1, V2, D4, !VP3

NOP; NOP; MADD.U8.SAT.ACC_I16 D8, V1, 123, D4
NOP; NOP; MADD.U8.SAT.ACC_I16 D8, V1, 123, D4, SP3
NOP; NOP; MADD.U8.SAT.ACC_I16 D8, V1, 123, D4, !SP3
NOP; NOP; MADD.U8.SAT.ACC_I16 D8, V1, 123, D4, VP3
NOP; NOP; MADD.U8.SAT.ACC_I16 D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.u8  st acc_i16 D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st acc_i16 D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st acc_i16 D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st acc_i16 D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st acc_i16 D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st acc_i16 D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st acc_i16 D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st acc_i16 D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st acc_i16 D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st acc_i16 D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.U8.NEG.ACC_I16 D8, V1, V2, D4
NOP; NOP; MADD.U8.NEG.ACC_I16 D8, V1, V2, D4, SP3
NOP; NOP; MADD.U8.NEG.ACC_I16 D8, V1, V2, D4, !SP3
NOP; NOP; MADD.U8.NEG.ACC_I16 D8, V1, V2, D4, VP3
NOP; NOP; MADD.U8.NEG.ACC_I16 D8, V1, V2, D4, !VP3

NOP; NOP; MADD.U8.NEG.ACC_I16 D8, V1, 123, D4
NOP; NOP; MADD.U8.NEG.ACC_I16 D8, V1, 123, D4, SP3
NOP; NOP; MADD.U8.NEG.ACC_I16 D8, V1, 123, D4, !SP3
NOP; NOP; MADD.U8.NEG.ACC_I16 D8, V1, 123, D4, VP3
NOP; NOP; MADD.U8.NEG.ACC_I16 D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.u8  neg acc_i16 D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg acc_i16 D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg acc_i16 D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg acc_i16 D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg acc_i16 D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg acc_i16 D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg acc_i16 D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg acc_i16 D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg acc_i16 D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  neg acc_i16 D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 D8, V1, V2, D4
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 D8, V1, V2, D4, SP3
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 D8, V1, V2, D4, !SP3
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 D8, V1, V2, D4, VP3
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 D8, V1, V2, D4, !VP3

NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 D8, V1, 123, D4
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 D8, V1, 123, D4, SP3
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 D8, V1, 123, D4, !SP3
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 D8, V1, 123, D4, VP3
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.u8  st neg acc_i16 D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg acc_i16 D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg acc_i16 D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg acc_i16 D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg acc_i16 D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg acc_i16 D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg acc_i16 D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg acc_i16 D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg acc_i16 D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8  st neg acc_i16 D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 ZP_REG D8, V1, V2, D4
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 ZP_REG D8, V1, V2, D4, SP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 ZP_REG D8, V1, V2, D4, !SP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 ZP_REG D8, V1, V2, D4, VP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 ZP_REG D8, V1, V2, D4, !VP3

NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 ZP_REG D8, V1, 123, D4
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 ZP_REG D8, V1, 123, D4, SP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 ZP_REG D8, V1, 123, D4, !SP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 ZP_REG D8, V1, 123, D4, VP3
NOP; NOP; MADD.I8.SAT.NEG.ACC_I16 ZP_REG D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8 st neg acc_i16 ZP_REG D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg acc_i16 ZP_REG D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg acc_i16 ZP_REG D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg acc_i16 ZP_REG D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg acc_i16 ZP_REG D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg acc_i16 ZP_REG D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg acc_i16 ZP_REG D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg acc_i16 ZP_REG D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg acc_i16 ZP_REG D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg acc_i16 ZP_REG D8, V1, 0x7b, D4, !VP3; 	nop


NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 ZP_REG D8, V1, V2, D4
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 ZP_REG D8, V1, V2, D4, SP3
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 ZP_REG D8, V1, V2, D4, !SP3
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 ZP_REG D8, V1, V2, D4, VP3
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 ZP_REG D8, V1, V2, D4, !VP3

NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 ZP_REG D8, V1, 123, D4
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 ZP_REG D8, V1, 123, D4, SP3
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 ZP_REG D8, V1, 123, D4, !SP3
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 ZP_REG D8, V1, 123, D4, VP3
NOP; NOP; MADD.U8.SAT.NEG.ACC_I16 ZP_REG D8, V1, 123, D4, !VP3

// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i16 ZP_REG D8, V1, V2, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i16 ZP_REG D8, V1, V2, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i16 ZP_REG D8, V1, V2, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i16 ZP_REG D8, V1, V2, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i16 ZP_REG D8, V1, V2, D4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i16 ZP_REG D8, V1, 0x7b, D4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i16 ZP_REG D8, V1, 0x7b, D4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i16 ZP_REG D8, V1, 0x7b, D4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i16 ZP_REG D8, V1, 0x7b, D4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i16 ZP_REG D8, V1, 0x7b, D4, !VP3; 	nop



NOP; NOP; MADD.I8.SAT.NEG ZP_REG A8, V1, V2, A4
NOP; NOP; MADD.I8.SAT.NEG ZP_REG A8, V1, V2, A4, SP3
NOP; NOP; MADD.I8.SAT.NEG ZP_REG A8, V1, V2, A4, !SP3
NOP; NOP; MADD.I8.SAT.NEG ZP_REG A8, V1, V2, A4, VP3
NOP; NOP; MADD.I8.SAT.NEG ZP_REG A8, V1, V2, A4, !VP3

NOP; NOP; MADD.I8.SAT.NEG ZP_REG A8, V1, 123, A4
NOP; NOP; MADD.I8.SAT.NEG ZP_REG A8, V1, 123, A4, SP3
NOP; NOP; MADD.I8.SAT.NEG ZP_REG A8, V1, 123, A4, !SP3
NOP; NOP; MADD.I8.SAT.NEG ZP_REG A8, V1, 123, A4, VP3
NOP; NOP; MADD.I8.SAT.NEG ZP_REG A8, V1, 123, A4, !VP3

// CHECK: 	nop; 	nop; 	madd.i8 st neg ZP_REG A8, V1, V2, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg ZP_REG A8, V1, V2, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg ZP_REG A8, V1, V2, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg ZP_REG A8, V1, V2, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg ZP_REG A8, V1, V2, A4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg ZP_REG A8, V1, 0x7b, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg ZP_REG A8, V1, 0x7b, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg ZP_REG A8, V1, 0x7b, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg ZP_REG A8, V1, 0x7b, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.i8 st neg ZP_REG A8, V1, 0x7b, A4, !VP3; 	nop

NOP; NOP; MADD.u8.SAT.NEG ZP_REG A8, V1, V2, A4
NOP; NOP; MADD.u8.SAT.NEG ZP_REG A8, V1, V2, A4, SP3
NOP; NOP; MADD.u8.SAT.NEG ZP_REG A8, V1, V2, A4, !SP3
NOP; NOP; MADD.u8.SAT.NEG ZP_REG A8, V1, V2, A4, VP3
NOP; NOP; MADD.u8.SAT.NEG ZP_REG A8, V1, V2, A4, !VP3

NOP; NOP; MADD.u8.SAT.NEG ZP_REG A8, V1, 123, A4
NOP; NOP; MADD.u8.SAT.NEG ZP_REG A8, V1, 123, A4, SP3
NOP; NOP; MADD.u8.SAT.NEG ZP_REG A8, V1, 123, A4, !SP3
NOP; NOP; MADD.u8.SAT.NEG ZP_REG A8, V1, 123, A4, VP3
NOP; NOP; MADD.u8.SAT.NEG ZP_REG A8, V1, 123, A4, !VP3

// CHECK: 	nop; 	nop; 	madd.u8 st neg ZP_REG A8, V1, V2, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg ZP_REG A8, V1, V2, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg ZP_REG A8, V1, V2, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg ZP_REG A8, V1, V2, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg ZP_REG A8, V1, V2, A4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg ZP_REG A8, V1, 0x7b, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg ZP_REG A8, V1, 0x7b, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg ZP_REG A8, V1, 0x7b, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg ZP_REG A8, V1, 0x7b, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg ZP_REG A8, V1, 0x7b, A4, !VP3; 	nop


NOP; NOP; MADD.u8.SAT.NEG.ACC_I32 ZP_REG A8, V1, V2, A4
NOP; NOP; MADD.u8.SAT.NEG.ACC_I32 ZP_REG A8, V1, V2, A4, SP3
NOP; NOP; MADD.u8.SAT.NEG.ACC_I32 ZP_REG A8, V1, V2, A4, !SP3
NOP; NOP; MADD.u8.SAT.NEG.ACC_I32 ZP_REG A8, V1, V2, A4, VP3
NOP; NOP; MADD.u8.SAT.NEG.ACC_I32 ZP_REG A8, V1, V2, A4, !VP3

NOP; NOP; MADD.u8.SAT.NEG.ACC_I32 ZP_REG A8, V1, 123, A4
NOP; NOP; MADD.u8.SAT.NEG.ACC_I32 ZP_REG A8, V1, 123, A4, SP3
NOP; NOP; MADD.u8.SAT.NEG.ACC_I32 ZP_REG A8, V1, 123, A4, !SP3
NOP; NOP; MADD.u8.SAT.NEG.ACC_I32 ZP_REG A8, V1, 123, A4, VP3
NOP; NOP; MADD.u8.SAT.NEG.ACC_I32 ZP_REG A8, V1, 123, A4, !VP3

// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i32 ZP_REG A8, V1, V2, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i32 ZP_REG A8, V1, V2, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i32 ZP_REG A8, V1, V2, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i32 ZP_REG A8, V1, V2, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i32 ZP_REG A8, V1, V2, A4, !VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i32 ZP_REG A8, V1, 0x7b, A4; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i32 ZP_REG A8, V1, 0x7b, A4, SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i32 ZP_REG A8, V1, 0x7b, A4, !SP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i32 ZP_REG A8, V1, 0x7b, A4, VP3; 	nop
// CHECK: 	nop; 	nop; 	madd.u8 st neg acc_i32 ZP_REG A8, V1, 0x7b, A4, !VP3; 	nop