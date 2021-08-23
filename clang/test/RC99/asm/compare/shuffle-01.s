// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; NOP; SHUFFLE.F32 V1, V2, V3
NOP; NOP; SHUFFLE.F32 V1, V2, V3, SP4
NOP; NOP; SHUFFLE.F32 V1, V2, V3, !SP4
NOP; NOP; SHUFFLE.F32 V1, V2, V3, VP4
NOP; NOP; SHUFFLE.F32 V1, V2, V3, !VP4

// CHECK: nop;    nop;    shuffle.f32  V1, V2, V3, SP0;   nop
// CHECK: nop;    nop;    shuffle.f32  V1, V2, V3, SP4;   nop
// CHECK: nop;    nop;    shuffle.f32  V1, V2, V3, !SP4;  nop
// CHECK: nop;    nop;    shuffle.f32  V1, V2, V3, VP4;   nop
// CHECK: nop;    nop;    shuffle.f32  V1, V2, V3, !VP4;  nop


NOP; NOP; SHUFFLE.I32 V1, V2, V3
NOP; NOP; SHUFFLE.I32 V1, V2, V3, SP4
NOP; NOP; SHUFFLE.I32 V1, V2, V3, !SP4
NOP; NOP; SHUFFLE.I32 V1, V2, V3, VP4
NOP; NOP; SHUFFLE.I32 V1, V2, V3, !VP4

// CHECK: nop;    nop;    shuffle.i32  V1, V2, V3, SP0;   nop
// CHECK: nop;    nop;    shuffle.i32  V1, V2, V3, SP4;   nop
// CHECK: nop;    nop;    shuffle.i32  V1, V2, V3, !SP4;  nop
// CHECK: nop;    nop;    shuffle.i32  V1, V2, V3, VP4;   nop
// CHECK: nop;    nop;    shuffle.i32  V1, V2, V3, !VP4;  nop


NOP; NOP; SHUFFLE.U32 V1, V2, V3
NOP; NOP; SHUFFLE.U32 V1, V2, V3, SP4
NOP; NOP; SHUFFLE.U32 V1, V2, V3, !SP4
NOP; NOP; SHUFFLE.U32 V1, V2, V3, VP4
NOP; NOP; SHUFFLE.U32 V1, V2, V3, !VP4

// CHECK: nop;    nop;    shuffle.u32  V1, V2, V3, SP0;   nop
// CHECK: nop;    nop;    shuffle.u32  V1, V2, V3, SP4;   nop
// CHECK: nop;    nop;    shuffle.u32  V1, V2, V3, !SP4;  nop
// CHECK: nop;    nop;    shuffle.u32  V1, V2, V3, VP4;   nop
// CHECK: nop;    nop;    shuffle.u32  V1, V2, V3, !VP4;  nop


NOP; NOP; SHUFFLE.I16 V1, V2, V3
NOP; NOP; SHUFFLE.I16 V1, V2, V3, SP4
NOP; NOP; SHUFFLE.I16 V1, V2, V3, !SP4
NOP; NOP; SHUFFLE.I16 V1, V2, V3, VP4
NOP; NOP; SHUFFLE.I16 V1, V2, V3, !VP4

// CHECK: nop;    nop;    shuffle.i16  V1, V2, V3, SP0;   nop
// CHECK: nop;    nop;    shuffle.i16  V1, V2, V3, SP4;   nop
// CHECK: nop;    nop;    shuffle.i16  V1, V2, V3, !SP4;  nop
// CHECK: nop;    nop;    shuffle.i16  V1, V2, V3, VP4;   nop
// CHECK: nop;    nop;    shuffle.i16  V1, V2, V3, !VP4;  nop


NOP; NOP; SHUFFLE.U16 V1, V2, V3
NOP; NOP; SHUFFLE.U16 V1, V2, V3, SP4
NOP; NOP; SHUFFLE.U16 V1, V2, V3, !SP4
NOP; NOP; SHUFFLE.U16 V1, V2, V3, VP4
NOP; NOP; SHUFFLE.U16 V1, V2, V3, !VP4

// CHECK: nop;    nop;    shuffle.u16  V1, V2, V3, SP0;   nop
// CHECK: nop;    nop;    shuffle.u16  V1, V2, V3, SP4;   nop
// CHECK: nop;    nop;    shuffle.u16  V1, V2, V3, !SP4;  nop
// CHECK: nop;    nop;    shuffle.u16  V1, V2, V3, VP4;   nop
// CHECK: nop;    nop;    shuffle.u16  V1, V2, V3, !VP4;  nop


NOP; NOP; SHUFFLE.I8 V1, V2, V3
NOP; NOP; SHUFFLE.I8 V1, V2, V3, SP4
NOP; NOP; SHUFFLE.I8 V1, V2, V3, !SP4
NOP; NOP; SHUFFLE.I8 V1, V2, V3, VP4
NOP; NOP; SHUFFLE.I8 V1, V2, V3, !VP4

// CHECK: nop;    nop;    shuffle.i8  V1, V2, V3, SP0;    nop
// CHECK: nop;    nop;    shuffle.i8  V1, V2, V3, SP4;    nop
// CHECK: nop;    nop;    shuffle.i8  V1, V2, V3, !SP4;   nop
// CHECK: nop;    nop;    shuffle.i8  V1, V2, V3, VP4;    nop
// CHECK: nop;    nop;    shuffle.i8  V1, V2, V3, !VP4;   nop


NOP; NOP; SHUFFLE.U8 V1, V2, V3
NOP; NOP; SHUFFLE.U8 V1, V2, V3, SP4
NOP; NOP; SHUFFLE.U8 V1, V2, V3, !SP4
NOP; NOP; SHUFFLE.U8 V1, V2, V3, VP4
NOP; NOP; SHUFFLE.U8 V1, V2, V3, !VP4

// CHECK: nop;    nop;    shuffle.u8  V1, V2, V3, SP0;    nop
// CHECK: nop;    nop;    shuffle.u8  V1, V2, V3, SP4;    nop
// CHECK: nop;    nop;    shuffle.u8  V1, V2, V3, !SP4;   nop
// CHECK: nop;    nop;    shuffle.u8  V1, V2, V3, VP4;    nop
// CHECK: nop;    nop;    shuffle.u8  V1, V2, V3, !VP4;   nop
