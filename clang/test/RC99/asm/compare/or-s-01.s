// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu goya2 %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s

NOP; OR.F32 S5, S1, S2
NOP; OR.F32 S5, S1, S2, SP3
NOP; OR.F32 S5, S1, S2, !SP3

// CHECK: nop;    or.f32  S5, S1, S2;       nop;    nop
// CHECK: nop;    or.f32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    or.f32  S5, S1, S2, !SP3;      nop;    nop

NOP; OR.F32 S5, S1, 0x3f4ccccd
NOP; OR.F32 S5, S1, 0x3f4ccccd, SP3
NOP; OR.F32 S5, S1, 0x3f4ccccd, !SP3

// CHECK: nop;    or.f32  S5, S1, 0x3f4ccccd;       nop;    nop
// CHECK: nop;    or.f32  S5, S1, 0x3f4ccccd, SP3;       nop;    nop
// CHECK: nop;    or.f32  S5, S1, 0x3f4ccccd, !SP3;      nop;    nop

NOP; OR.I32 S5, S1, S2
NOP; OR.I32 S5, S1, S2, SP3
NOP; OR.I32 S5, S1, S2, !SP3

// CHECK: nop;    or.i32  S5, S1, S2;       nop;    nop
// CHECK: nop;    or.i32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    or.i32  S5, S1, S2, !SP3;      nop;    nop

NOP; OR.I32 S5, S1, 123
NOP; OR.I32 S5, S1, 123, SP3
NOP; OR.I32 S5, S1, 123, !SP3

// CHECK: nop;    or.i32  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    or.i32  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    or.i32  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; OR.U32 S5, S1, S2
NOP; OR.U32 S5, S1, S2, SP3
NOP; OR.U32 S5, S1, S2, !SP3

// CHECK: nop;    or.u32  S5, S1, S2;       nop;    nop
// CHECK: nop;    or.u32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    or.u32  S5, S1, S2, !SP3;      nop;    nop

NOP; OR.U32 S5, S1, 123
NOP; OR.U32 S5, S1, 123, SP3
NOP; OR.U32 S5, S1, 123, !SP3

// CHECK: nop;    or.u32  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    or.u32  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    or.u32  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; OR.I16 S5, S1, S2
NOP; OR.I16 S5, S1, S2, SP3
NOP; OR.I16 S5, S1, S2, !SP3

// CHECK: nop;    or.i16  S5, S1, S2;       nop;    nop
// CHECK: nop;    or.i16  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    or.i16  S5, S1, S2, !SP3;      nop;    nop

NOP; OR.I16 S5, S1, 123
NOP; OR.I16 S5, S1, 123, SP3
NOP; OR.I16 S5, S1, 123, !SP3

// CHECK: nop;    or.i16  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    or.i16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    or.i16  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; OR.U16 S5, S1, S2
NOP; OR.U16 S5, S1, S2, SP3
NOP; OR.U16 S5, S1, S2, !SP3

// CHECK: nop;    or.u16  S5, S1, S2;       nop;    nop
// CHECK: nop;    or.u16  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    or.u16  S5, S1, S2, !SP3;      nop;    nop

NOP; OR.U16 S5, S1, 123
NOP; OR.U16 S5, S1, 123, SP3
NOP; OR.U16 S5, S1, 123, !SP3

// CHECK: nop;    or.u16  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    or.u16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    or.u16  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; OR.I8 S5, S1, S2
NOP; OR.I8 S5, S1, S2, SP3
NOP; OR.I8 S5, S1, S2, !SP3

// CHECK: nop;    or.i8  S5, S1, S2;        nop;    nop
// CHECK: nop;    or.i8  S5, S1, S2, SP3;        nop;    nop
// CHECK: nop;    or.i8  S5, S1, S2, !SP3;       nop;    nop

NOP; OR.I8 S5, S1, 123
NOP; OR.I8 S5, S1, 123, SP3
NOP; OR.I8 S5, S1, 123, !SP3

// CHECK: nop;    or.i8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    or.i8  S5, S1, 0x7b, SP3;      nop;    nop
// CHECK: nop;    or.i8  S5, S1, 0x7b, !SP3;     nop;    nop

NOP; OR.U8 S5, S1, S2
NOP; OR.U8 S5, S1, S2, SP3
NOP; OR.U8 S5, S1, S2, !SP3

// CHECK: nop;    or.u8  S5, S1, S2;        nop;    nop
// CHECK: nop;    or.u8  S5, S1, S2, SP3;        nop;    nop
// CHECK: nop;    or.u8  S5, S1, S2, !SP3;       nop;    nop

NOP; OR.U8 S5, S1, 123
NOP; OR.U8 S5, S1, 123, SP3
NOP; OR.U8 S5, S1, 123, !SP3

// CHECK: nop;    or.u8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    or.u8  S5, S1, 0x7b, SP3;      nop;    nop
// CHECK: nop;    or.u8  S5, S1, 0x7b, !SP3;     nop;    nop

NOP; OR.B SP5, SP1, SP2
NOP; OR.B SP5, SP1, SP2, SP3
NOP; OR.B SP5, SP1, SP2, !SP3

// CHECK: nop;    or.b  SP5, SP1, SP2;      nop;    nop
// CHECK: nop;    or.b  SP5, SP1, SP2, SP3;      nop;    nop
// CHECK: nop;    or.b  SP5, SP1, SP2, !SP3;     nop;    nop


NOP; OR.I32 b10001 I5, I1, I2
NOP; OR.I32 b10001 I5, I1, I2, SP3
NOP; OR.I32 b10001 I5, I1, I2, !SP3

// CHECK: nop;    or.i32  b10001 I5, I1, I2;        nop;    nop
// CHECK: nop;    or.i32  b10001 I5, I1, I2, SP3;        nop;    nop
// CHECK: nop;    or.i32  b10001 I5, I1, I2, !SP3;       nop;    nop

NOP; OR.I32 b10001 I5, S1, I2
NOP; OR.I32 b10001 I5, S1, I2, SP3
NOP; OR.I32 b10001 I5, S1, I2, !SP3

// CHECK: nop;    or.i32  b10001 I5, S1, I2;        nop;    nop
// CHECK: nop;    or.i32  b10001 I5, S1, I2, SP3;        nop;    nop
// CHECK: nop;    or.i32  b10001 I5, S1, I2, !SP3;       nop;    nop

NOP; OR.I32 b10001 I5, 123, I2
NOP; OR.I32 b10001 I5, 123, I2, SP3
NOP; OR.I32 b10001 I5, 123, I2, !SP3

// CHECK: nop;    or.i32  b10001 I5, 0x7b, I2;      nop;    nop
// CHECK: nop;    or.i32  b10001 I5, 0x7b, I2, SP3;      nop;    nop
// CHECK: nop;    or.i32  b10001 I5, 0x7b, I2, !SP3;     nop;    nop
