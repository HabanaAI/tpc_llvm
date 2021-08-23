// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm -mcpu gaudi %t.o | FileCheck %s



NOP; NOP; MAX.F32 V5, V1, V2
NOP; NOP; MAX.F32 V5, V1, V2, SP3
NOP; NOP; MAX.F32 V5, V1, V2, !SP3
NOP; NOP; MAX.F32 V5, V1, V2, VP3
NOP; NOP; MAX.F32 V5, V1, V2, !VP3

// CHECK: nop;    nop;    max.f32  V5, V1, V2, SP0;       nop
// CHECK: nop;    nop;    max.f32  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    max.f32  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    max.f32  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    max.f32  V5, V1, V2, !VP3;      nop

NOP; NOP; MAX.F32 V5, V1, S2
NOP; NOP; MAX.F32 V5, V1, S2, SP3
NOP; NOP; MAX.F32 V5, V1, S2, !SP3
NOP; NOP; MAX.F32 V5, V1, S2, VP3
NOP; NOP; MAX.F32 V5, V1, S2, !VP3

// CHECK: nop;    nop;    max.f32  V5, V1, S2, SP0;       nop
// CHECK: nop;    nop;    max.f32  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    max.f32  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    max.f32  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    max.f32  V5, V1, S2, !VP3;      nop

NOP; NOP; MAX.F32 V5, V1, 0x3f4ccccd
NOP; NOP; MAX.F32 V5, V1, 0x3f4ccccd, SP3
NOP; NOP; MAX.F32 V5, V1, 0x3f4ccccd, !SP3
NOP; NOP; MAX.F32 V5, V1, 0x3f4ccccd, VP3
NOP; NOP; MAX.F32 V5, V1, 0x3f4ccccd, !VP3

// CHECK: nop;    nop;    max.f32  V5, V1, 0x3f4ccccd, SP0;       nop
// CHECK: nop;    nop;    max.f32  V5, V1, 0x3f4ccccd, SP3;       nop
// CHECK: nop;    nop;    max.f32  V5, V1, 0x3f4ccccd, !SP3;      nop
// CHECK: nop;    nop;    max.f32  V5, V1, 0x3f4ccccd, VP3;       nop
// CHECK: nop;    nop;    max.f32  V5, V1, 0x3f4ccccd, !VP3;      nop

NOP; NOP; MAX.I32 V5, V1, V2
NOP; NOP; MAX.I32 V5, V1, V2, SP3
NOP; NOP; MAX.I32 V5, V1, V2, !SP3
NOP; NOP; MAX.I32 V5, V1, V2, VP3
NOP; NOP; MAX.I32 V5, V1, V2, !VP3

// CHECK: nop;    nop;    max.i32  V5, V1, V2, SP0;       nop
// CHECK: nop;    nop;    max.i32  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    max.i32  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    max.i32  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    max.i32  V5, V1, V2, !VP3;      nop

NOP; NOP; MAX.I32 V5, V1, S2
NOP; NOP; MAX.I32 V5, V1, S2, SP3
NOP; NOP; MAX.I32 V5, V1, S2, !SP3
NOP; NOP; MAX.I32 V5, V1, S2, VP3
NOP; NOP; MAX.I32 V5, V1, S2, !VP3

// CHECK: nop;    nop;    max.i32  V5, V1, S2, SP0;       nop
// CHECK: nop;    nop;    max.i32  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    max.i32  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    max.i32  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    max.i32  V5, V1, S2, !VP3;      nop

NOP; NOP; MAX.I32 V5, V1, 123
NOP; NOP; MAX.I32 V5, V1, 123, SP3
NOP; NOP; MAX.I32 V5, V1, 123, !SP3
NOP; NOP; MAX.I32 V5, V1, 123, VP3
NOP; NOP; MAX.I32 V5, V1, 123, !VP3

// CHECK: nop;    nop;    max.i32  V5, V1, 0x7b, SP0;     nop
// CHECK: nop;    nop;    max.i32  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    max.i32  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    max.i32  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    max.i32  V5, V1, 0x7b, !VP3;    nop

NOP; NOP; MAX.U32 V5, V1, V2
NOP; NOP; MAX.U32 V5, V1, V2, SP3
NOP; NOP; MAX.U32 V5, V1, V2, !SP3
NOP; NOP; MAX.U32 V5, V1, V2, VP3
NOP; NOP; MAX.U32 V5, V1, V2, !VP3

// CHECK: nop;    nop;    max.u32  V5, V1, V2, SP0;       nop
// CHECK: nop;    nop;    max.u32  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    max.u32  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    max.u32  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    max.u32  V5, V1, V2, !VP3;      nop

NOP; NOP; MAX.U32 V5, V1, S2
NOP; NOP; MAX.U32 V5, V1, S2, SP3
NOP; NOP; MAX.U32 V5, V1, S2, !SP3
NOP; NOP; MAX.U32 V5, V1, S2, VP3
NOP; NOP; MAX.U32 V5, V1, S2, !VP3

// CHECK: nop;    nop;    max.u32  V5, V1, S2, SP0;       nop
// CHECK: nop;    nop;    max.u32  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    max.u32  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    max.u32  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    max.u32  V5, V1, S2, !VP3;      nop

NOP; NOP; MAX.U32 V5, V1, 123
NOP; NOP; MAX.U32 V5, V1, 123, SP3
NOP; NOP; MAX.U32 V5, V1, 123, !SP3
NOP; NOP; MAX.U32 V5, V1, 123, VP3
NOP; NOP; MAX.U32 V5, V1, 123, !VP3

// CHECK: nop;    nop;    max.u32  V5, V1, 0x7b, SP0;     nop
// CHECK: nop;    nop;    max.u32  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    max.u32  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    max.u32  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    max.u32  V5, V1, 0x7b, !VP3;    nop

NOP; NOP; MAX.I16 V5, V1, V2
NOP; NOP; MAX.I16 V5, V1, V2, SP3
NOP; NOP; MAX.I16 V5, V1, V2, !SP3
NOP; NOP; MAX.I16 V5, V1, V2, VP3
NOP; NOP; MAX.I16 V5, V1, V2, !VP3

// CHECK: nop;    nop;    max.i16  V5, V1, V2, SP0;       nop
// CHECK: nop;    nop;    max.i16  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    max.i16  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    max.i16  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    max.i16  V5, V1, V2, !VP3;      nop

NOP; NOP; MAX.I16 V5, V1, S2
NOP; NOP; MAX.I16 V5, V1, S2, SP3
NOP; NOP; MAX.I16 V5, V1, S2, !SP3
NOP; NOP; MAX.I16 V5, V1, S2, VP3
NOP; NOP; MAX.I16 V5, V1, S2, !VP3

// CHECK: nop;    nop;    max.i16  V5, V1, S2, SP0;       nop
// CHECK: nop;    nop;    max.i16  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    max.i16  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    max.i16  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    max.i16  V5, V1, S2, !VP3;      nop

NOP; NOP; MAX.I16 V5, V1, 123
NOP; NOP; MAX.I16 V5, V1, 123, SP3
NOP; NOP; MAX.I16 V5, V1, 123, !SP3
NOP; NOP; MAX.I16 V5, V1, 123, VP3
NOP; NOP; MAX.I16 V5, V1, 123, !VP3

// CHECK: nop;    nop;    max.i16  V5, V1, 0x7b, SP0;     nop
// CHECK: nop;    nop;    max.i16  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    max.i16  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    max.i16  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    max.i16  V5, V1, 0x7b, !VP3;     nop

NOP; NOP; MAX.U16 V5, V1, V2
NOP; NOP; MAX.U16 V5, V1, V2, SP3
NOP; NOP; MAX.U16 V5, V1, V2, !SP3
NOP; NOP; MAX.U16 V5, V1, V2, VP3
NOP; NOP; MAX.U16 V5, V1, V2, !VP3

// CHECK: nop;    nop;    max.u16  V5, V1, V2, SP0;       nop
// CHECK: nop;    nop;    max.u16  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    max.u16  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    max.u16  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    max.u16  V5, V1, V2, !VP3;      nop

NOP; NOP; MAX.U16 V5, V1, S2
NOP; NOP; MAX.U16 V5, V1, S2, SP3
NOP; NOP; MAX.U16 V5, V1, S2, !SP3
NOP; NOP; MAX.U16 V5, V1, S2, VP3
NOP; NOP; MAX.U16 V5, V1, S2, !VP3

// CHECK: nop;    nop;    max.u16  V5, V1, S2, SP0;       nop
// CHECK: nop;    nop;    max.u16  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    max.u16  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    max.u16  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    max.u16  V5, V1, S2, !VP3;      nop

NOP; NOP; MAX.U16 V5, V1, 123
NOP; NOP; MAX.U16 V5, V1, 123, SP3
NOP; NOP; MAX.U16 V5, V1, 123, !SP3
NOP; NOP; MAX.U16 V5, V1, 123, VP3
NOP; NOP; MAX.U16 V5, V1, 123, !VP3

// CHECK: nop;    nop;    max.u16  V5, V1, 0x7b, SP0;     nop
// CHECK: nop;    nop;    max.u16  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    max.u16  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    max.u16  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    max.u16  V5, V1, 0x7b, !VP3;    nop

NOP; NOP; MAX.I8 V5, V1, V2
NOP; NOP; MAX.I8 V5, V1, V2, SP3
NOP; NOP; MAX.I8 V5, V1, V2, !SP3
NOP; NOP; MAX.I8 V5, V1, V2, VP3
NOP; NOP; MAX.I8 V5, V1, V2, !VP3

// CHECK: nop;    nop;    max.i8  V5, V1, V2, SP0;        nop
// CHECK: nop;    nop;    max.i8  V5, V1, V2, SP3;        nop
// CHECK: nop;    nop;    max.i8  V5, V1, V2, !SP3;       nop
// CHECK: nop;    nop;    max.i8  V5, V1, V2, VP3;        nop
// CHECK: nop;    nop;    max.i8  V5, V1, V2, !VP3;       nop

NOP; NOP; MAX.I8 V5, V1, S2
NOP; NOP; MAX.I8 V5, V1, S2, SP3
NOP; NOP; MAX.I8 V5, V1, S2, !SP3
NOP; NOP; MAX.I8 V5, V1, S2, VP3
NOP; NOP; MAX.I8 V5, V1, S2, !VP3

// CHECK: nop;    nop;    max.i8  V5, V1, S2, SP0;        nop
// CHECK: nop;    nop;    max.i8  V5, V1, S2, SP3;        nop
// CHECK: nop;    nop;    max.i8  V5, V1, S2, !SP3;       nop
// CHECK: nop;    nop;    max.i8  V5, V1, S2, VP3;        nop
// CHECK: nop;    nop;    max.i8  V5, V1, S2, !VP3;       nop

NOP; NOP; MAX.I8 V5, V1, 123
NOP; NOP; MAX.I8 V5, V1, 123, SP3
NOP; NOP; MAX.I8 V5, V1, 123, !SP3
NOP; NOP; MAX.I8 V5, V1, 123, VP3
NOP; NOP; MAX.I8 V5, V1, 123, !VP3

// CHECK: nop;    nop;    max.i8  V5, V1, 0x7b, SP0;      nop
// CHECK: nop;    nop;    max.i8  V5, V1, 0x7b, SP3;      nop
// CHECK: nop;    nop;    max.i8  V5, V1, 0x7b, !SP3;     nop
// CHECK: nop;    nop;    max.i8  V5, V1, 0x7b, VP3;      nop
// CHECK: nop;    nop;    max.i8  V5, V1, 0x7b, !VP3;     nop

NOP; NOP; MAX.U8 V5, V1, V2
NOP; NOP; MAX.U8 V5, V1, V2, SP3
NOP; NOP; MAX.U8 V5, V1, V2, !SP3
NOP; NOP; MAX.U8 V5, V1, V2, VP3
NOP; NOP; MAX.U8 V5, V1, V2, !VP3

// CHECK: nop;    nop;    max.u8  V5, V1, V2, SP0;        nop
// CHECK: nop;    nop;    max.u8  V5, V1, V2, SP3;        nop
// CHECK: nop;    nop;    max.u8  V5, V1, V2, !SP3;       nop
// CHECK: nop;    nop;    max.u8  V5, V1, V2, VP3;        nop
// CHECK: nop;    nop;    max.u8  V5, V1, V2, !VP3;       nop

NOP; NOP; MAX.U8 V5, V1, S2
NOP; NOP; MAX.U8 V5, V1, S2, SP3
NOP; NOP; MAX.U8 V5, V1, S2, !SP3
NOP; NOP; MAX.U8 V5, V1, S2, VP3
NOP; NOP; MAX.U8 V5, V1, S2, !VP3

// CHECK: nop;    nop;    max.u8  V5, V1, S2, SP0;        nop
// CHECK: nop;    nop;    max.u8  V5, V1, S2, SP3;        nop
// CHECK: nop;    nop;    max.u8  V5, V1, S2, !SP3;       nop
// CHECK: nop;    nop;    max.u8  V5, V1, S2, VP3;        nop
// CHECK: nop;    nop;    max.u8  V5, V1, S2, !VP3;       nop

NOP; NOP; MAX.U8 V5, V1, 123
NOP; NOP; MAX.U8 V5, V1, 123, SP3
NOP; NOP; MAX.U8 V5, V1, 123, !SP3
NOP; NOP; MAX.U8 V5, V1, 123, VP3
NOP; NOP; MAX.U8 V5, V1, 123, !VP3

// CHECK: nop;    nop;    max.u8  V5, V1, 0x7b, SP0;      nop
// CHECK: nop;    nop;    max.u8  V5, V1, 0x7b, SP3;      nop
// CHECK: nop;    nop;    max.u8  V5, V1, 0x7b, !SP3;     nop
// CHECK: nop;    nop;    max.u8  V5, V1, 0x7b, VP3;      nop
// CHECK: nop;    nop;    max.u8  V5, V1, 0x7b, !VP3;     nop
