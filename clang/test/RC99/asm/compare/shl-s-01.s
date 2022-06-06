// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu goya2 %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s


NOP; SHL.F32 S5, S1, S2; NOP; NOP
NOP; SHL.F32 S5, S1, S2, SP3; NOP; NOP
NOP; SHL.F32 S5, S1, S2, !SP3; NOP; NOP
NOP; SHL.F32 S5, S1, 0x3f4ccccd; NOP; NOP
NOP; SHL.F32 S5, S1, 0x3f4ccccd, SP3; NOP; NOP
NOP; SHL.F32 S5, S1, 0x3f4ccccd, !SP3; NOP; NOP

// CHECK: nop;    shl.f32  S5, S1, S2;       nop;    nop
// CHECK: nop;    shl.f32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    shl.f32  S5, S1, S2, !SP3;      nop;    nop
// CHECK: nop;    shl.f32  S5, S1, 0x3f4ccccd;       nop;    nop
// CHECK: nop;    shl.f32  S5, S1, 0x3f4ccccd, SP3;       nop;    nop
// CHECK: nop;    shl.f32  S5, S1, 0x3f4ccccd, !SP3;      nop;    nop


NOP; SHL.I32 S5, S1, S2; NOP; NOP
NOP; SHL.I32 S5, S1, S2, SP3; NOP; NOP
NOP; SHL.I32 S5, S1, S2, !SP3; NOP; NOP
NOP; SHL.I32 S5, S1, 123; NOP; NOP
NOP; SHL.I32 S5, S1, 123, SP3; NOP; NOP
NOP; SHL.I32 S5, S1, 123, !SP3; NOP; NOP

// CHECK: nop;    shl.i32  S5, S1, S2;       nop;    nop
// CHECK: nop;    shl.i32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    shl.i32  S5, S1, S2, !SP3;      nop;    nop
// CHECK: nop;    shl.i32  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    shl.i32  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    shl.i32  S5, S1, 0x7b, !SP3;    nop;    nop


NOP; SHL.U32 S5, S1, S2; NOP; NOP
NOP; SHL.U32 S5, S1, S2, SP3; NOP; NOP
NOP; SHL.U32 S5, S1, S2, !SP3; NOP; NOP
NOP; SHL.U32 S5, S1, 123; NOP; NOP
NOP; SHL.U32 S5, S1, 123, SP3; NOP; NOP
NOP; SHL.U32 S5, S1, 123, !SP3; NOP; NOP

// CHECK: nop;    shl.u32  S5, S1, S2;       nop;    nop
// CHECK: nop;    shl.u32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    shl.u32  S5, S1, S2, !SP3;      nop;    nop
// CHECK: nop;    shl.u32  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    shl.u32  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    shl.u32  S5, S1, 0x7b, !SP3;    nop;    nop


NOP; SHL.I16 S5, S1, S2; NOP; NOP
NOP; SHL.I16 S5, S1, S2, SP3; NOP; NOP
NOP; SHL.I16 S5, S1, S2, !SP3; NOP; NOP
NOP; SHL.I16 S5, S1, 123; NOP; NOP
NOP; SHL.I16 S5, S1, 123, SP3; NOP; NOP
NOP; SHL.I16 S5, S1, 123, !SP3; NOP; NOP

// CHECK: nop;    shl.i16  S5, S1, S2;       nop;    nop
// CHECK: nop;    shl.i16  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    shl.i16  S5, S1, S2, !SP3;      nop;    nop
// CHECK: nop;    shl.i16  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    shl.i16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    shl.i16  S5, S1, 0x7b, !SP3;    nop;    nop


NOP; SHL.U16 S5, S1, S2; NOP; NOP
NOP; SHL.U16 S5, S1, S2, SP3; NOP; NOP
NOP; SHL.U16 S5, S1, S2, !SP3; NOP; NOP
NOP; SHL.U16 S5, S1, 123; NOP; NOP
NOP; SHL.U16 S5, S1, 123, SP3; NOP; NOP
NOP; SHL.U16 S5, S1, 123, !SP3; NOP; NOP

// CHECK: nop;    shl.u16  S5, S1, S2;       nop;    nop
// CHECK: nop;    shl.u16  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    shl.u16  S5, S1, S2, !SP3;      nop;    nop
// CHECK: nop;    shl.u16  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    shl.u16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    shl.u16  S5, S1, 0x7b, !SP3;    nop;    nop


NOP; SHL.I8 S5, S1, S2; NOP; NOP
NOP; SHL.I8 S5, S1, S2, SP3; NOP; NOP
NOP; SHL.I8 S5, S1, S2, !SP3; NOP; NOP
NOP; SHL.I8 S5, S1, 123; NOP; NOP
NOP; SHL.I8 S5, S1, 123, SP3; NOP; NOP
NOP; SHL.I8 S5, S1, 123, !SP3; NOP; NOP

// CHECK: nop;    shl.i8  S5, S1, S2;        nop;    nop
// CHECK: nop;    shl.i8  S5, S1, S2, SP3;        nop;    nop
// CHECK: nop;    shl.i8  S5, S1, S2, !SP3;       nop;    nop
// CHECK: nop;    shl.i8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    shl.i8  S5, S1, 0x7b, SP3;      nop;    nop
// CHECK: nop;    shl.i8  S5, S1, 0x7b, !SP3;     nop;    nop


NOP; SHL.U8 S5, S1, S2; NOP; NOP
NOP; SHL.U8 S5, S1, S2, SP3; NOP; NOP
NOP; SHL.U8 S5, S1, S2, !SP3; NOP; NOP
NOP; SHL.U8 S5, S1, 123; NOP; NOP
NOP; SHL.U8 S5, S1, 123, SP3; NOP; NOP
NOP; SHL.U8 S5, S1, 123, !SP3; NOP; NOP

// CHECK: nop;    shl.u8  S5, S1, S2;        nop;    nop
// CHECK: nop;    shl.u8  S5, S1, S2, SP3;        nop;    nop
// CHECK: nop;    shl.u8  S5, S1, S2, !SP3;       nop;    nop
// CHECK: nop;    shl.u8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    shl.u8  S5, S1, 0x7b, SP3;      nop;    nop
// CHECK: nop;    shl.u8  S5, S1, 0x7b, !SP3;     nop;    nop
