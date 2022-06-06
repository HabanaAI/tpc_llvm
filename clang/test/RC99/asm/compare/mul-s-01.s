// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s


NOP; MUL.I32 S5, S1, S2
NOP; MUL.U32 S5, S1, S2, SP3
NOP; MUL.I8  S5, S1, S2, !SP3

// CHECK: nop;    mul.i32  S5, S1, S2;       nop;    nop
// CHECK: nop;    mul.u32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    mul.i8  S5, S1, S2, !SP3;       nop;    nop


NOP; MUL.U8  S5, S1, u8_123
NOP; MUL.I16 S5, S1, 123, SP3
NOP; MUL.U16 S5, S1, u16_222, !SP3

// CHECK: nop;    mul.u8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    mul.i16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    mul.u16  S5, S1, 0xde, !SP3;    nop;    nop


NOP; MUL.I32.UPPER32 S5, S1, S2
NOP; MUL.U32.UPPER32 S5, S1, S2, SP3

// CHECK: nop;    mul.i32  upper32 S5, S1, S2;       nop;    nop
// CHECK: nop;    mul.u32  upper32 S5, S1, S2, SP3;       nop;    nop


NOP; MUL.I32 b11011 I5, I4, I2
NOP; MUL.I32 b11011 I5, I4, I2, SP1
NOP; MUL.I32 b11011 I5, I4, I2, !SP1

// CHECK: nop;    mul.i32  b11011 I5, I4, I2;        nop;    nop
// CHECK: nop;    mul.i32  b11011 I5, I4, I2, SP1;        nop;    nop
// CHECK: nop;    mul.i32  b11011 I5, I4, I2, !SP1;       nop;    nop


NOP; MUL.I32 b11111 I5, i32_1234, I2
NOP; MUL.I32 b11111 I5, i32_1234, I2, SP1
NOP; MUL.I32 b11111 I5, i32_1234, I2, !SP1

// CHECK: nop;    mul.i32  b11111 I5, 0x4d2, I2;     nop;    nop
// CHECK: nop;    mul.i32  b11111 I5, 0x4d2, I2, SP1;     nop;    nop
// CHECK: nop;    mul.i32  b11111 I5, 0x4d2, I2, !SP1;    nop;    nop


NOP; MUL.I32 b11011 I5, S5, I2
NOP; MUL.I32 b11011 I5, S5, I2, SP1
NOP; MUL.I32 b11011 I5, S5, I2, !SP1

// CHECK: nop;    mul.i32  b11011 I5, S5, I2;        nop;    nop
// CHECK: nop;    mul.i32  b11011 I5, S5, I2, SP1;        nop;    nop
// CHECK: nop;    mul.i32  b11011 I5, S5, I2, !SP1;       nop;    nop

NOP; MUL.I32 b00000 I5, S5, I2, !SP1

// fixCHECK: nop;    mul.i32  b00000 I5, S5, I2, SP0;        nop;    nop
