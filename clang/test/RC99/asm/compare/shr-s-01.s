// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu goya2 %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s


NOP; SHR.F32 S5, S1, S2; NOP; NOP
NOP; SHR.F32 S5, S1, S2, SP3; NOP; NOP
NOP; SHR.F32 S5, S1, S2, !SP3; NOP; NOP
NOP; SHR.F32 S5, S1, 0x3f4ccccd; NOP; NOP
NOP; SHR.F32 S5, S1, 0x3f4ccccd, SP3; NOP; NOP
NOP; SHR.F32 S5, S1, 0x3f4ccccd, !SP3; NOP; NOP

// CHECK: nop;    shr.f32  S5, S1, S2;       nop;    nop
// CHECK: nop;    shr.f32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    shr.f32  S5, S1, S2, !SP3;      nop;    nop
// CHECK: nop;    shr.f32  S5, S1, 0x3f4ccccd;       nop;    nop
// CHECK: nop;    shr.f32  S5, S1, 0x3f4ccccd, SP3;       nop;    nop
// CHECK: nop;    shr.f32  S5, S1, 0x3f4ccccd, !SP3;      nop;    nop


NOP; SHR.I32 S5, S1, S2; NOP; NOP
NOP; SHR.I32 S5, S1, S2, SP3; NOP; NOP
NOP; SHR.I32 S5, S1, S2, !SP3; NOP; NOP
NOP; SHR.I32 S5, S1, 123; NOP; NOP
NOP; SHR.I32 S5, S1, 123, SP3; NOP; NOP
NOP; SHR.I32 S5, S1, 123, !SP3; NOP; NOP

// CHECK: nop;    shr.i32  S5, S1, S2;       nop;    nop
// CHECK: nop;    shr.i32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    shr.i32  S5, S1, S2, !SP3;      nop;    nop
// CHECK: nop;    shr.i32  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    shr.i32  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    shr.i32  S5, S1, 0x7b, !SP3;    nop;    nop


NOP; SHR.U32 S5, S1, S2; NOP; NOP
NOP; SHR.U32 S5, S1, S2, SP3; NOP; NOP
NOP; SHR.U32 S5, S1, S2, !SP3; NOP; NOP
NOP; SHR.U32 S5, S1, 123; NOP; NOP
NOP; SHR.U32 S5, S1, 123, SP3; NOP; NOP
NOP; SHR.U32 S5, S1, 123, !SP3; NOP; NOP

// CHECK: nop;    shr.u32  S5, S1, S2;       nop;    nop
// CHECK: nop;    shr.u32  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    shr.u32  S5, S1, S2, !SP3;      nop;    nop
// CHECK: nop;    shr.u32  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    shr.u32  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    shr.u32  S5, S1, 0x7b, !SP3;    nop;    nop


NOP; SHR.I16 S5, S1, S2; NOP; NOP
NOP; SHR.I16 S5, S1, S2, SP3; NOP; NOP
NOP; SHR.I16 S5, S1, S2, !SP3; NOP; NOP
NOP; SHR.I16 S5, S1, 123; NOP; NOP
NOP; SHR.I16 S5, S1, 123, SP3; NOP; NOP
NOP; SHR.I16 S5, S1, 123, !SP3; NOP; NOP

// CHECK: nop;    shr.i16  S5, S1, S2;       nop;    nop
// CHECK: nop;    shr.i16  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    shr.i16  S5, S1, S2, !SP3;      nop;    nop
// CHECK: nop;    shr.i16  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    shr.i16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    shr.i16  S5, S1, 0x7b, !SP3;    nop;    nop


NOP; SHR.U16 S5, S1, S2; NOP; NOP
NOP; SHR.U16 S5, S1, S2, SP3; NOP; NOP
NOP; SHR.U16 S5, S1, S2, !SP3; NOP; NOP
NOP; SHR.U16 S5, S1, 123; NOP; NOP
NOP; SHR.U16 S5, S1, 123, SP3; NOP; NOP
NOP; SHR.U16 S5, S1, 123, !SP3; NOP; NOP

// CHECK: nop;    shr.u16  S5, S1, S2;       nop;    nop
// CHECK: nop;    shr.u16  S5, S1, S2, SP3;       nop;    nop
// CHECK: nop;    shr.u16  S5, S1, S2, !SP3;      nop;    nop
// CHECK: nop;    shr.u16  S5, S1, 0x7b;     nop;    nop
// CHECK: nop;    shr.u16  S5, S1, 0x7b, SP3;     nop;    nop
// CHECK: nop;    shr.u16  S5, S1, 0x7b, !SP3;    nop;    nop


NOP; SHR.I8 S5, S1, S2; NOP; NOP
NOP; SHR.I8 S5, S1, S2, SP3; NOP; NOP
NOP; SHR.I8 S5, S1, S2, !SP3; NOP; NOP
NOP; SHR.I8 S5, S1, 123; NOP; NOP
NOP; SHR.I8 S5, S1, 123, SP3; NOP; NOP
NOP; SHR.I8 S5, S1, 123, !SP3; NOP; NOP

// CHECK: nop;    shr.i8  S5, S1, S2;        nop;    nop
// CHECK: nop;    shr.i8  S5, S1, S2, SP3;        nop;    nop
// CHECK: nop;    shr.i8  S5, S1, S2, !SP3;       nop;    nop
// CHECK: nop;    shr.i8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    shr.i8  S5, S1, 0x7b, SP3;      nop;    nop
// CHECK: nop;    shr.i8  S5, S1, 0x7b, !SP3;     nop;    nop


NOP; SHR.U8 S5, S1, S2; NOP; NOP
NOP; SHR.U8 S5, S1, S2, SP3; NOP; NOP
NOP; SHR.U8 S5, S1, S2, !SP3; NOP; NOP
NOP; SHR.U8 S5, S1, 123; NOP; NOP
NOP; SHR.U8 S5, S1, 123, SP3; NOP; NOP
NOP; SHR.U8 S5, S1, 123, !SP3; NOP; NOP

// CHECK: nop;    shr.u8  S5, S1, S2;        nop;    nop
// CHECK: nop;    shr.u8  S5, S1, S2, SP3;        nop;    nop
// CHECK: nop;    shr.u8  S5, S1, S2, !SP3;       nop;    nop
// CHECK: nop;    shr.u8  S5, S1, 0x7b;      nop;    nop
// CHECK: nop;    shr.u8  S5, S1, 0x7b, SP3;      nop;    nop
// CHECK: nop;    shr.u8  S5, S1, 0x7b, !SP3;     nop;    nop
