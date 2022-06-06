// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu goya2 %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s

NOP; MIN.F32 S5, S1, S2
NOP; MIN.F32 S5, S1, S2, SP3
NOP; MIN.F32 S5, S1, S2, !SP3

// CHECK: nop;    min.f32  S5, S1, S2;       nop;    nop
// CHECK: nop;    min.f32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    min.f32  S5, S1, S2, !SP3;      nop;    nop

NOP; MIN.F32 S5, S1, 0x3f4ccccd
NOP; MIN.F32 S5, S1, 0x3f4ccccd, SP3
NOP; MIN.F32 S5, S1, 0x3f4ccccd, !SP3

// CHECK: nop;    min.f32  S5, S1, 0x3f4ccccd;       nop;    nop
// CHECK: nop;    min.f32  S5, S1, 0x3f4ccccd, SP3;       nop;    nop
// CHECK: nop;    min.f32  S5, S1, 0x3f4ccccd, !SP3;      nop;    nop

NOP; MIN.I32 S5, S1, S2
NOP; MIN.I32 S5, S1, S2, SP3
NOP; MIN.I32 S5, S1, S2, !SP3

// CHECK: nop;    min.i32  S5, S1, S2;       nop;    nop
// CHECK: nop;    min.i32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    min.i32  S5, S1, S2, !SP3;      nop;    nop

NOP; MIN.I32 S5, S1, 123
NOP; MIN.I32 S5, S1, 123, SP3
NOP; MIN.I32 S5, S1, 123, !SP3

// CHECK: nop;    min.i32  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    min.i32  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    min.i32  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; MIN.U32 S5, S1, S2
NOP; MIN.U32 S5, S1, S2, SP3
NOP; MIN.U32 S5, S1, S2, !SP3

// CHECK: nop;    min.u32  S5, S1, S2;       nop;    nop
// CHECK: nop;    min.u32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    min.u32  S5, S1, S2, !SP3;      nop;    nop

NOP; MIN.U32 S5, S1, 123
NOP; MIN.U32 S5, S1, 123, SP3
NOP; MIN.U32 S5, S1, 123, !SP3

// CHECK: nop;    min.u32  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    min.u32  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    min.u32  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; MIN.I16 S5, S1, S2
NOP; MIN.I16 S5, S1, S2, SP3
NOP; MIN.I16 S5, S1, S2, !SP3

// CHECK: nop;    min.i16  S5, S1, S2;       nop;    nop
// CHECK: nop;    min.i16  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    min.i16  S5, S1, S2, !SP3;      nop;    nop

NOP; MIN.I16 S5, S1, 123
NOP; MIN.I16 S5, S1, 123, SP3
NOP; MIN.I16 S5, S1, 123, !SP3

// CHECK: nop;    min.i16  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    min.i16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    min.i16  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; MIN.U16 S5, S1, S2
NOP; MIN.U16 S5, S1, S2, SP3
NOP; MIN.U16 S5, S1, S2, !SP3

// CHECK: nop;    min.u16  S5, S1, S2;       nop;    nop
// CHECK: nop;    min.u16  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    min.u16  S5, S1, S2, !SP3;      nop;    nop

NOP; MIN.U16 S5, S1, 123
NOP; MIN.U16 S5, S1, 123, SP3
NOP; MIN.U16 S5, S1, 123, !SP3

// CHECK: nop;    min.u16  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    min.u16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    min.u16  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; MIN.I8 S5, S1, S2
NOP; MIN.I8 S5, S1, S2, SP3
NOP; MIN.I8 S5, S1, S2, !SP3

// CHECK: nop;    min.i8  S5, S1, S2;        nop;    nop
// CHECK: nop;    min.i8  S5, S1, S2, SP3;        nop;    nop
// CHECK: nop;    min.i8  S5, S1, S2, !SP3;       nop;    nop

NOP; MIN.I8 S5, S1, 123
NOP; MIN.I8 S5, S1, 123, SP3
NOP; MIN.I8 S5, S1, 123, !SP3

// CHECK: nop;    min.i8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    min.i8  S5, S1, 0x7b, SP3;      nop;    nop
// CHECK: nop;    min.i8  S5, S1, 0x7b, !SP3;     nop;    nop

NOP; MIN.U8 S5, S1, S2
NOP; MIN.U8 S5, S1, S2, SP3
NOP; MIN.U8 S5, S1, S2, !SP3

// CHECK: nop;    min.u8  S5, S1, S2;        nop;    nop
// CHECK: nop;    min.u8  S5, S1, S2, SP3;        nop;    nop
// CHECK: nop;    min.u8  S5, S1, S2, !SP3;       nop;    nop

NOP; MIN.U8 S5, S1, 123
NOP; MIN.U8 S5, S1, 123, SP3
NOP; MIN.U8 S5, S1, 123, !SP3

// CHECK: nop;    min.u8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    min.u8  S5, S1, 0x7b, SP3;      nop;    nop
// CHECK: nop;    min.u8  S5, S1, 0x7b, !SP3;     nop;    nop


NOP; MIN.I32 b10001 I5, I1, I2
NOP; MIN.I32 b10001 I5, I1, I2, SP3
NOP; MIN.I32 b10001 I5, I1, I2, !SP3

// CHECK: nop;    min.i32  b10001 I5, I1, I2;        nop;    nop
// CHECK: nop;    min.i32  b10001 I5, I1, I2, SP3;        nop;    nop
// CHECK: nop;    min.i32  b10001 I5, I1, I2, !SP3;       nop;    nop

NOP; MIN.I32 b10001 I5, S1, I2
NOP; MIN.I32 b10001 I5, S1, I2, SP3
NOP; MIN.I32 b10001 I5, S1, I2, !SP3

// CHECK: nop;    min.i32  b10001 I5, S1, I2;        nop;    nop
// CHECK: nop;    min.i32  b10001 I5, S1, I2, SP3;        nop;    nop
// CHECK: nop;    min.i32  b10001 I5, S1, I2, !SP3;       nop;    nop

NOP; MIN.I32 b10001 I5, 123, I2
NOP; MIN.I32 b10001 I5, 123, I2, SP3
NOP; MIN.I32 b10001 I5, 123, I2, !SP3

// CHECK: nop;    min.i32  b10001 I5, 0x7b, I2;      nop;    nop
// CHECK: nop;    min.i32  b10001 I5, 0x7b, I2, SP3;      nop;    nop
// CHECK: nop;    min.i32  b10001 I5, 0x7b, I2, !SP3;     nop;    nop
