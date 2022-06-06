// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu goya2 %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s

NOP; NOP; AND.F32 V5, V1, V2
NOP; NOP; AND.F32 V5, V1, V2, SP3
NOP; NOP; AND.F32 V5, V1, V2, !SP3
NOP; NOP; AND.F32 V5, V1, V2, VP3
NOP; NOP; AND.F32 V5, V1, V2, !VP3

// CHECK: nop;    nop;    and.f32  V5, V1, V2;       nop
// CHECK: nop;    nop;    and.f32  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    and.f32  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    and.f32  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    and.f32  V5, V1, V2, !VP3;      nop

NOP; NOP; AND.F32 V5, V1, S2
NOP; NOP; AND.F32 V5, V1, S2, SP3
NOP; NOP; AND.F32 V5, V1, S2, !SP3
NOP; NOP; AND.F32 V5, V1, S2, VP3
NOP; NOP; AND.F32 V5, V1, S2, !VP3

// CHECK: nop;    nop;    and.f32  V5, V1, S2;       nop
// CHECK: nop;    nop;    and.f32  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    and.f32  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    and.f32  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    and.f32  V5, V1, S2, !VP3;      nop

NOP; NOP; AND.F32 V5, V1, 0x3f4ccccd
NOP; NOP; AND.F32 V5, V1, 0x3f4ccccd, SP3
NOP; NOP; AND.F32 V5, V1, 0x3f4ccccd, !SP3
NOP; NOP; AND.F32 V5, V1, 0x3f4ccccd, VP3
NOP; NOP; AND.F32 V5, V1, 0x3f4ccccd, !VP3

// CHECK: nop;    nop;    and.f32  V5, V1, 0x3f4ccccd;       nop
// CHECK: nop;    nop;    and.f32  V5, V1, 0x3f4ccccd, SP3;       nop
// CHECK: nop;    nop;    and.f32  V5, V1, 0x3f4ccccd, !SP3;      nop
// CHECK: nop;    nop;    and.f32  V5, V1, 0x3f4ccccd, VP3;       nop
// CHECK: nop;    nop;    and.f32  V5, V1, 0x3f4ccccd, !VP3;      nop

NOP; NOP; AND.I32 V5, V1, V2
NOP; NOP; AND.I32 V5, V1, V2, SP3
NOP; NOP; AND.I32 V5, V1, V2, !SP3
NOP; NOP; AND.I32 V5, V1, V2, VP3
NOP; NOP; AND.I32 V5, V1, V2, !VP3

// CHECK: nop;    nop;    and.i32  V5, V1, V2;       nop
// CHECK: nop;    nop;    and.i32  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    and.i32  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    and.i32  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    and.i32  V5, V1, V2, !VP3;      nop

NOP; NOP; AND.I32 V5, V1, S2
NOP; NOP; AND.I32 V5, V1, S2, SP3
NOP; NOP; AND.I32 V5, V1, S2, !SP3
NOP; NOP; AND.I32 V5, V1, S2, VP3
NOP; NOP; AND.I32 V5, V1, S2, !VP3

// CHECK: nop;    nop;    and.i32  V5, V1, S2;       nop
// CHECK: nop;    nop;    and.i32  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    and.i32  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    and.i32  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    and.i32  V5, V1, S2, !VP3;      nop

NOP; NOP; AND.I32 V5, V1, 123
NOP; NOP; AND.I32 V5, V1, 123, SP3
NOP; NOP; AND.I32 V5, V1, 123, !SP3
NOP; NOP; AND.I32 V5, V1, 123, VP3
NOP; NOP; AND.I32 V5, V1, 123, !VP3

// CHECK: nop;    nop;    and.i32  V5, V1, 0x7b;     nop
// CHECK: nop;    nop;    and.i32  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    and.i32  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    and.i32  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    and.i32  V5, V1, 0x7b, !VP3;    nop

NOP; NOP; AND.U32 V5, V1, V2
NOP; NOP; AND.U32 V5, V1, V2, SP3
NOP; NOP; AND.U32 V5, V1, V2, !SP3
NOP; NOP; AND.U32 V5, V1, V2, VP3
NOP; NOP; AND.U32 V5, V1, V2, !VP3

// CHECK: nop;    nop;    and.u32  V5, V1, V2;       nop
// CHECK: nop;    nop;    and.u32  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    and.u32  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    and.u32  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    and.u32  V5, V1, V2, !VP3;      nop

NOP; NOP; AND.U32 V5, V1, S2
NOP; NOP; AND.U32 V5, V1, S2, SP3
NOP; NOP; AND.U32 V5, V1, S2, !SP3
NOP; NOP; AND.U32 V5, V1, S2, VP3
NOP; NOP; AND.U32 V5, V1, S2, !VP3

// CHECK: nop;    nop;    and.u32  V5, V1, S2;       nop
// CHECK: nop;    nop;    and.u32  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    and.u32  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    and.u32  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    and.u32  V5, V1, S2, !VP3;      nop

NOP; NOP; AND.U32 V5, V1, 123
NOP; NOP; AND.U32 V5, V1, 123, SP3
NOP; NOP; AND.U32 V5, V1, 123, !SP3
NOP; NOP; AND.U32 V5, V1, 123, VP3
NOP; NOP; AND.U32 V5, V1, 123, !VP3

// CHECK: nop;    nop;    and.u32  V5, V1, 0x7b;     nop
// CHECK: nop;    nop;    and.u32  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    and.u32  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    and.u32  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    and.u32  V5, V1, 0x7b, !VP3;    nop

NOP; NOP; AND.I16 V5, V1, V2
NOP; NOP; AND.I16 V5, V1, V2, SP3
NOP; NOP; AND.I16 V5, V1, V2, !SP3
NOP; NOP; AND.I16 V5, V1, V2, VP3
NOP; NOP; AND.I16 V5, V1, V2, !VP3

// CHECK: nop;    nop;    and.i16  V5, V1, V2;       nop
// CHECK: nop;    nop;    and.i16  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    and.i16  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    and.i16  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    and.i16  V5, V1, V2, !VP3;      nop

NOP; NOP; AND.I16 V5, V1, S2
NOP; NOP; AND.I16 V5, V1, S2, SP3
NOP; NOP; AND.I16 V5, V1, S2, !SP3
NOP; NOP; AND.I16 V5, V1, S2, VP3
NOP; NOP; AND.I16 V5, V1, S2, !VP3

// CHECK: nop;    nop;    and.i16  V5, V1, S2;       nop
// CHECK: nop;    nop;    and.i16  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    and.i16  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    and.i16  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    and.i16  V5, V1, S2, !VP3;      nop

NOP; NOP; AND.I16 V5, V1, 123
NOP; NOP; AND.I16 V5, V1, 123, SP3
NOP; NOP; AND.I16 V5, V1, 123, !SP3
NOP; NOP; AND.I16 V5, V1, 123, VP3
NOP; NOP; AND.I16 V5, V1, 123, !VP3

// CHECK: nop;    nop;    and.i16  V5, V1, 0x7b;     nop
// CHECK: nop;    nop;    and.i16  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    and.i16  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    and.i16  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    and.i16  V5, V1, 0x7b, !VP3;     nop

NOP; NOP; AND.U16 V5, V1, V2
NOP; NOP; AND.U16 V5, V1, V2, SP3
NOP; NOP; AND.U16 V5, V1, V2, !SP3
NOP; NOP; AND.U16 V5, V1, V2, VP3
NOP; NOP; AND.U16 V5, V1, V2, !VP3

// CHECK: nop;    nop;    and.u16  V5, V1, V2;       nop
// CHECK: nop;    nop;    and.u16  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    and.u16  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    and.u16  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    and.u16  V5, V1, V2, !VP3;      nop

NOP; NOP; AND.U16 V5, V1, S2
NOP; NOP; AND.U16 V5, V1, S2, SP3
NOP; NOP; AND.U16 V5, V1, S2, !SP3
NOP; NOP; AND.U16 V5, V1, S2, VP3
NOP; NOP; AND.U16 V5, V1, S2, !VP3

// CHECK: nop;    nop;    and.u16  V5, V1, S2;       nop
// CHECK: nop;    nop;    and.u16  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    and.u16  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    and.u16  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    and.u16  V5, V1, S2, !VP3;      nop

NOP; NOP; AND.U16 V5, V1, 123
NOP; NOP; AND.U16 V5, V1, 123, SP3
NOP; NOP; AND.U16 V5, V1, 123, !SP3
NOP; NOP; AND.U16 V5, V1, 123, VP3
NOP; NOP; AND.U16 V5, V1, 123, !VP3

// CHECK: nop;    nop;    and.u16  V5, V1, 0x7b;     nop
// CHECK: nop;    nop;    and.u16  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    and.u16  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    and.u16  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    and.u16  V5, V1, 0x7b, !VP3;    nop

NOP; NOP; AND.I8 V5, V1, V2
NOP; NOP; AND.I8 V5, V1, V2, SP3
NOP; NOP; AND.I8 V5, V1, V2, !SP3
NOP; NOP; AND.I8 V5, V1, V2, VP3
NOP; NOP; AND.I8 V5, V1, V2, !VP3

// CHECK: nop;    nop;    and.i8  V5, V1, V2;        nop
// CHECK: nop;    nop;    and.i8  V5, V1, V2, SP3;        nop
// CHECK: nop;    nop;    and.i8  V5, V1, V2, !SP3;       nop
// CHECK: nop;    nop;    and.i8  V5, V1, V2, VP3;        nop
// CHECK: nop;    nop;    and.i8  V5, V1, V2, !VP3;       nop

NOP; NOP; AND.I8 V5, V1, S2
NOP; NOP; AND.I8 V5, V1, S2, SP3
NOP; NOP; AND.I8 V5, V1, S2, !SP3
NOP; NOP; AND.I8 V5, V1, S2, VP3
NOP; NOP; AND.I8 V5, V1, S2, !VP3

// CHECK: nop;    nop;    and.i8  V5, V1, S2;        nop
// CHECK: nop;    nop;    and.i8  V5, V1, S2, SP3;        nop
// CHECK: nop;    nop;    and.i8  V5, V1, S2, !SP3;       nop
// CHECK: nop;    nop;    and.i8  V5, V1, S2, VP3;        nop
// CHECK: nop;    nop;    and.i8  V5, V1, S2, !VP3;       nop

NOP; NOP; AND.I8 V5, V1, 123
NOP; NOP; AND.I8 V5, V1, 123, SP3
NOP; NOP; AND.I8 V5, V1, 123, !SP3
NOP; NOP; AND.I8 V5, V1, 123, VP3
NOP; NOP; AND.I8 V5, V1, 123, !VP3

// CHECK: nop;    nop;    and.i8  V5, V1, 0x7b;      nop
// CHECK: nop;    nop;    and.i8  V5, V1, 0x7b, SP3;      nop
// CHECK: nop;    nop;    and.i8  V5, V1, 0x7b, !SP3;     nop
// CHECK: nop;    nop;    and.i8  V5, V1, 0x7b, VP3;      nop
// CHECK: nop;    nop;    and.i8  V5, V1, 0x7b, !VP3;     nop

NOP; NOP; AND.U8 V5, V1, V2
NOP; NOP; AND.U8 V5, V1, V2, SP3
NOP; NOP; AND.U8 V5, V1, V2, !SP3
NOP; NOP; AND.U8 V5, V1, V2, VP3
NOP; NOP; AND.U8 V5, V1, V2, !VP3

// CHECK: nop;    nop;    and.u8  V5, V1, V2;        nop
// CHECK: nop;    nop;    and.u8  V5, V1, V2, SP3;        nop
// CHECK: nop;    nop;    and.u8  V5, V1, V2, !SP3;       nop
// CHECK: nop;    nop;    and.u8  V5, V1, V2, VP3;        nop
// CHECK: nop;    nop;    and.u8  V5, V1, V2, !VP3;       nop

NOP; NOP; AND.U8 V5, V1, S2
NOP; NOP; AND.U8 V5, V1, S2, SP3
NOP; NOP; AND.U8 V5, V1, S2, !SP3
NOP; NOP; AND.U8 V5, V1, S2, VP3
NOP; NOP; AND.U8 V5, V1, S2, !VP3

// CHECK: nop;    nop;    and.u8  V5, V1, S2;        nop
// CHECK: nop;    nop;    and.u8  V5, V1, S2, SP3;        nop
// CHECK: nop;    nop;    and.u8  V5, V1, S2, !SP3;       nop
// CHECK: nop;    nop;    and.u8  V5, V1, S2, VP3;        nop
// CHECK: nop;    nop;    and.u8  V5, V1, S2, !VP3;       nop

NOP; NOP; AND.U8 V5, V1, 123
NOP; NOP; AND.U8 V5, V1, 123, SP3
NOP; NOP; AND.U8 V5, V1, 123, !SP3
NOP; NOP; AND.U8 V5, V1, 123, VP3
NOP; NOP; AND.U8 V5, V1, 123, !VP3

// CHECK: nop;    nop;    and.u8  V5, V1, 0x7b;      nop
// CHECK: nop;    nop;    and.u8  V5, V1, 0x7b, SP3;      nop
// CHECK: nop;    nop;    and.u8  V5, V1, 0x7b, !SP3;     nop
// CHECK: nop;    nop;    and.u8  V5, V1, 0x7b, VP3;      nop
// CHECK: nop;    nop;    and.u8  V5, V1, 0x7b, !VP3;     nop
