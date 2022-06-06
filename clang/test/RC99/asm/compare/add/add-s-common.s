// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s


NOP; ADD.F32 S5, S1, S2
NOP; ADD.F32 S5, S1, S2, SP3
NOP; ADD.F32 S5, S1, S2, !SP3

// CHECK: nop;    add.f32  S5, S1, S2;       nop;    nop
// CHECK: nop;    add.f32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    add.f32  S5, S1, S2, !SP3;       nop;    nop


NOP; ADD.F32 S5, S1, 0x3f4ccccd
NOP; ADD.F32 S5, S1, 0x3f4ccccd, SP3
NOP; ADD.F32 S5, S1, 0x3f4ccccd, !SP3

// CHECK: nop;    add.f32  S5, S1, 0x3f4ccccd;    nop;    nop
// CHECK: nop;    add.f32 S5, S1, 0x3f4ccccd, SP3;     nop;    nop
// CHECK: nop;    add.f32 S5, S1, 0x3f4ccccd, !SP3;    nop;    nop


NOP; ADD.I32 S5, S1, S2
NOP; ADD.U32 S5, S1, S2, SP3
NOP; ADD.I8  S5, S1, S2, !SP3

// CHECK: nop;    add.i32  S5, S1, S2;       nop;    nop
// CHECK: nop;    add.u32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    add.i8  S5, S1, S2, !SP3;       nop;    nop


NOP; ADD.U8  S5, S1, u8_123
NOP; ADD.I16 S5, S1, 123, SP3
NOP; ADD.U16 S5, S1, u16_222, !SP3

// CHECK: nop;    add.u8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    add.i16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    add.u16  S5, S1, 0xde, !SP3;    nop;    nop


NOP; ADD.I32.SAT S5, S1, S2
NOP; ADD.U32.SAT S5, S1, S2, SP3
NOP; ADD.I8.SAT  S5, S1, S2, !SP3

// CHECK: nop;    add.i32  st S5, S1, S2;    nop;    nop
// CHECK: nop;    add.u32  st S5, S1, S2, SP3;    nop;    nop
// CHECK: nop;    add.i8  st S5, S1, S2, !SP3;    nop;    nop


NOP; ADD.U8.SAT  S5, S1, u8_123
NOP; ADD.I16.SAT S5, S1, 123, SP3
NOP; ADD.U16.SAT S5, S1, u16_222, !SP3

// CHECK: nop;    add.u8  st S5, S1, 0x7b;   nop;    nop
// CHECK: nop;    add.i16  st S5, S1, 0x7b, SP3;  nop;    nop
// CHECK: nop;    add.u16  st S5, S1, 0xde, !SP3;         nop;    nop


NOP; ADD.I32 b11011 I5, I4, I2
NOP; ADD.I32 b11011 I5, I4, I2, SP1
NOP; ADD.I32 b11011 I5, I4, I2, !SP1

// CHECK: nop;    add.i32  b11011 I5, I4, I2;       nop;    nop
// CHECK: nop;    add.i32  b11011 I5, I4, I2, SP1;       nop;    nop
// CHECK: nop;    add.i32  b11011 I5, I4, I2, !SP1;      nop;    nop


NOP; ADD.I32 b11111 I5, i32_1234, I2
NOP; ADD.I32 b11111 I5, i32_1234, I2, SP1
NOP; ADD.I32 b11111 I5, i32_1234, I2, !SP1

// CHECK: nop;    add.i32  b11111 I5, 0x4d2, I2;    nop;    nop
// CHECK: nop;    add.i32  b11111 I5, 0x4d2, I2, SP1;    nop;    nop
// CHECK: nop;    add.i32  b11111 I5, 0x4d2, I2, !SP1;   nop;    nop


NOP; ADD.I32 b11011 I5, S5, I2
NOP; ADD.I32 b11011 I5, S5, I2, SP1
NOP; ADD.I32 b11011 I5, S5, I2, !SP1

// CHECK: nop;    add.i32  b11011 I5, S5, I2;       nop;    nop
// CHECK: nop;    add.i32  b11011 I5, S5, I2, SP1;       nop;    nop
// CHECK: nop;    add.i32  b11011 I5, S5, I2, !SP1;      nop;    nop
