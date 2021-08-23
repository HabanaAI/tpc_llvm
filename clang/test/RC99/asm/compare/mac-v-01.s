// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s




NOP; NOP; MAC.F32 V5, V1, V2
NOP; NOP; MAC.F32 V5, V1, V2, SP3
NOP; NOP; MAC.F32 V5, V1, V2, !SP3
NOP; NOP; MAC.F32 V5, V1, V2, VP3
NOP; NOP; MAC.F32 V5, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.f32  V5, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  V5, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  V5, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  V5, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  V5, V1, V2, !VP3; 	nop


NOP; NOP; MAC.F32 V5, V1, 0X3F800000
NOP; NOP; MAC.F32 V5, V1, 0X3F800000, SP3
NOP; NOP; MAC.F32 V5, V1, 0X3F800000, !SP3
NOP; NOP; MAC.F32 V5, V1, 0X3F800000, VP3
NOP; NOP; MAC.F32 V5, V1, 0X3F800000, !VP3

// CHECK: 	nop; 	nop; 	mac.f32  V5, V1, 0x3f800000, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  V5, V1, 0x3f800000, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  V5, V1, 0x3f800000, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  V5, V1, 0x3f800000, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  V5, V1, 0x3f800000, !VP3; 	nop


NOP; NOP; MAC.F32.NEG V5, V1, V2
NOP; NOP; MAC.F32.NEG V5, V1, V2, SP3
NOP; NOP; MAC.F32.NEG V5, V1, V2, !SP3
NOP; NOP; MAC.F32.NEG V5, V1, V2, VP3
NOP; NOP; MAC.F32.NEG V5, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.f32  neg V5, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  neg V5, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  neg V5, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  neg V5, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  neg V5, V1, V2, !VP3; 	nop


NOP; NOP; MAC.F32.NEG V5, V1, 0X3F800000
NOP; NOP; MAC.F32.NEG V5, V1, 0X3F800000, SP3
NOP; NOP; MAC.F32.NEG V5, V1, 0X3F800000, !SP3
NOP; NOP; MAC.F32.NEG V5, V1, 0X3F800000, VP3
NOP; NOP; MAC.F32.NEG V5, V1, 0X3F800000, !VP3

// CHECK: 	nop; 	nop; 	mac.f32  neg V5, V1, 0x3f800000, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  neg V5, V1, 0x3f800000, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  neg V5, V1, 0x3f800000, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  neg V5, V1, 0x3f800000, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.f32  neg V5, V1, 0x3f800000, !VP3; 	nop


NOP; NOP; MAC.I16 D8, V1, V2
NOP; NOP; MAC.I16 D8, V1, V2, SP3
NOP; NOP; MAC.I16 D8, V1, V2, !SP3
NOP; NOP; MAC.I16 D8, V1, V2, VP3
NOP; NOP; MAC.I16 D8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.I16 D8, V1, 123
NOP; NOP; MAC.I16 D8, V1, 123, SP3
NOP; NOP; MAC.I16 D8, V1, 123, !SP3
NOP; NOP; MAC.I16 D8, V1, 123, VP3
NOP; NOP; MAC.I16 D8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, 0x7b, !VP3; 	nop


NOP; NOP; MAC.I16.SAT D8, V1, V2
NOP; NOP; MAC.I16.SAT D8, V1, V2, SP3
NOP; NOP; MAC.I16.SAT D8, V1, V2, !SP3
NOP; NOP; MAC.I16.SAT D8, V1, V2, VP3
NOP; NOP; MAC.I16.SAT D8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.I16.SAT D8, V1, 123
NOP; NOP; MAC.I16.SAT D8, V1, 123, SP3
NOP; NOP; MAC.I16.SAT D8, V1, 123, !SP3
NOP; NOP; MAC.I16.SAT D8, V1, 123, VP3
NOP; NOP; MAC.I16.SAT D8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, 0x7b, !VP3; 	nop


NOP; NOP; MAC.I16 V8, V1, V2
NOP; NOP; MAC.I16 V8, V1, V2, SP3
NOP; NOP; MAC.I16 V8, V1, V2, !SP3
NOP; NOP; MAC.I16 V8, V1, V2, VP3
NOP; NOP; MAC.I16 V8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.I16 V8, V1, 123
NOP; NOP; MAC.I16 V8, V1, 123, SP3
NOP; NOP; MAC.I16 V8, V1, 123, !SP3
NOP; NOP; MAC.I16 V8, V1, 123, VP3
NOP; NOP; MAC.I16 V8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  D8, V1, 0x7b, !VP3; 	nop


NOP; NOP; MAC.I16.SAT V8, V1, V2
NOP; NOP; MAC.I16.SAT V8, V1, V2, SP3
NOP; NOP; MAC.I16.SAT V8, V1, V2, !SP3
NOP; NOP; MAC.I16.SAT V8, V1, V2, VP3
NOP; NOP; MAC.I16.SAT V8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.I16.SAT V8, V1, 123
NOP; NOP; MAC.I16.SAT V8, V1, 123, SP3
NOP; NOP; MAC.I16.SAT V8, V1, 123, !SP3
NOP; NOP; MAC.I16.SAT V8, V1, 123, VP3
NOP; NOP; MAC.I16.SAT V8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i16  st D8, V1, 0x7b, !VP3; 	nop


NOP; NOP; MAC.U16 D8, V1, V2
NOP; NOP; MAC.U16 D8, V1, V2, SP3
NOP; NOP; MAC.U16 D8, V1, V2, !SP3
NOP; NOP; MAC.U16 D8, V1, V2, VP3
NOP; NOP; MAC.U16 D8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.U16 D8, V1, 123
NOP; NOP; MAC.U16 D8, V1, 123, SP3
NOP; NOP; MAC.U16 D8, V1, 123, !SP3
NOP; NOP; MAC.U16 D8, V1, 123, VP3
NOP; NOP; MAC.U16 D8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, 0x7b, !VP3; 	nop


NOP; NOP; MAC.U16.SAT D8, V1, V2
NOP; NOP; MAC.U16.SAT D8, V1, V2, SP3
NOP; NOP; MAC.U16.SAT D8, V1, V2, !SP3
NOP; NOP; MAC.U16.SAT D8, V1, V2, VP3
NOP; NOP; MAC.U16.SAT D8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.U16.SAT D8, V1, 123
NOP; NOP; MAC.U16.SAT D8, V1, 123, SP3
NOP; NOP; MAC.U16.SAT D8, V1, 123, !SP3
NOP; NOP; MAC.U16.SAT D8, V1, 123, VP3
NOP; NOP; MAC.U16.SAT D8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, 0x7b, !VP3; 	nop


NOP; NOP; MAC.U16 V8, V1, V2
NOP; NOP; MAC.U16 V8, V1, V2, SP3
NOP; NOP; MAC.U16 V8, V1, V2, !SP3
NOP; NOP; MAC.U16 V8, V1, V2, VP3
NOP; NOP; MAC.U16 V8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.U16 V8, V1, 123
NOP; NOP; MAC.U16 V8, V1, 123, SP3
NOP; NOP; MAC.U16 V8, V1, 123, !SP3
NOP; NOP; MAC.U16 V8, V1, 123, VP3
NOP; NOP; MAC.U16 V8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  D8, V1, 0x7b, !VP3; 	nop


NOP; NOP; MAC.U16.SAT V8, V1, V2
NOP; NOP; MAC.U16.SAT V8, V1, V2, SP3
NOP; NOP; MAC.U16.SAT V8, V1, V2, !SP3
NOP; NOP; MAC.U16.SAT V8, V1, V2, VP3
NOP; NOP; MAC.U16.SAT V8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.U16.SAT V8, V1, 123
NOP; NOP; MAC.U16.SAT V8, V1, 123, SP3
NOP; NOP; MAC.U16.SAT V8, V1, 123, !SP3
NOP; NOP; MAC.U16.SAT V8, V1, 123, VP3
NOP; NOP; MAC.U16.SAT V8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u16  st D8, V1, 0x7b, !VP3; 	nop


NOP; NOP; MAC.I8 A8, V1, V2
NOP; NOP; MAC.I8 A8, V1, V2, SP3
NOP; NOP; MAC.I8 A8, V1, V2, !SP3
NOP; NOP; MAC.I8 A8, V1, V2, VP3
NOP; NOP; MAC.I8 A8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.I8 A8, V1, 123
NOP; NOP; MAC.I8 A8, V1, 123, SP3
NOP; NOP; MAC.I8 A8, V1, 123, !SP3
NOP; NOP; MAC.I8 A8, V1, 123, VP3
NOP; NOP; MAC.I8 A8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, 0x7b, !VP3; 	nop


NOP; NOP; MAC.I8.SAT A8, V1, V2
NOP; NOP; MAC.I8.SAT A8, V1, V2, SP3
NOP; NOP; MAC.I8.SAT A8, V1, V2, !SP3
NOP; NOP; MAC.I8.SAT A8, V1, V2, VP3
NOP; NOP; MAC.I8.SAT A8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.I8.SAT A8, V1, 123
NOP; NOP; MAC.I8.SAT A8, V1, 123, SP3
NOP; NOP; MAC.I8.SAT A8, V1, 123, !SP3
NOP; NOP; MAC.I8.SAT A8, V1, 123, VP3
NOP; NOP; MAC.I8.SAT A8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, 0x7b, !VP3; 	nop


NOP; NOP; MAC.I8 V8, V1, V2
NOP; NOP; MAC.I8 V8, V1, V2, SP3
NOP; NOP; MAC.I8 V8, V1, V2, !SP3
NOP; NOP; MAC.I8 V8, V1, V2, VP3
NOP; NOP; MAC.I8 V8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.I8 V8, V1, 123
NOP; NOP; MAC.I8 V8, V1, 123, SP3
NOP; NOP; MAC.I8 V8, V1, 123, !SP3
NOP; NOP; MAC.I8 V8, V1, 123, VP3
NOP; NOP; MAC.I8 V8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  A8, V1, 0x7b, !VP3; 	nop


NOP; NOP; MAC.I8.SAT V8, V1, V2
NOP; NOP; MAC.I8.SAT V8, V1, V2, SP3
NOP; NOP; MAC.I8.SAT V8, V1, V2, !SP3
NOP; NOP; MAC.I8.SAT V8, V1, V2, VP3
NOP; NOP; MAC.I8.SAT V8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.I8.SAT V8, V1, 123
NOP; NOP; MAC.I8.SAT V8, V1, 123, SP3
NOP; NOP; MAC.I8.SAT V8, V1, 123, !SP3
NOP; NOP; MAC.I8.SAT V8, V1, 123, VP3
NOP; NOP; MAC.I8.SAT V8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.i8  st A8, V1, 0x7b, !VP3; 	nop


NOP; NOP; MAC.U8 A8, V1, V2
NOP; NOP; MAC.U8 A8, V1, V2, SP3
NOP; NOP; MAC.U8 A8, V1, V2, !SP3
NOP; NOP; MAC.U8 A8, V1, V2, VP3
NOP; NOP; MAC.U8 A8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.U8 A8, V1, 123
NOP; NOP; MAC.U8 A8, V1, 123, SP3
NOP; NOP; MAC.U8 A8, V1, 123, !SP3
NOP; NOP; MAC.U8 A8, V1, 123, VP3
NOP; NOP; MAC.U8 A8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, 0x7b, !VP3; 	nop


NOP; NOP; MAC.U8.SAT A8, V1, V2
NOP; NOP; MAC.U8.SAT A8, V1, V2, SP3
NOP; NOP; MAC.U8.SAT A8, V1, V2, !SP3
NOP; NOP; MAC.U8.SAT A8, V1, V2, VP3
NOP; NOP; MAC.U8.SAT A8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.U8.SAT A8, V1, 123
NOP; NOP; MAC.U8.SAT A8, V1, 123, SP3
NOP; NOP; MAC.U8.SAT A8, V1, 123, !SP3
NOP; NOP; MAC.U8.SAT A8, V1, 123, VP3
NOP; NOP; MAC.U8.SAT A8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, 0x7b, !VP3; 	nop


NOP; NOP; MAC.U8 V8, V1, V2
NOP; NOP; MAC.U8 V8, V1, V2, SP3
NOP; NOP; MAC.U8 V8, V1, V2, !SP3
NOP; NOP; MAC.U8 V8, V1, V2, VP3
NOP; NOP; MAC.U8 V8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.U8 V8, V1, 123
NOP; NOP; MAC.U8 V8, V1, 123, SP3
NOP; NOP; MAC.U8 V8, V1, 123, !SP3
NOP; NOP; MAC.U8 V8, V1, 123, VP3
NOP; NOP; MAC.U8 V8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  A8, V1, 0x7b, !VP3; 	nop


NOP; NOP; MAC.U8.SAT V8, V1, V2
NOP; NOP; MAC.U8.SAT V8, V1, V2, SP3
NOP; NOP; MAC.U8.SAT V8, V1, V2, !SP3
NOP; NOP; MAC.U8.SAT V8, V1, V2, VP3
NOP; NOP; MAC.U8.SAT V8, V1, V2, !VP3

// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, V2, !VP3; 	nop


NOP; NOP; MAC.U8.SAT V8, V1, 123
NOP; NOP; MAC.U8.SAT V8, V1, 123, SP3
NOP; NOP; MAC.U8.SAT V8, V1, 123, !SP3
NOP; NOP; MAC.U8.SAT V8, V1, 123, VP3
NOP; NOP; MAC.U8.SAT V8, V1, 123, !VP3

// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, 0x7b, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, 0x7b, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, 0x7b, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, 0x7b, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.u8  st A8, V1, 0x7b, !VP3; 	nop


