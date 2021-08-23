// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm -mcpu gaudi %t.o | FileCheck %s



NOP; NOP; OR.F32 V5, V1, V2
NOP; NOP; OR.F32 V5, V1, V2, SP3
NOP; NOP; OR.F32 V5, V1, V2, !SP3
NOP; NOP; OR.F32 V5, V1, V2, VP3
NOP; NOP; OR.F32 V5, V1, V2, !VP3

// CHECK: nop;    nop;    or.f32  V5, V1, V2, SP0;       nop
// CHECK: nop;    nop;    or.f32  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    or.f32  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    or.f32  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    or.f32  V5, V1, V2, !VP3;      nop

NOP; NOP; OR.F32 V5, V1, S2
NOP; NOP; OR.F32 V5, V1, S2, SP3
NOP; NOP; OR.F32 V5, V1, S2, !SP3
NOP; NOP; OR.F32 V5, V1, S2, VP3
NOP; NOP; OR.F32 V5, V1, S2, !VP3

// CHECK: nop;    nop;    or.f32  V5, V1, S2, SP0;       nop
// CHECK: nop;    nop;    or.f32  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    or.f32  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    or.f32  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    or.f32  V5, V1, S2, !VP3;      nop

NOP; NOP; OR.F32 V5, V1, 0x3f4ccccd
NOP; NOP; OR.F32 V5, V1, 0x3f4ccccd, SP3
NOP; NOP; OR.F32 V5, V1, 0x3f4ccccd, !SP3
NOP; NOP; OR.F32 V5, V1, 0x3f4ccccd, VP3
NOP; NOP; OR.F32 V5, V1, 0x3f4ccccd, !VP3

// CHECK: nop;    nop;    or.f32  V5, V1, 0x3f4ccccd, SP0;       nop
// CHECK: nop;    nop;    or.f32  V5, V1, 0x3f4ccccd, SP3;       nop
// CHECK: nop;    nop;    or.f32  V5, V1, 0x3f4ccccd, !SP3;      nop
// CHECK: nop;    nop;    or.f32  V5, V1, 0x3f4ccccd, VP3;       nop
// CHECK: nop;    nop;    or.f32  V5, V1, 0x3f4ccccd, !VP3;      nop

NOP; NOP; OR.I32 V5, V1, V2
NOP; NOP; OR.I32 V5, V1, V2, SP3
NOP; NOP; OR.I32 V5, V1, V2, !SP3
NOP; NOP; OR.I32 V5, V1, V2, VP3
NOP; NOP; OR.I32 V5, V1, V2, !VP3

// CHECK: nop;    nop;    or.i32  V5, V1, V2, SP0;       nop
// CHECK: nop;    nop;    or.i32  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    or.i32  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    or.i32  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    or.i32  V5, V1, V2, !VP3;      nop

NOP; NOP; OR.I32 V5, V1, S2
NOP; NOP; OR.I32 V5, V1, S2, SP3
NOP; NOP; OR.I32 V5, V1, S2, !SP3
NOP; NOP; OR.I32 V5, V1, S2, VP3
NOP; NOP; OR.I32 V5, V1, S2, !VP3

// CHECK: nop;    nop;    or.i32  V5, V1, S2, SP0;       nop
// CHECK: nop;    nop;    or.i32  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    or.i32  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    or.i32  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    or.i32  V5, V1, S2, !VP3;      nop

NOP; NOP; OR.I32 V5, V1, 123
NOP; NOP; OR.I32 V5, V1, 123, SP3
NOP; NOP; OR.I32 V5, V1, 123, !SP3
NOP; NOP; OR.I32 V5, V1, 123, VP3
NOP; NOP; OR.I32 V5, V1, 123, !VP3

// CHECK: nop;    nop;    or.i32  V5, V1, 0x7b, SP0;     nop
// CHECK: nop;    nop;    or.i32  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    or.i32  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    or.i32  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    or.i32  V5, V1, 0x7b, !VP3;    nop

NOP; NOP; OR.U32 V5, V1, V2
NOP; NOP; OR.U32 V5, V1, V2, SP3
NOP; NOP; OR.U32 V5, V1, V2, !SP3
NOP; NOP; OR.U32 V5, V1, V2, VP3
NOP; NOP; OR.U32 V5, V1, V2, !VP3

// CHECK: nop;    nop;    or.u32  V5, V1, V2, SP0;       nop
// CHECK: nop;    nop;    or.u32  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    or.u32  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    or.u32  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    or.u32  V5, V1, V2, !VP3;      nop

NOP; NOP; OR.U32 V5, V1, S2
NOP; NOP; OR.U32 V5, V1, S2, SP3
NOP; NOP; OR.U32 V5, V1, S2, !SP3
NOP; NOP; OR.U32 V5, V1, S2, VP3
NOP; NOP; OR.U32 V5, V1, S2, !VP3

// CHECK: nop;    nop;    or.u32  V5, V1, S2, SP0;       nop
// CHECK: nop;    nop;    or.u32  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    or.u32  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    or.u32  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    or.u32  V5, V1, S2, !VP3;      nop

NOP; NOP; OR.U32 V5, V1, 123
NOP; NOP; OR.U32 V5, V1, 123, SP3
NOP; NOP; OR.U32 V5, V1, 123, !SP3
NOP; NOP; OR.U32 V5, V1, 123, VP3
NOP; NOP; OR.U32 V5, V1, 123, !VP3

// CHECK: nop;    nop;    or.u32  V5, V1, 0x7b, SP0;     nop
// CHECK: nop;    nop;    or.u32  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    or.u32  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    or.u32  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    or.u32  V5, V1, 0x7b, !VP3;    nop

NOP; NOP; OR.I16 V5, V1, V2
NOP; NOP; OR.I16 V5, V1, V2, SP3
NOP; NOP; OR.I16 V5, V1, V2, !SP3
NOP; NOP; OR.I16 V5, V1, V2, VP3
NOP; NOP; OR.I16 V5, V1, V2, !VP3

// CHECK: nop;    nop;    or.i16  V5, V1, V2, SP0;       nop
// CHECK: nop;    nop;    or.i16  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    or.i16  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    or.i16  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    or.i16  V5, V1, V2, !VP3;      nop

NOP; NOP; OR.I16 V5, V1, S2
NOP; NOP; OR.I16 V5, V1, S2, SP3
NOP; NOP; OR.I16 V5, V1, S2, !SP3
NOP; NOP; OR.I16 V5, V1, S2, VP3
NOP; NOP; OR.I16 V5, V1, S2, !VP3

// CHECK: nop;    nop;    or.i16  V5, V1, S2, SP0;       nop
// CHECK: nop;    nop;    or.i16  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    or.i16  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    or.i16  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    or.i16  V5, V1, S2, !VP3;      nop

NOP; NOP; OR.I16 V5, V1, 123
NOP; NOP; OR.I16 V5, V1, 123, SP3
NOP; NOP; OR.I16 V5, V1, 123, !SP3
NOP; NOP; OR.I16 V5, V1, 123, VP3
NOP; NOP; OR.I16 V5, V1, 123, !VP3

// CHECK: nop;    nop;    or.i16  V5, V1, 0x7b, SP0;     nop
// CHECK: nop;    nop;    or.i16  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    or.i16  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    or.i16  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    or.i16  V5, V1, 0x7b, !VP3;     nop

NOP; NOP; OR.U16 V5, V1, V2
NOP; NOP; OR.U16 V5, V1, V2, SP3
NOP; NOP; OR.U16 V5, V1, V2, !SP3
NOP; NOP; OR.U16 V5, V1, V2, VP3
NOP; NOP; OR.U16 V5, V1, V2, !VP3

// CHECK: nop;    nop;    or.u16  V5, V1, V2, SP0;       nop
// CHECK: nop;    nop;    or.u16  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    or.u16  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    or.u16  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    or.u16  V5, V1, V2, !VP3;      nop

NOP; NOP; OR.U16 V5, V1, S2
NOP; NOP; OR.U16 V5, V1, S2, SP3
NOP; NOP; OR.U16 V5, V1, S2, !SP3
NOP; NOP; OR.U16 V5, V1, S2, VP3
NOP; NOP; OR.U16 V5, V1, S2, !VP3

// CHECK: nop;    nop;    or.u16  V5, V1, S2, SP0;       nop
// CHECK: nop;    nop;    or.u16  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    or.u16  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    or.u16  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    or.u16  V5, V1, S2, !VP3;      nop

NOP; NOP; OR.U16 V5, V1, 123
NOP; NOP; OR.U16 V5, V1, 123, SP3
NOP; NOP; OR.U16 V5, V1, 123, !SP3
NOP; NOP; OR.U16 V5, V1, 123, VP3
NOP; NOP; OR.U16 V5, V1, 123, !VP3

// CHECK: nop;    nop;    or.u16  V5, V1, 0x7b, SP0;     nop
// CHECK: nop;    nop;    or.u16  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    or.u16  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    or.u16  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    or.u16  V5, V1, 0x7b, !VP3;    nop

NOP; NOP; OR.I8 V5, V1, V2
NOP; NOP; OR.I8 V5, V1, V2, SP3
NOP; NOP; OR.I8 V5, V1, V2, !SP3
NOP; NOP; OR.I8 V5, V1, V2, VP3
NOP; NOP; OR.I8 V5, V1, V2, !VP3

// CHECK: nop;    nop;    or.i8  V5, V1, V2, SP0;        nop
// CHECK: nop;    nop;    or.i8  V5, V1, V2, SP3;        nop
// CHECK: nop;    nop;    or.i8  V5, V1, V2, !SP3;       nop
// CHECK: nop;    nop;    or.i8  V5, V1, V2, VP3;        nop
// CHECK: nop;    nop;    or.i8  V5, V1, V2, !VP3;       nop

NOP; NOP; OR.I8 V5, V1, S2
NOP; NOP; OR.I8 V5, V1, S2, SP3
NOP; NOP; OR.I8 V5, V1, S2, !SP3
NOP; NOP; OR.I8 V5, V1, S2, VP3
NOP; NOP; OR.I8 V5, V1, S2, !VP3

// CHECK: nop;    nop;    or.i8  V5, V1, S2, SP0;        nop
// CHECK: nop;    nop;    or.i8  V5, V1, S2, SP3;        nop
// CHECK: nop;    nop;    or.i8  V5, V1, S2, !SP3;       nop
// CHECK: nop;    nop;    or.i8  V5, V1, S2, VP3;        nop
// CHECK: nop;    nop;    or.i8  V5, V1, S2, !VP3;       nop

NOP; NOP; OR.I8 V5, V1, 123
NOP; NOP; OR.I8 V5, V1, 123, SP3
NOP; NOP; OR.I8 V5, V1, 123, !SP3
NOP; NOP; OR.I8 V5, V1, 123, VP3
NOP; NOP; OR.I8 V5, V1, 123, !VP3

// CHECK: nop;    nop;    or.i8  V5, V1, 0x7b, SP0;      nop
// CHECK: nop;    nop;    or.i8  V5, V1, 0x7b, SP3;      nop
// CHECK: nop;    nop;    or.i8  V5, V1, 0x7b, !SP3;     nop
// CHECK: nop;    nop;    or.i8  V5, V1, 0x7b, VP3;      nop
// CHECK: nop;    nop;    or.i8  V5, V1, 0x7b, !VP3;     nop

NOP; NOP; OR.U8 V5, V1, V2
NOP; NOP; OR.U8 V5, V1, V2, SP3
NOP; NOP; OR.U8 V5, V1, V2, !SP3
NOP; NOP; OR.U8 V5, V1, V2, VP3
NOP; NOP; OR.U8 V5, V1, V2, !VP3

// CHECK: nop;    nop;    or.u8  V5, V1, V2, SP0;        nop
// CHECK: nop;    nop;    or.u8  V5, V1, V2, SP3;        nop
// CHECK: nop;    nop;    or.u8  V5, V1, V2, !SP3;       nop
// CHECK: nop;    nop;    or.u8  V5, V1, V2, VP3;        nop
// CHECK: nop;    nop;    or.u8  V5, V1, V2, !VP3;       nop

NOP; NOP; OR.U8 V5, V1, S2
NOP; NOP; OR.U8 V5, V1, S2, SP3
NOP; NOP; OR.U8 V5, V1, S2, !SP3
NOP; NOP; OR.U8 V5, V1, S2, VP3
NOP; NOP; OR.U8 V5, V1, S2, !VP3

// CHECK: nop;    nop;    or.u8  V5, V1, S2, SP0;        nop
// CHECK: nop;    nop;    or.u8  V5, V1, S2, SP3;        nop
// CHECK: nop;    nop;    or.u8  V5, V1, S2, !SP3;       nop
// CHECK: nop;    nop;    or.u8  V5, V1, S2, VP3;        nop
// CHECK: nop;    nop;    or.u8  V5, V1, S2, !VP3;       nop

NOP; NOP; OR.U8 V5, V1, 123
NOP; NOP; OR.U8 V5, V1, 123, SP3
NOP; NOP; OR.U8 V5, V1, 123, !SP3
NOP; NOP; OR.U8 V5, V1, 123, VP3
NOP; NOP; OR.U8 V5, V1, 123, !VP3

// CHECK: nop;    nop;    or.u8  V5, V1, 0x7b, SP0;      nop
// CHECK: nop;    nop;    or.u8  V5, V1, 0x7b, SP3;      nop
// CHECK: nop;    nop;    or.u8  V5, V1, 0x7b, !SP3;     nop
// CHECK: nop;    nop;    or.u8  V5, V1, 0x7b, VP3;      nop
// CHECK: nop;    nop;    or.u8  V5, V1, 0x7b, !VP3;     nop
