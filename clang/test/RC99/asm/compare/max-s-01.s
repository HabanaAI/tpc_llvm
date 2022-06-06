// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu goya2 %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s

NOP; MAX.F32 S5, S1, S2
NOP; MAX.F32 S5, S1, S2, SP3
NOP; MAX.F32 S5, S1, S2, !SP3

// CHECK: nop;    max.f32  S5, S1, S2;       nop;    nop
// CHECK: nop;    max.f32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    max.f32  S5, S1, S2, !SP3;      nop;    nop

NOP; MAX.F32 S5, S1, 0x3f4ccccd
NOP; MAX.F32 S5, S1, 0x3f4ccccd, SP3
NOP; MAX.F32 S5, S1, 0x3f4ccccd, !SP3

// CHECK: nop;    max.f32  S5, S1, 0x3f4ccccd;       nop;    nop
// CHECK: nop;    max.f32  S5, S1, 0x3f4ccccd, SP3;       nop;    nop
// CHECK: nop;    max.f32  S5, S1, 0x3f4ccccd, !SP3;      nop;    nop

NOP; MAX.I32 S5, S1, S2
NOP; MAX.I32 S5, S1, S2, SP3
NOP; MAX.I32 S5, S1, S2, !SP3

// CHECK: nop;    max.i32  S5, S1, S2;       nop;    nop
// CHECK: nop;    max.i32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    max.i32  S5, S1, S2, !SP3;      nop;    nop

NOP; MAX.I32 S5, S1, 123
NOP; MAX.I32 S5, S1, 123, SP3
NOP; MAX.I32 S5, S1, 123, !SP3

// CHECK: nop;    max.i32  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    max.i32  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    max.i32  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; MAX.U32 S5, S1, S2
NOP; MAX.U32 S5, S1, S2, SP3
NOP; MAX.U32 S5, S1, S2, !SP3

// CHECK: nop;    max.u32  S5, S1, S2;       nop;    nop
// CHECK: nop;    max.u32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    max.u32  S5, S1, S2, !SP3;      nop;    nop

NOP; MAX.U32 S5, S1, 123
NOP; MAX.U32 S5, S1, 123, SP3
NOP; MAX.U32 S5, S1, 123, !SP3

// CHECK: nop;    max.u32  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    max.u32  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    max.u32  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; MAX.I16 S5, S1, S2
NOP; MAX.I16 S5, S1, S2, SP3
NOP; MAX.I16 S5, S1, S2, !SP3

// CHECK: nop;    max.i16  S5, S1, S2;       nop;    nop
// CHECK: nop;    max.i16  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    max.i16  S5, S1, S2, !SP3;      nop;    nop

NOP; MAX.I16 S5, S1, 123
NOP; MAX.I16 S5, S1, 123, SP3
NOP; MAX.I16 S5, S1, 123, !SP3

// CHECK: nop;    max.i16  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    max.i16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    max.i16  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; MAX.U16 S5, S1, S2
NOP; MAX.U16 S5, S1, S2, SP3
NOP; MAX.U16 S5, S1, S2, !SP3

// CHECK: nop;    max.u16  S5, S1, S2;       nop;    nop
// CHECK: nop;    max.u16  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    max.u16  S5, S1, S2, !SP3;      nop;    nop

NOP; MAX.U16 S5, S1, 123
NOP; MAX.U16 S5, S1, 123, SP3
NOP; MAX.U16 S5, S1, 123, !SP3

// CHECK: nop;    max.u16  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    max.u16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    max.u16  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; MAX.I8 S5, S1, S2
NOP; MAX.I8 S5, S1, S2, SP3
NOP; MAX.I8 S5, S1, S2, !SP3

// CHECK: nop;    max.i8  S5, S1, S2;        nop;    nop
// CHECK: nop;    max.i8  S5, S1, S2, SP3;        nop;    nop
// CHECK: nop;    max.i8  S5, S1, S2, !SP3;       nop;    nop

NOP; MAX.I8 S5, S1, 123
NOP; MAX.I8 S5, S1, 123, SP3
NOP; MAX.I8 S5, S1, 123, !SP3

// CHECK: nop;    max.i8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    max.i8  S5, S1, 0x7b, SP3;      nop;    nop
// CHECK: nop;    max.i8  S5, S1, 0x7b, !SP3;     nop;    nop

NOP; MAX.U8 S5, S1, S2
NOP; MAX.U8 S5, S1, S2, SP3
NOP; MAX.U8 S5, S1, S2, !SP3

// CHECK: nop;    max.u8  S5, S1, S2;        nop;    nop
// CHECK: nop;    max.u8  S5, S1, S2, SP3;        nop;    nop
// CHECK: nop;    max.u8  S5, S1, S2, !SP3;       nop;    nop

NOP; MAX.U8 S5, S1, 123
NOP; MAX.U8 S5, S1, 123, SP3
NOP; MAX.U8 S5, S1, 123, !SP3

// CHECK: nop;    max.u8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    max.u8  S5, S1, 0x7b, SP3;      nop;    nop
// CHECK: nop;    max.u8  S5, S1, 0x7b, !SP3;     nop;    nop


NOP; MAX.I32 b10001 I5, I1, I2
NOP; MAX.I32 b10001 I5, I1, I2, SP3
NOP; MAX.I32 b10001 I5, I1, I2, !SP3

// CHECK: nop;    max.i32  b10001 I5, I1, I2;        nop;    nop
// CHECK: nop;    max.i32  b10001 I5, I1, I2, SP3;        nop;    nop
// CHECK: nop;    max.i32  b10001 I5, I1, I2, !SP3;       nop;    nop

NOP; MAX.I32 b10001 I5, S1, I2
NOP; MAX.I32 b10001 I5, S1, I2, SP3
NOP; MAX.I32 b10001 I5, S1, I2, !SP3

// CHECK: nop;    max.i32  b10001 I5, S1, I2;        nop;    nop
// CHECK: nop;    max.i32  b10001 I5, S1, I2, SP3;        nop;    nop
// CHECK: nop;    max.i32  b10001 I5, S1, I2, !SP3;       nop;    nop

NOP; MAX.I32 b10001 I5, 123, I2
NOP; MAX.I32 b10001 I5, 123, I2, SP3
NOP; MAX.I32 b10001 I5, 123, I2, !SP3

// CHECK: nop;    max.i32  b10001 I5, 0x7b, I2;      nop;    nop
// CHECK: nop;    max.i32  b10001 I5, 0x7b, I2, SP3;      nop;    nop
// CHECK: nop;    max.i32  b10001 I5, 0x7b, I2, !SP3;     nop;    nop
