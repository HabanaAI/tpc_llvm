// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu goya2 %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s

NOP; NOP; XOR.F32 V5, V1, V2
NOP; NOP; XOR.F32 V5, V1, V2, SP3
NOP; NOP; XOR.F32 V5, V1, V2, !SP3
NOP; NOP; XOR.F32 V5, V1, V2, VP3
NOP; NOP; XOR.F32 V5, V1, V2, !VP3

// CHECK: nop;    nop;    xor.f32  V5, V1, V2;       nop
// CHECK: nop;    nop;    xor.f32  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    xor.f32  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    xor.f32  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    xor.f32  V5, V1, V2, !VP3;      nop

NOP; NOP; XOR.F32 V5, V1, S2
NOP; NOP; XOR.F32 V5, V1, S2, SP3
NOP; NOP; XOR.F32 V5, V1, S2, !SP3
NOP; NOP; XOR.F32 V5, V1, S2, VP3
NOP; NOP; XOR.F32 V5, V1, S2, !VP3

// CHECK: nop;    nop;    xor.f32  V5, V1, S2;       nop
// CHECK: nop;    nop;    xor.f32  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    xor.f32  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    xor.f32  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    xor.f32  V5, V1, S2, !VP3;      nop

NOP; NOP; XOR.F32 V5, V1, 0x3f4ccccd
NOP; NOP; XOR.F32 V5, V1, 0x3f4ccccd, SP3
NOP; NOP; XOR.F32 V5, V1, 0x3f4ccccd, !SP3
NOP; NOP; XOR.F32 V5, V1, 0x3f4ccccd, VP3
NOP; NOP; XOR.F32 V5, V1, 0x3f4ccccd, !VP3

// CHECK: nop;    nop;    xor.f32  V5, V1, 0x3f4ccccd;       nop
// CHECK: nop;    nop;    xor.f32  V5, V1, 0x3f4ccccd, SP3;       nop
// CHECK: nop;    nop;    xor.f32  V5, V1, 0x3f4ccccd, !SP3;      nop
// CHECK: nop;    nop;    xor.f32  V5, V1, 0x3f4ccccd, VP3;       nop
// CHECK: nop;    nop;    xor.f32  V5, V1, 0x3f4ccccd, !VP3;      nop

NOP; NOP; XOR.I32 V5, V1, V2
NOP; NOP; XOR.I32 V5, V1, V2, SP3
NOP; NOP; XOR.I32 V5, V1, V2, !SP3
NOP; NOP; XOR.I32 V5, V1, V2, VP3
NOP; NOP; XOR.I32 V5, V1, V2, !VP3

// CHECK: nop;    nop;    xor.i32  V5, V1, V2;       nop
// CHECK: nop;    nop;    xor.i32  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    xor.i32  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    xor.i32  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    xor.i32  V5, V1, V2, !VP3;      nop

NOP; NOP; XOR.I32 V5, V1, S2
NOP; NOP; XOR.I32 V5, V1, S2, SP3
NOP; NOP; XOR.I32 V5, V1, S2, !SP3
NOP; NOP; XOR.I32 V5, V1, S2, VP3
NOP; NOP; XOR.I32 V5, V1, S2, !VP3

// CHECK: nop;    nop;    xor.i32  V5, V1, S2;       nop
// CHECK: nop;    nop;    xor.i32  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    xor.i32  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    xor.i32  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    xor.i32  V5, V1, S2, !VP3;      nop

NOP; NOP; XOR.I32 V5, V1, 123
NOP; NOP; XOR.I32 V5, V1, 123, SP3
NOP; NOP; XOR.I32 V5, V1, 123, !SP3
NOP; NOP; XOR.I32 V5, V1, 123, VP3
NOP; NOP; XOR.I32 V5, V1, 123, !VP3

// CHECK: nop;    nop;    xor.i32  V5, V1, 0x7b;     nop
// CHECK: nop;    nop;    xor.i32  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    xor.i32  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    xor.i32  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    xor.i32  V5, V1, 0x7b, !VP3;    nop

NOP; NOP; XOR.U32 V5, V1, V2
NOP; NOP; XOR.U32 V5, V1, V2, SP3
NOP; NOP; XOR.U32 V5, V1, V2, !SP3
NOP; NOP; XOR.U32 V5, V1, V2, VP3
NOP; NOP; XOR.U32 V5, V1, V2, !VP3

// CHECK: nop;    nop;    xor.u32  V5, V1, V2;       nop
// CHECK: nop;    nop;    xor.u32  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    xor.u32  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    xor.u32  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    xor.u32  V5, V1, V2, !VP3;      nop

NOP; NOP; XOR.U32 V5, V1, S2
NOP; NOP; XOR.U32 V5, V1, S2, SP3
NOP; NOP; XOR.U32 V5, V1, S2, !SP3
NOP; NOP; XOR.U32 V5, V1, S2, VP3
NOP; NOP; XOR.U32 V5, V1, S2, !VP3

// CHECK: nop;    nop;    xor.u32  V5, V1, S2;       nop
// CHECK: nop;    nop;    xor.u32  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    xor.u32  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    xor.u32  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    xor.u32  V5, V1, S2, !VP3;      nop

NOP; NOP; XOR.U32 V5, V1, 123
NOP; NOP; XOR.U32 V5, V1, 123, SP3
NOP; NOP; XOR.U32 V5, V1, 123, !SP3
NOP; NOP; XOR.U32 V5, V1, 123, VP3
NOP; NOP; XOR.U32 V5, V1, 123, !VP3

// CHECK: nop;    nop;    xor.u32  V5, V1, 0x7b;     nop
// CHECK: nop;    nop;    xor.u32  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    xor.u32  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    xor.u32  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    xor.u32  V5, V1, 0x7b, !VP3;    nop

NOP; NOP; XOR.I16 V5, V1, V2
NOP; NOP; XOR.I16 V5, V1, V2, SP3
NOP; NOP; XOR.I16 V5, V1, V2, !SP3
NOP; NOP; XOR.I16 V5, V1, V2, VP3
NOP; NOP; XOR.I16 V5, V1, V2, !VP3

// CHECK: nop;    nop;    xor.i16  V5, V1, V2;       nop
// CHECK: nop;    nop;    xor.i16  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    xor.i16  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    xor.i16  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    xor.i16  V5, V1, V2, !VP3;      nop

NOP; NOP; XOR.I16 V5, V1, S2
NOP; NOP; XOR.I16 V5, V1, S2, SP3
NOP; NOP; XOR.I16 V5, V1, S2, !SP3
NOP; NOP; XOR.I16 V5, V1, S2, VP3
NOP; NOP; XOR.I16 V5, V1, S2, !VP3

// CHECK: nop;    nop;    xor.i16  V5, V1, S2;       nop
// CHECK: nop;    nop;    xor.i16  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    xor.i16  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    xor.i16  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    xor.i16  V5, V1, S2, !VP3;      nop

NOP; NOP; XOR.I16 V5, V1, 123
NOP; NOP; XOR.I16 V5, V1, 123, SP3
NOP; NOP; XOR.I16 V5, V1, 123, !SP3
NOP; NOP; XOR.I16 V5, V1, 123, VP3
NOP; NOP; XOR.I16 V5, V1, 123, !VP3

// CHECK: nop;    nop;    xor.i16  V5, V1, 0x7b;     nop
// CHECK: nop;    nop;    xor.i16  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    xor.i16  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    xor.i16  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    xor.i16  V5, V1, 0x7b, !VP3;     nop

NOP; NOP; XOR.U16 V5, V1, V2
NOP; NOP; XOR.U16 V5, V1, V2, SP3
NOP; NOP; XOR.U16 V5, V1, V2, !SP3
NOP; NOP; XOR.U16 V5, V1, V2, VP3
NOP; NOP; XOR.U16 V5, V1, V2, !VP3

// CHECK: nop;    nop;    xor.u16  V5, V1, V2;       nop
// CHECK: nop;    nop;    xor.u16  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    xor.u16  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    xor.u16  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    xor.u16  V5, V1, V2, !VP3;      nop

NOP; NOP; XOR.U16 V5, V1, S2
NOP; NOP; XOR.U16 V5, V1, S2, SP3
NOP; NOP; XOR.U16 V5, V1, S2, !SP3
NOP; NOP; XOR.U16 V5, V1, S2, VP3
NOP; NOP; XOR.U16 V5, V1, S2, !VP3

// CHECK: nop;    nop;    xor.u16  V5, V1, S2;       nop
// CHECK: nop;    nop;    xor.u16  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    xor.u16  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    xor.u16  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    xor.u16  V5, V1, S2, !VP3;      nop

NOP; NOP; XOR.U16 V5, V1, 123
NOP; NOP; XOR.U16 V5, V1, 123, SP3
NOP; NOP; XOR.U16 V5, V1, 123, !SP3
NOP; NOP; XOR.U16 V5, V1, 123, VP3
NOP; NOP; XOR.U16 V5, V1, 123, !VP3

// CHECK: nop;    nop;    xor.u16  V5, V1, 0x7b;     nop
// CHECK: nop;    nop;    xor.u16  V5, V1, 0x7b, SP3;     nop
// CHECK: nop;    nop;    xor.u16  V5, V1, 0x7b, !SP3;    nop
// CHECK: nop;    nop;    xor.u16  V5, V1, 0x7b, VP3;     nop
// CHECK: nop;    nop;    xor.u16  V5, V1, 0x7b, !VP3;    nop

NOP; NOP; XOR.I8 V5, V1, V2
NOP; NOP; XOR.I8 V5, V1, V2, SP3
NOP; NOP; XOR.I8 V5, V1, V2, !SP3
NOP; NOP; XOR.I8 V5, V1, V2, VP3
NOP; NOP; XOR.I8 V5, V1, V2, !VP3

// CHECK: nop;    nop;    xor.i8  V5, V1, V2;        nop
// CHECK: nop;    nop;    xor.i8  V5, V1, V2, SP3;        nop
// CHECK: nop;    nop;    xor.i8  V5, V1, V2, !SP3;       nop
// CHECK: nop;    nop;    xor.i8  V5, V1, V2, VP3;        nop
// CHECK: nop;    nop;    xor.i8  V5, V1, V2, !VP3;       nop

NOP; NOP; XOR.I8 V5, V1, S2
NOP; NOP; XOR.I8 V5, V1, S2, SP3
NOP; NOP; XOR.I8 V5, V1, S2, !SP3
NOP; NOP; XOR.I8 V5, V1, S2, VP3
NOP; NOP; XOR.I8 V5, V1, S2, !VP3

// CHECK: nop;    nop;    xor.i8  V5, V1, S2;        nop
// CHECK: nop;    nop;    xor.i8  V5, V1, S2, SP3;        nop
// CHECK: nop;    nop;    xor.i8  V5, V1, S2, !SP3;       nop
// CHECK: nop;    nop;    xor.i8  V5, V1, S2, VP3;        nop
// CHECK: nop;    nop;    xor.i8  V5, V1, S2, !VP3;       nop

NOP; NOP; XOR.I8 V5, V1, 123
NOP; NOP; XOR.I8 V5, V1, 123, SP3
NOP; NOP; XOR.I8 V5, V1, 123, !SP3
NOP; NOP; XOR.I8 V5, V1, 123, VP3
NOP; NOP; XOR.I8 V5, V1, 123, !VP3

// CHECK: nop;    nop;    xor.i8  V5, V1, 0x7b;      nop
// CHECK: nop;    nop;    xor.i8  V5, V1, 0x7b, SP3;      nop
// CHECK: nop;    nop;    xor.i8  V5, V1, 0x7b, !SP3;     nop
// CHECK: nop;    nop;    xor.i8  V5, V1, 0x7b, VP3;      nop
// CHECK: nop;    nop;    xor.i8  V5, V1, 0x7b, !VP3;     nop

NOP; NOP; XOR.U8 V5, V1, V2
NOP; NOP; XOR.U8 V5, V1, V2, SP3
NOP; NOP; XOR.U8 V5, V1, V2, !SP3
NOP; NOP; XOR.U8 V5, V1, V2, VP3
NOP; NOP; XOR.U8 V5, V1, V2, !VP3

// CHECK: nop;    nop;    xor.u8  V5, V1, V2;        nop
// CHECK: nop;    nop;    xor.u8  V5, V1, V2, SP3;        nop
// CHECK: nop;    nop;    xor.u8  V5, V1, V2, !SP3;       nop
// CHECK: nop;    nop;    xor.u8  V5, V1, V2, VP3;        nop
// CHECK: nop;    nop;    xor.u8  V5, V1, V2, !VP3;       nop

NOP; NOP; XOR.U8 V5, V1, S2
NOP; NOP; XOR.U8 V5, V1, S2, SP3
NOP; NOP; XOR.U8 V5, V1, S2, !SP3
NOP; NOP; XOR.U8 V5, V1, S2, VP3
NOP; NOP; XOR.U8 V5, V1, S2, !VP3

// CHECK: nop;    nop;    xor.u8  V5, V1, S2;        nop
// CHECK: nop;    nop;    xor.u8  V5, V1, S2, SP3;        nop
// CHECK: nop;    nop;    xor.u8  V5, V1, S2, !SP3;       nop
// CHECK: nop;    nop;    xor.u8  V5, V1, S2, VP3;        nop
// CHECK: nop;    nop;    xor.u8  V5, V1, S2, !VP3;       nop

NOP; NOP; XOR.U8 V5, V1, 123
NOP; NOP; XOR.U8 V5, V1, 123, SP3
NOP; NOP; XOR.U8 V5, V1, 123, !SP3
NOP; NOP; XOR.U8 V5, V1, 123, VP3
NOP; NOP; XOR.U8 V5, V1, 123, !VP3

// CHECK: nop;    nop;    xor.u8  V5, V1, 0x7b;      nop
// CHECK: nop;    nop;    xor.u8  V5, V1, 0x7b, SP3;      nop
// CHECK: nop;    nop;    xor.u8  V5, V1, 0x7b, !SP3;     nop
// CHECK: nop;    nop;    xor.u8  V5, V1, 0x7b, VP3;      nop
// CHECK: nop;    nop;    xor.u8  V5, V1, 0x7b, !VP3;     nop
