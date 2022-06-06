// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu goya2 %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s

NOP; XOR.F32 S5, S1, S2
NOP; XOR.F32 S5, S1, S2, SP3
NOP; XOR.F32 S5, S1, S2, !SP3

// CHECK: nop;    xor.f32  S5, S1, S2;       nop;    nop
// CHECK: nop;    xor.f32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    xor.f32  S5, S1, S2, !SP3;      nop;    nop

NOP; XOR.F32 S5, S1, 0x3f4ccccd
NOP; XOR.F32 S5, S1, 0x3f4ccccd, SP3
NOP; XOR.F32 S5, S1, 0x3f4ccccd, !SP3

// CHECK: nop;    xor.f32  S5, S1, 0x3f4ccccd;       nop;    nop
// CHECK: nop;    xor.f32  S5, S1, 0x3f4ccccd, SP3;       nop;    nop
// CHECK: nop;    xor.f32  S5, S1, 0x3f4ccccd, !SP3;      nop;    nop

NOP; XOR.I32 S5, S1, S2
NOP; XOR.I32 S5, S1, S2, SP3
NOP; XOR.I32 S5, S1, S2, !SP3

// CHECK: nop;    xor.i32  S5, S1, S2;       nop;    nop
// CHECK: nop;    xor.i32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    xor.i32  S5, S1, S2, !SP3;      nop;    nop

NOP; XOR.I32 S5, S1, 123
NOP; XOR.I32 S5, S1, 123, SP3
NOP; XOR.I32 S5, S1, 123, !SP3

// CHECK: nop;    xor.i32  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    xor.i32  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    xor.i32  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; XOR.U32 S5, S1, S2
NOP; XOR.U32 S5, S1, S2, SP3
NOP; XOR.U32 S5, S1, S2, !SP3

// CHECK: nop;    xor.u32  S5, S1, S2;       nop;    nop
// CHECK: nop;    xor.u32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    xor.u32  S5, S1, S2, !SP3;      nop;    nop

NOP; XOR.U32 S5, S1, 123
NOP; XOR.U32 S5, S1, 123, SP3
NOP; XOR.U32 S5, S1, 123, !SP3

// CHECK: nop;    xor.u32  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    xor.u32  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    xor.u32  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; XOR.I16 S5, S1, S2
NOP; XOR.I16 S5, S1, S2, SP3
NOP; XOR.I16 S5, S1, S2, !SP3

// CHECK: nop;    xor.i16  S5, S1, S2;       nop;    nop
// CHECK: nop;    xor.i16  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    xor.i16  S5, S1, S2, !SP3;      nop;    nop

NOP; XOR.I16 S5, S1, 123
NOP; XOR.I16 S5, S1, 123, SP3
NOP; XOR.I16 S5, S1, 123, !SP3

// CHECK: nop;    xor.i16  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    xor.i16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    xor.i16  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; XOR.U16 S5, S1, S2
NOP; XOR.U16 S5, S1, S2, SP3
NOP; XOR.U16 S5, S1, S2, !SP3

// CHECK: nop;    xor.u16  S5, S1, S2;       nop;    nop
// CHECK: nop;    xor.u16  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    xor.u16  S5, S1, S2, !SP3;      nop;    nop

NOP; XOR.U16 S5, S1, 123
NOP; XOR.U16 S5, S1, 123, SP3
NOP; XOR.U16 S5, S1, 123, !SP3

// CHECK: nop;    xor.u16  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    xor.u16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    xor.u16  S5, S1, 0x7b, !SP3;    nop;    nop

NOP; XOR.I8 S5, S1, S2
NOP; XOR.I8 S5, S1, S2, SP3
NOP; XOR.I8 S5, S1, S2, !SP3

// CHECK: nop;    xor.i8  S5, S1, S2;        nop;    nop
// CHECK: nop;    xor.i8  S5, S1, S2, SP3;        nop;    nop
// CHECK: nop;    xor.i8  S5, S1, S2, !SP3;       nop;    nop

NOP; XOR.I8 S5, S1, 123
NOP; XOR.I8 S5, S1, 123, SP3
NOP; XOR.I8 S5, S1, 123, !SP3

// CHECK: nop;    xor.i8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    xor.i8  S5, S1, 0x7b, SP3;      nop;    nop
// CHECK: nop;    xor.i8  S5, S1, 0x7b, !SP3;     nop;    nop

NOP; XOR.U8 S5, S1, S2
NOP; XOR.U8 S5, S1, S2, SP3
NOP; XOR.U8 S5, S1, S2, !SP3

// CHECK: nop;    xor.u8  S5, S1, S2;        nop;    nop
// CHECK: nop;    xor.u8  S5, S1, S2, SP3;        nop;    nop
// CHECK: nop;    xor.u8  S5, S1, S2, !SP3;       nop;    nop

NOP; XOR.U8 S5, S1, 123
NOP; XOR.U8 S5, S1, 123, SP3
NOP; XOR.U8 S5, S1, 123, !SP3

// CHECK: nop;    xor.u8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    xor.u8  S5, S1, 0x7b, SP3;      nop;    nop
// CHECK: nop;    xor.u8  S5, S1, 0x7b, !SP3;     nop;    nop

NOP; XOR.B SP5, SP1, SP2
NOP; XOR.B SP5, SP1, SP2, SP3
NOP; XOR.B SP5, SP1, SP2, !SP3

// CHECK: nop;    xor.b  SP5, SP1, SP2;      nop;    nop
// CHECK: nop;    xor.b  SP5, SP1, SP2, SP3;      nop;    nop
// CHECK: nop;    xor.b  SP5, SP1, SP2, !SP3;     nop;    nop


NOP; XOR.I32 b10001 I5, I1, I2
NOP; XOR.I32 b10001 I5, I1, I2, SP3
NOP; XOR.I32 b10001 I5, I1, I2, !SP3

// CHECK: nop;    xor.i32  b10001 I5, I1, I2;        nop;    nop
// CHECK: nop;    xor.i32  b10001 I5, I1, I2, SP3;        nop;    nop
// CHECK: nop;    xor.i32  b10001 I5, I1, I2, !SP3;       nop;    nop

NOP; XOR.I32 b10001 I5, S1, I2
NOP; XOR.I32 b10001 I5, S1, I2, SP3
NOP; XOR.I32 b10001 I5, S1, I2, !SP3

// CHECK: nop;    xor.i32  b10001 I5, S1, I2;        nop;    nop
// CHECK: nop;    xor.i32  b10001 I5, S1, I2, SP3;        nop;    nop
// CHECK: nop;    xor.i32  b10001 I5, S1, I2, !SP3;       nop;    nop

NOP; XOR.I32 b10001 I5, 123, I2
NOP; XOR.I32 b10001 I5, 123, I2, SP3
NOP; XOR.I32 b10001 I5, 123, I2, !SP3

// CHECK: nop;    xor.i32  b10001 I5, 0x7b, I2;      nop;    nop
// CHECK: nop;    xor.i32  b10001 I5, 0x7b, I2, SP3;      nop;    nop
// CHECK: nop;    xor.i32  b10001 I5, 0x7b, I2, !SP3;     nop;    nop
