// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu goya2 %s -o %t.gen4.o
// RUN: %disasm --mcpu goya2 %t.gen4.o | FileCheck %s

NOP; AND.F32 S5, S1, S2
NOP; AND.F32 S5, S1, S2, SP3
NOP; AND.F32 S5, S1, S2, !SP3

// CHECK: nop;    and.f32  S5, S1, S2;       nop;    nop
// CHECK: nop;    and.f32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    and.f32  S5, S1, S2, !SP3;      nop;    nop

NOP; AND.F32 S5, S1, 0x3f4ccccd
NOP; AND.F32 S5, S1, 0x3f4ccccd, SP3
NOP; AND.F32 S5, S1, 0x3f4ccccd, !SP3

// CHECK: nop;    and.f32  S5, S1, 0x3f4ccccd;       nop;    nop
// CHECK: nop;    and.f32  S5, S1, 0x3f4ccccd, SP3;       nop;    nop
// CHECK: nop;    and.f32  S5, S1, 0x3f4ccccd, !SP3;      nop;    nop

NOP; AND.I32 S5, S1, S2
NOP; AND.I32 S5, S1, S2, SP3
NOP; AND.I32 S5, S1, S2, !SP3

// CHECK: nop;    and.i32  S5, S1, S2;       nop;    nop
// CHECK: nop;    and.i32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    and.i32  S5, S1, S2, !SP3;      nop;    nop

NOP; AND.I32 S5, S1, 123
NOP; AND.I32 S5, S1, 123, SP3
NOP; AND.I32 S5, S1, 123, !SP3

// CHECK: nop;    and.i32  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    and.i32  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    and.i32  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; AND.U32 S5, S1, S2
NOP; AND.U32 S5, S1, S2, SP3
NOP; AND.U32 S5, S1, S2, !SP3

// CHECK: nop;    and.u32  S5, S1, S2;       nop;    nop
// CHECK: nop;    and.u32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    and.u32  S5, S1, S2, !SP3;      nop;    nop

NOP; AND.U32 S5, S1, 123
NOP; AND.U32 S5, S1, 123, SP3
NOP; AND.U32 S5, S1, 123, !SP3

// CHECK: nop;    and.u32  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    and.u32  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    and.u32  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; AND.I16 S5, S1, S2
NOP; AND.I16 S5, S1, S2, SP3
NOP; AND.I16 S5, S1, S2, !SP3

// CHECK: nop;    and.i16  S5, S1, S2;       nop;    nop
// CHECK: nop;    and.i16  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    and.i16  S5, S1, S2, !SP3;      nop;    nop

NOP; AND.I16 S5, S1, 123
NOP; AND.I16 S5, S1, 123, SP3
NOP; AND.I16 S5, S1, 123, !SP3

// CHECK: nop;    and.i16  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    and.i16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    and.i16  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; AND.U16 S5, S1, S2
NOP; AND.U16 S5, S1, S2, SP3
NOP; AND.U16 S5, S1, S2, !SP3

// CHECK: nop;    and.u16  S5, S1, S2;       nop;    nop
// CHECK: nop;    and.u16  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    and.u16  S5, S1, S2, !SP3;      nop;    nop

NOP; AND.U16 S5, S1, 123
NOP; AND.U16 S5, S1, 123, SP3
NOP; AND.U16 S5, S1, 123, !SP3

// CHECK: nop;    and.u16  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    and.u16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    and.u16  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; AND.I8 S5, S1, S2
NOP; AND.I8 S5, S1, S2, SP3
NOP; AND.I8 S5, S1, S2, !SP3

// CHECK: nop;    and.i8  S5, S1, S2;        nop;    nop
// CHECK: nop;    and.i8  S5, S1, S2, SP3;        nop;    nop
// CHECK: nop;    and.i8  S5, S1, S2, !SP3;       nop;    nop

NOP; AND.I8 S5, S1, 123
NOP; AND.I8 S5, S1, 123, SP3
NOP; AND.I8 S5, S1, 123, !SP3

// CHECK: nop;    and.i8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    and.i8  S5, S1, 0x7b, SP3;      nop;    nop
// CHECK: nop;    and.i8  S5, S1, 0x7b, !SP3;     nop;    nop

NOP; AND.U8 S5, S1, S2
NOP; AND.U8 S5, S1, S2, SP3
NOP; AND.U8 S5, S1, S2, !SP3

// CHECK: nop;    and.u8  S5, S1, S2;        nop;    nop
// CHECK: nop;    and.u8  S5, S1, S2, SP3;        nop;    nop
// CHECK: nop;    and.u8  S5, S1, S2, !SP3;       nop;    nop

NOP; AND.U8 S5, S1, 123
NOP; AND.U8 S5, S1, 123, SP3
NOP; AND.U8 S5, S1, 123, !SP3

// CHECK: nop;    and.u8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    and.u8  S5, S1, 0x7b, SP3;      nop;    nop
// CHECK: nop;    and.u8  S5, S1, 0x7b, !SP3;     nop;    nop

NOP; AND.B SP5, SP1, SP2
NOP; AND.B SP5, SP1, SP2, SP3
NOP; AND.B SP5, SP1, SP2, !SP3

// CHECK: nop;    and.b  SP5, SP1, SP2;      nop;    nop
// CHECK: nop;    and.b  SP5, SP1, SP2, SP3;      nop;    nop
// CHECK: nop;    and.b  SP5, SP1, SP2, !SP3;     nop;    nop


NOP; AND.I32 b10001 I5, I1, I2
NOP; AND.I32 b10001 I5, I1, I2, SP3
NOP; AND.I32 b10001 I5, I1, I2, !SP3

// CHECK: nop;    and.i32  b10001 I5, I1, I2;        nop;    nop
// CHECK: nop;    and.i32  b10001 I5, I1, I2, SP3;        nop;    nop
// CHECK: nop;    and.i32  b10001 I5, I1, I2, !SP3;       nop;    nop

NOP; AND.I32 b10001 I5, S1, I2
NOP; AND.I32 b10001 I5, S1, I2, SP3
NOP; AND.I32 b10001 I5, S1, I2, !SP3

// CHECK: nop;    and.i32  b10001 I5, S1, I2;        nop;    nop
// CHECK: nop;    and.i32  b10001 I5, S1, I2, SP3;        nop;    nop
// CHECK: nop;    and.i32  b10001 I5, S1, I2, !SP3;       nop;    nop

NOP; AND.I32 b10001 I5, 123, I2
NOP; AND.I32 b10001 I5, 123, I2, SP3
NOP; AND.I32 b10001 I5, 123, I2, !SP3

// CHECK: nop;    and.i32  b10001 I5, 0x7b, I2;      nop;    nop
// CHECK: nop;    and.i32  b10001 I5, 0x7b, I2, SP3;      nop;    nop
// CHECK: nop;    and.i32  b10001 I5, 0x7b, I2, !SP3;     nop;    nop
