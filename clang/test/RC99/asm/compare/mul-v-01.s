// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s


NOP; NOP; MUL.F32 V3, V2, V1
NOP; NOP; MUL.F32 V3, V2, V1, SP4
NOP; NOP; MUL.F32 V3, V2, V1, !SP4
NOP; NOP; MUL.F32 V3, V2, V1, VP4
NOP; NOP; MUL.F32 V3, V2, V1, !VP4

NOP; NOP; MUL.F32 V3, V2, S1
NOP; NOP; MUL.F32 V3, V2, S1, SP4
NOP; NOP; MUL.F32 V3, V2, S1, !SP4
NOP; NOP; MUL.F32 V3, V2, S1, VP4
NOP; NOP; MUL.F32 V3, V2, S1, !VP4

NOP; NOP; MUL.F32 V3, V2, 0x3f4ccccd
NOP; NOP; MUL.F32 V3, V2, 0x3f4ccccd, SP4
NOP; NOP; MUL.F32 V3, V2, 0x3f4ccccd, !SP4
NOP; NOP; MUL.F32 V3, V2, 0x3f4ccccd, VP4
NOP; NOP; MUL.F32 V3, V2, 0x3f4ccccd, !VP4

// CHECK: nop;    nop;    mul.f32  V3, V2, V1, SP0;       nop
// CHECK: nop;    nop;    mul.f32  V3, V2, V1, SP4;       nop
// CHECK: nop;    nop;    mul.f32  V3, V2, V1, !SP4;      nop
// CHECK: nop;    nop;    mul.f32  V3, V2, V1, VP4;       nop
// CHECK: nop;    nop;    mul.f32  V3, V2, V1, !VP4;      nop
// CHECK: nop;    nop;    mul.f32  V3, V2, S1, SP0;       nop
// CHECK: nop;    nop;    mul.f32  V3, V2, S1, SP4;       nop
// CHECK: nop;    nop;    mul.f32  V3, V2, S1, !SP4;      nop
// CHECK: nop;    nop;    mul.f32  V3, V2, S1, VP4;       nop
// CHECK: nop;    nop;    mul.f32  V3, V2, S1, !VP4;      nop
// CHECK: nop;    nop;    mul.f32  V3, V2, 0x3f4ccccd, SP0;       nop
// CHECK: nop;    nop;    mul.f32  V3, V2, 0x3f4ccccd, SP4;       nop
// CHECK: nop;    nop;    mul.f32  V3, V2, 0x3f4ccccd, !SP4;      nop
// CHECK: nop;    nop;    mul.f32  V3, V2, 0x3f4ccccd, VP4;       nop
// CHECK: nop;    nop;    mul.f32  V3, V2, 0x3f4ccccd, !VP4;      nop


NOP; NOP; MUL.I32 D6, V2, V1
NOP; NOP; MUL.I32 D6, V2, V1, SP4
NOP; NOP; MUL.I32 D6, V2, V1, !SP4
NOP; NOP; MUL.I32 D6, V2, V1, VP4
NOP; NOP; MUL.I32 D6, V2, V1, !VP4

NOP; NOP; MUL.I32 D6, V2, S1
NOP; NOP; MUL.I32 D6, V2, S1, SP4
NOP; NOP; MUL.I32 D6, V2, S1, !SP4
NOP; NOP; MUL.I32 D6, V2, S1, VP4
NOP; NOP; MUL.I32 D6, V2, S1, !VP4

NOP; NOP; MUL.I32 D6, V2, 123
NOP; NOP; MUL.I32 D6, V2, 123, SP4
NOP; NOP; MUL.I32 D6, V2, 123, !SP4
NOP; NOP; MUL.I32 D6, V2, 123, VP4
NOP; NOP; MUL.I32 D6, V2, 123, !VP4

// CHECK: nop;    nop;    mul.i32  D6, V2, V1, SP0;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, V1, SP4;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, V1, !SP4;      nop
// CHECK: nop;    nop;    mul.i32  D6, V2, V1, VP4;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, V1, !VP4;      nop
// CHECK: nop;    nop;    mul.i32  D6, V2, S1, SP0;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, S1, SP4;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, S1, !SP4;      nop
// CHECK: nop;    nop;    mul.i32  D6, V2, S1, VP4;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, S1, !VP4;      nop
// CHECK: nop;    nop;    mul.i32  D6, V2, 0x7b, SP0;     nop
// CHECK: nop;    nop;    mul.i32  D6, V2, 0x7b, SP4;     nop
// CHECK: nop;    nop;    mul.i32  D6, V2, 0x7b, !SP4;    nop
// CHECK: nop;    nop;    mul.i32  D6, V2, 0x7b, VP4;     nop
// CHECK: nop;    nop;    mul.i32  D6, V2, 0x7b, !VP4;    nop


NOP; NOP; MUL.I32 V6, V2, V1
NOP; NOP; MUL.I32 V6, V2, V1, SP4
NOP; NOP; MUL.I32 V6, V2, V1, !SP4
NOP; NOP; MUL.I32 V6, V2, V1, VP4
NOP; NOP; MUL.I32 V6, V2, V1, !VP4

NOP; NOP; MUL.I32 V6, V2, S1
NOP; NOP; MUL.I32 V6, V2, S1, SP4
NOP; NOP; MUL.I32 V6, V2, S1, !SP4
NOP; NOP; MUL.I32 V6, V2, S1, VP4
NOP; NOP; MUL.I32 V6, V2, S1, !VP4

NOP; NOP; MUL.I32 V6, V2, 123
NOP; NOP; MUL.I32 V6, V2, 123, SP4
NOP; NOP; MUL.I32 V6, V2, 123, !SP4
NOP; NOP; MUL.I32 V6, V2, 123, VP4
NOP; NOP; MUL.I32 V6, V2, 123, !VP4

// CHECK: nop;    nop;    mul.i32  D6, V2, V1, SP0;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, V1, SP4;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, V1, !SP4;      nop
// CHECK: nop;    nop;    mul.i32  D6, V2, V1, VP4;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, V1, !VP4;      nop
// CHECK: nop;    nop;    mul.i32  D6, V2, S1, SP0;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, S1, SP4;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, S1, !SP4;      nop
// CHECK: nop;    nop;    mul.i32  D6, V2, S1, VP4;       nop
// CHECK: nop;    nop;    mul.i32  D6, V2, S1, !VP4;      nop
// CHECK: nop;    nop;    mul.i32  D6, V2, 0x7b, SP0;     nop
// CHECK: nop;    nop;    mul.i32  D6, V2, 0x7b, SP4;     nop
// CHECK: nop;    nop;    mul.i32  D6, V2, 0x7b, !SP4;    nop
// CHECK: nop;    nop;    mul.i32  D6, V2, 0x7b, VP4;     nop
// CHECK: nop;    nop;    mul.i32  D6, V2, 0x7b, !VP4;    nop


NOP; NOP; MUL.I32.DOUBLE_AND_ROUND32 V3, V2, V1
NOP; NOP; MUL.I32.DOUBLE_AND_ROUND32 V3, V2, V1, SP6
NOP; NOP; MUL.I32.DOUBLE_AND_ROUND32 V3, V2, V1, !SP6
NOP; NOP; MUL.I32.DOUBLE_AND_ROUND32 V3, V2, V1, VP6
NOP; NOP; MUL.I32.DOUBLE_AND_ROUND32 V3, V2, V1, !VP6

NOP; NOP; MUL.I32 DOUBLE_AND_ROUND32 V3, V2, S1
NOP; NOP; MUL.I32 DOUBLE_AND_ROUND32 V3, V2, S1, SP6
NOP; NOP; MUL.I32 DOUBLE_AND_ROUND32 V3, V2, S1, !SP6
NOP; NOP; MUL.I32 DOUBLE_AND_ROUND32 V3, V2, S1, VP6
NOP; NOP; MUL.I32 DOUBLE_AND_ROUND32 V3, V2, S1, !VP6

NOP; NOP; MUL.I32.DOUBLE_AND_ROUND32 V3, V2, I32_123
NOP; NOP; MUL.I32.DOUBLE_AND_ROUND32 V3, V2, I32_123, SP6
NOP; NOP; MUL.I32.DOUBLE_AND_ROUND32 V3, V2, I32_123, !SP6
NOP; NOP; MUL.I32.DOUBLE_AND_ROUND32 V3, V2, I32_123, VP6
NOP; NOP; MUL.I32.DOUBLE_AND_ROUND32 V3, V2, I32_123, !VP6

NOP; NOP; MUL.I32 DOUBLE_AND_ROUND32 V3, V2, V1

// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, V1, SP0;       nop
// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, V1, SP6;       nop
// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, V1, !SP6;      nop
// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, V1, VP6;       nop
// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, V1, !VP6;      nop
// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, S1, SP0;       nop
// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, S1, SP6;       nop
// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, S1, !SP6;      nop
// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, S1, VP6;       nop
// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, S1, !VP6;      nop
// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, 0x7b, SP0;     nop
// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, 0x7b, SP6;     nop
// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, 0x7b, !SP6;    nop
// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, 0x7b, VP6;     nop
// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, 0x7b, !VP6;    nop

// CHECK: nop;    nop;    mul.i32  double_and_round32 V3, V2, V1, SP0;       nop


NOP; NOP; MUL.U32 D6, V2, V1
NOP; NOP; MUL.U32 D6, V2, V1, SP4
NOP; NOP; MUL.U32 D6, V2, V1, !SP4
NOP; NOP; MUL.U32 D6, V2, V1, VP4
NOP; NOP; MUL.U32 D6, V2, V1, !VP4

NOP; NOP; MUL.U32 D6, V2, S1
NOP; NOP; MUL.U32 D6, V2, S1, SP4
NOP; NOP; MUL.U32 D6, V2, S1, !SP4
NOP; NOP; MUL.U32 D6, V2, S1, VP4
NOP; NOP; MUL.U32 D6, V2, S1, !VP4

NOP; NOP; MUL.U32 D6, V2, 123
NOP; NOP; MUL.U32 D6, V2, 123, SP4
NOP; NOP; MUL.U32 D6, V2, 123, !SP4
NOP; NOP; MUL.U32 D6, V2, 123, VP4
NOP; NOP; MUL.U32 D6, V2, 123, !VP4

// CHECK: nop;    nop;    mul.u32  D6, V2, V1, SP0;       nop
// CHECK: nop;    nop;    mul.u32  D6, V2, V1, SP4;       nop
// CHECK: nop;    nop;    mul.u32  D6, V2, V1, !SP4;      nop
// CHECK: nop;    nop;    mul.u32  D6, V2, V1, VP4;       nop
// CHECK: nop;    nop;    mul.u32  D6, V2, V1, !VP4;      nop
// CHECK: nop;    nop;    mul.u32  D6, V2, S1, SP0;       nop
// CHECK: nop;    nop;    mul.u32  D6, V2, S1, SP4;       nop
// CHECK: nop;    nop;    mul.u32  D6, V2, S1, !SP4;      nop
// CHECK: nop;    nop;    mul.u32  D6, V2, S1, VP4;       nop
// CHECK: nop;    nop;    mul.u32  D6, V2, S1, !VP4;      nop
// CHECK: nop;    nop;    mul.u32  D6, V2, 0x7b, SP0;     nop
// CHECK: nop;    nop;    mul.u32  D6, V2, 0x7b, SP4;     nop
// CHECK: nop;    nop;    mul.u32  D6, V2, 0x7b, !SP4;    nop
// CHECK: nop;    nop;    mul.u32  D6, V2, 0x7b, VP4;     nop
// CHECK: nop;    nop;    mul.u32  D6, V2, 0x7b, !VP4;    nop


NOP; NOP; MUL.U32 V6, V2, V1
NOP; NOP; MUL.U32 V6, V2, V1, SP4
NOP; NOP; MUL.U32 V6, V2, V1, !SP4
NOP; NOP; MUL.U32 V6, V2, V1, VP4
NOP; NOP; MUL.U32 V6, V2, V1, !VP4

NOP; NOP; MUL.U32 V6, V2, S1
NOP; NOP; MUL.U32 V6, V2, S1, SP4
NOP; NOP; MUL.U32 V6, V2, S1, !SP4
NOP; NOP; MUL.U32 V6, V2, S1, VP4
NOP; NOP; MUL.U32 V6, V2, S1, !VP4

NOP; NOP; MUL.U32 V6, V2, 123
NOP; NOP; MUL.U32 V6, V2, 123, SP4
NOP; NOP; MUL.U32 V6, V2, 123, !SP4
NOP; NOP; MUL.U32 V6, V2, 123, VP4
NOP; NOP; MUL.U32 V6, V2, 123, !VP4

// CHECK: nop;    nop;    mul.u32  D6, V2, V1, SP0;       nop
// CHECK: nop;    nop;    mul.u32  D6, V2, V1, SP4;       nop
// CHECK: nop;    nop;    mul.u32  D6, V2, V1, !SP4;      nop
// CHECK: nop;    nop;    mul.u32  D6, V2, V1, VP4;       nop
// CHECK: nop;    nop;    mul.u32  D6, V2, V1, !VP4;      nop
// CHECK: nop;    nop;    mul.u32  D6, V2, S1, SP0;       nop
// CHECK: nop;    nop;    mul.u32  D6, V2, S1, SP4;       nop
// CHECK: nop;    nop;    mul.u32  D6, V2, S1, !SP4;      nop
// CHECK: nop;    nop;    mul.u32  D6, V2, S1, VP4;       nop
// CHECK: nop;    nop;    mul.u32  D6, V2, S1, !VP4;      nop
// CHECK: nop;    nop;    mul.u32  D6, V2, 0x7b, SP0;     nop
// CHECK: nop;    nop;    mul.u32  D6, V2, 0x7b, SP4;     nop
// CHECK: nop;    nop;    mul.u32  D6, V2, 0x7b, !SP4;    nop
// CHECK: nop;    nop;    mul.u32  D6, V2, 0x7b, VP4;     nop
// CHECK: nop;    nop;    mul.u32  D6, V2, 0x7b, !VP4;    nop

NOP; NOP; MUL.U32.DOUBLE_AND_ROUND32 V3, V2, V1
NOP; NOP; MUL.U32.DOUBLE_AND_ROUND32 V3, V2, V1, SP6
NOP; NOP; MUL.U32.DOUBLE_AND_ROUND32 V3, V2, V1, !SP6
NOP; NOP; MUL.U32.DOUBLE_AND_ROUND32 V3, V2, V1, VP6
NOP; NOP; MUL.U32.DOUBLE_AND_ROUND32 V3, V2, V1, !VP6

NOP; NOP; MUL.U32 DOUBLE_AND_ROUND32 V3, V2, S1
NOP; NOP; MUL.U32 DOUBLE_AND_ROUND32 V3, V2, S1, SP6
NOP; NOP; MUL.U32 DOUBLE_AND_ROUND32 V3, V2, S1, !SP6
NOP; NOP; MUL.U32 DOUBLE_AND_ROUND32 V3, V2, S1, VP6
NOP; NOP; MUL.U32 DOUBLE_AND_ROUND32 V3, V2, S1, !VP6

NOP; NOP; MUL.U32.DOUBLE_AND_ROUND32 V3, V2, U32_123
NOP; NOP; MUL.U32.DOUBLE_AND_ROUND32 V3, V2, U32_123, SP6
NOP; NOP; MUL.U32.DOUBLE_AND_ROUND32 V3, V2, U32_123, !SP6
NOP; NOP; MUL.U32.DOUBLE_AND_ROUND32 V3, V2, U32_123, VP6
NOP; NOP; MUL.U32.DOUBLE_AND_ROUND32 V3, V2, U32_123, !VP6

NOP; NOP; MUL.U32 DOUBLE_AND_ROUND32 V3, V2, V1

// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, V1, SP0;       nop
// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, V1, SP6;       nop
// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, V1, !SP6;      nop
// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, V1, VP6;       nop
// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, V1, !VP6;      nop
// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, S1, SP0;       nop
// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, S1, SP6;       nop
// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, S1, !SP6;      nop
// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, S1, VP6;       nop
// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, S1, !VP6;      nop
// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, 0x7b, SP0;     nop
// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, 0x7b, SP6;     nop
// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, 0x7b, !SP6;    nop
// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, 0x7b, VP6;     nop
// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, 0x7b, !VP6;    nop

// CHECK: nop;    nop;    mul.u32  double_and_round32 V3, V2, V1, SP0;       nop



NOP; NOP; MUL.I16 D6, V2, V1
NOP; NOP; MUL.I16 D6, V2, V1, SP4
NOP; NOP; MUL.I16 D6, V2, V1, !SP4
NOP; NOP; MUL.I16 D6, V2, V1, VP4
NOP; NOP; MUL.I16 D6, V2, V1, !VP4

NOP; NOP; MUL.I16 D6, V2, S1
NOP; NOP; MUL.I16 D6, V2, S1, SP4
NOP; NOP; MUL.I16 D6, V2, S1, !SP4
NOP; NOP; MUL.I16 D6, V2, S1, VP4
NOP; NOP; MUL.I16 D6, V2, S1, !VP4

NOP; NOP; MUL.I16 D6, V2, 123
NOP; NOP; MUL.I16 D6, V2, 123, SP4
NOP; NOP; MUL.I16 D6, V2, 123, !SP4
NOP; NOP; MUL.I16 D6, V2, 123, VP4
NOP; NOP; MUL.I16 D6, V2, 123, !VP4

// CHECK: nop;    nop;    mul.i16  D6, V2, V1, SP0;       nop
// CHECK: nop;    nop;    mul.i16  D6, V2, V1, SP4;       nop
// CHECK: nop;    nop;    mul.i16  D6, V2, V1, !SP4;      nop
// CHECK: nop;    nop;    mul.i16  D6, V2, V1, VP4;       nop
// CHECK: nop;    nop;    mul.i16  D6, V2, V1, !VP4;      nop
// CHECK: nop;    nop;    mul.i16  D6, V2, S1, SP0;       nop
// CHECK: nop;    nop;    mul.i16  D6, V2, S1, SP4;       nop
// CHECK: nop;    nop;    mul.i16  D6, V2, S1, !SP4;      nop
// CHECK: nop;    nop;    mul.i16  D6, V2, S1, VP4;       nop
// CHECK: nop;    nop;    mul.i16  D6, V2, S1, !VP4;      nop
// CHECK: nop;    nop;    mul.i16  D6, V2, 0x7b, SP0;     nop
// CHECK: nop;    nop;    mul.i16  D6, V2, 0x7b, SP4;     nop
// CHECK: nop;    nop;    mul.i16  D6, V2, 0x7b, !SP4;    nop
// CHECK: nop;    nop;    mul.i16  D6, V2, 0x7b, VP4;     nop
// CHECK: nop;    nop;    mul.i16  D6, V2, 0x7b, !VP4;    nop


NOP; NOP; MUL.I16 V6, V2, V1
NOP; NOP; MUL.I16 V6, V2, V1, SP4
NOP; NOP; MUL.I16 V6, V2, V1, !SP4
NOP; NOP; MUL.I16 V6, V2, V1, VP4
NOP; NOP; MUL.I16 V6, V2, V1, !VP4

NOP; NOP; MUL.I16 V6, V2, S1
NOP; NOP; MUL.I16 V6, V2, S1, SP4
NOP; NOP; MUL.I16 V6, V2, S1, !SP4
NOP; NOP; MUL.I16 V6, V2, S1, VP4
NOP; NOP; MUL.I16 V6, V2, S1, !VP4

NOP; NOP; MUL.I16 V6, V2, 123
NOP; NOP; MUL.I16 V6, V2, 123, SP4
NOP; NOP; MUL.I16 V6, V2, 123, !SP4
NOP; NOP; MUL.I16 V6, V2, 123, VP4
NOP; NOP; MUL.I16 V6, V2, 123, !VP4

// CHECK: nop;    nop;    mul.i16  D6, V2, V1, SP0;       nop
// CHECK: nop;    nop;    mul.i16  D6, V2, V1, SP4;       nop
// CHECK: nop;    nop;    mul.i16  D6, V2, V1, !SP4;      nop
// CHECK: nop;    nop;    mul.i16  D6, V2, V1, VP4;       nop
// CHECK: nop;    nop;    mul.i16  D6, V2, V1, !VP4;      nop
// CHECK: nop;    nop;    mul.i16  D6, V2, S1, SP0;       nop
// CHECK: nop;    nop;    mul.i16  D6, V2, S1, SP4;       nop
// CHECK: nop;    nop;    mul.i16  D6, V2, S1, !SP4;      nop
// CHECK: nop;    nop;    mul.i16  D6, V2, S1, VP4;       nop
// CHECK: nop;    nop;    mul.i16  D6, V2, S1, !VP4;      nop
// CHECK: nop;    nop;    mul.i16  D6, V2, 0x7b, SP0;     nop
// CHECK: nop;    nop;    mul.i16  D6, V2, 0x7b, SP4;     nop
// CHECK: nop;    nop;    mul.i16  D6, V2, 0x7b, !SP4;    nop
// CHECK: nop;    nop;    mul.i16  D6, V2, 0x7b, VP4;     nop
// CHECK: nop;    nop;    mul.i16  D6, V2, 0x7b, !VP4;    nop


NOP; NOP; MUL.U16 D6, V2, V1
NOP; NOP; MUL.U16 D6, V2, V1, SP4
NOP; NOP; MUL.U16 D6, V2, V1, !SP4
NOP; NOP; MUL.U16 D6, V2, V1, VP4
NOP; NOP; MUL.U16 D6, V2, V1, !VP4

NOP; NOP; MUL.U16 D6, V2, S1
NOP; NOP; MUL.U16 D6, V2, S1, SP4
NOP; NOP; MUL.U16 D6, V2, S1, !SP4
NOP; NOP; MUL.U16 D6, V2, S1, VP4
NOP; NOP; MUL.U16 D6, V2, S1, !VP4

NOP; NOP; MUL.U16 D6, V2, 123
NOP; NOP; MUL.U16 D6, V2, 123, SP4
NOP; NOP; MUL.U16 D6, V2, 123, !SP4
NOP; NOP; MUL.U16 D6, V2, 123, VP4
NOP; NOP; MUL.U16 D6, V2, 123, !VP4

// CHECK: nop;    nop;    mul.u16  D6, V2, V1, SP0;       nop
// CHECK: nop;    nop;    mul.u16  D6, V2, V1, SP4;       nop
// CHECK: nop;    nop;    mul.u16  D6, V2, V1, !SP4;      nop
// CHECK: nop;    nop;    mul.u16  D6, V2, V1, VP4;       nop
// CHECK: nop;    nop;    mul.u16  D6, V2, V1, !VP4;      nop
// CHECK: nop;    nop;    mul.u16  D6, V2, S1, SP0;       nop
// CHECK: nop;    nop;    mul.u16  D6, V2, S1, SP4;       nop
// CHECK: nop;    nop;    mul.u16  D6, V2, S1, !SP4;      nop
// CHECK: nop;    nop;    mul.u16  D6, V2, S1, VP4;       nop
// CHECK: nop;    nop;    mul.u16  D6, V2, S1, !VP4;      nop
// CHECK: nop;    nop;    mul.u16  D6, V2, 0x7b, SP0;     nop
// CHECK: nop;    nop;    mul.u16  D6, V2, 0x7b, SP4;     nop
// CHECK: nop;    nop;    mul.u16  D6, V2, 0x7b, !SP4;    nop
// CHECK: nop;    nop;    mul.u16  D6, V2, 0x7b, VP4;     nop
// CHECK: nop;    nop;    mul.u16  D6, V2, 0x7b, !VP4;    nop


NOP; NOP; MUL.U16 V6, V2, V1
NOP; NOP; MUL.U16 V6, V2, V1, SP4
NOP; NOP; MUL.U16 V6, V2, V1, !SP4
NOP; NOP; MUL.U16 V6, V2, V1, VP4
NOP; NOP; MUL.U16 V6, V2, V1, !VP4

NOP; NOP; MUL.U16 V6, V2, S1
NOP; NOP; MUL.U16 V6, V2, S1, SP4
NOP; NOP; MUL.U16 V6, V2, S1, !SP4
NOP; NOP; MUL.U16 V6, V2, S1, VP4
NOP; NOP; MUL.U16 V6, V2, S1, !VP4

NOP; NOP; MUL.U16 V6, V2, 123
NOP; NOP; MUL.U16 V6, V2, 123, SP4
NOP; NOP; MUL.U16 V6, V2, 123, !SP4
NOP; NOP; MUL.U16 V6, V2, 123, VP4
NOP; NOP; MUL.U16 V6, V2, 123, !VP4

// CHECK: nop;    nop;    mul.u16  D6, V2, V1, SP0;       nop
// CHECK: nop;    nop;    mul.u16  D6, V2, V1, SP4;       nop
// CHECK: nop;    nop;    mul.u16  D6, V2, V1, !SP4;      nop
// CHECK: nop;    nop;    mul.u16  D6, V2, V1, VP4;       nop
// CHECK: nop;    nop;    mul.u16  D6, V2, V1, !VP4;      nop
// CHECK: nop;    nop;    mul.u16  D6, V2, S1, SP0;       nop
// CHECK: nop;    nop;    mul.u16  D6, V2, S1, SP4;       nop
// CHECK: nop;    nop;    mul.u16  D6, V2, S1, !SP4;      nop
// CHECK: nop;    nop;    mul.u16  D6, V2, S1, VP4;       nop
// CHECK: nop;    nop;    mul.u16  D6, V2, S1, !VP4;      nop
// CHECK: nop;    nop;    mul.u16  D6, V2, 0x7b, SP0;     nop
// CHECK: nop;    nop;    mul.u16  D6, V2, 0x7b, SP4;     nop
// CHECK: nop;    nop;    mul.u16  D6, V2, 0x7b, !SP4;    nop
// CHECK: nop;    nop;    mul.u16  D6, V2, 0x7b, VP4;     nop
// CHECK: nop;    nop;    mul.u16  D6, V2, 0x7b, !VP4;    nop


NOP; NOP; MUL.I8 A8, V2, V1
NOP; NOP; MUL.I8 A8, V2, V1, SP4
NOP; NOP; MUL.I8 A8, V2, V1, !SP4
NOP; NOP; MUL.I8 A8, V2, V1, VP4
NOP; NOP; MUL.I8 A8, V2, V1, !VP4

NOP; NOP; MUL.I8 A8, V2, S1
NOP; NOP; MUL.I8 A8, V2, S1, SP4
NOP; NOP; MUL.I8 A8, V2, S1, !SP4
NOP; NOP; MUL.I8 A8, V2, S1, VP4
NOP; NOP; MUL.I8 A8, V2, S1, !VP4

NOP; NOP; MUL.I8 A8, V2, 123
NOP; NOP; MUL.I8 A8, V2, 123, SP4
NOP; NOP; MUL.I8 A8, V2, 123, !SP4
NOP; NOP; MUL.I8 A8, V2, 123, VP4
NOP; NOP; MUL.I8 A8, V2, 123, !VP4

// CHECK: nop;    nop;    mul.i8  A8, V2, V1, SP0;        nop
// CHECK: nop;    nop;    mul.i8  A8, V2, V1, SP4;        nop
// CHECK: nop;    nop;    mul.i8  A8, V2, V1, !SP4;       nop
// CHECK: nop;    nop;    mul.i8  A8, V2, V1, VP4;        nop
// CHECK: nop;    nop;    mul.i8  A8, V2, V1, !VP4;       nop
// CHECK: nop;    nop;    mul.i8  A8, V2, S1, SP0;        nop
// CHECK: nop;    nop;    mul.i8  A8, V2, S1, SP4;        nop
// CHECK: nop;    nop;    mul.i8  A8, V2, S1, !SP4;       nop
// CHECK: nop;    nop;    mul.i8  A8, V2, S1, VP4;        nop
// CHECK: nop;    nop;    mul.i8  A8, V2, S1, !VP4;       nop
// CHECK: nop;    nop;    mul.i8  A8, V2, 0x7b, SP0;      nop
// CHECK: nop;    nop;    mul.i8  A8, V2, 0x7b, SP4;      nop
// CHECK: nop;    nop;    mul.i8  A8, V2, 0x7b, !SP4;     nop
// CHECK: nop;    nop;    mul.i8  A8, V2, 0x7b, VP4;      nop
// CHECK: nop;    nop;    mul.i8  A8, V2, 0x7b, !VP4;     nop


NOP; NOP; MUL.I8 V8, V2, V1
NOP; NOP; MUL.I8 V8, V2, V1, SP4
NOP; NOP; MUL.I8 V8, V2, V1, !SP4
NOP; NOP; MUL.I8 V8, V2, V1, VP4
NOP; NOP; MUL.I8 V8, V2, V1, !VP4

NOP; NOP; MUL.I8 V8, V2, S1
NOP; NOP; MUL.I8 V8, V2, S1, SP4
NOP; NOP; MUL.I8 V8, V2, S1, !SP4
NOP; NOP; MUL.I8 V8, V2, S1, VP4
NOP; NOP; MUL.I8 V8, V2, S1, !VP4

NOP; NOP; MUL.I8 V8, V2, 123
NOP; NOP; MUL.I8 V8, V2, 123, SP4
NOP; NOP; MUL.I8 V8, V2, 123, !SP4
NOP; NOP; MUL.I8 V8, V2, 123, VP4
NOP; NOP; MUL.I8 V8, V2, 123, !VP4

// CHECK: nop;    nop;    mul.i8  A8, V2, V1, SP0;        nop
// CHECK: nop;    nop;    mul.i8  A8, V2, V1, SP4;        nop
// CHECK: nop;    nop;    mul.i8  A8, V2, V1, !SP4;       nop
// CHECK: nop;    nop;    mul.i8  A8, V2, V1, VP4;        nop
// CHECK: nop;    nop;    mul.i8  A8, V2, V1, !VP4;       nop
// CHECK: nop;    nop;    mul.i8  A8, V2, S1, SP0;        nop
// CHECK: nop;    nop;    mul.i8  A8, V2, S1, SP4;        nop
// CHECK: nop;    nop;    mul.i8  A8, V2, S1, !SP4;       nop
// CHECK: nop;    nop;    mul.i8  A8, V2, S1, VP4;        nop
// CHECK: nop;    nop;    mul.i8  A8, V2, S1, !VP4;       nop
// CHECK: nop;    nop;    mul.i8  A8, V2, 0x7b, SP0;      nop
// CHECK: nop;    nop;    mul.i8  A8, V2, 0x7b, SP4;      nop
// CHECK: nop;    nop;    mul.i8  A8, V2, 0x7b, !SP4;     nop
// CHECK: nop;    nop;    mul.i8  A8, V2, 0x7b, VP4;      nop
// CHECK: nop;    nop;    mul.i8  A8, V2, 0x7b, !VP4;     nop


NOP; NOP; MUL.U8 A8, V2, V1
NOP; NOP; MUL.U8 A8, V2, V1, SP4
NOP; NOP; MUL.U8 A8, V2, V1, !SP4
NOP; NOP; MUL.U8 A8, V2, V1, VP4
NOP; NOP; MUL.U8 A8, V2, V1, !VP4

NOP; NOP; MUL.U8 A8, V2, S1
NOP; NOP; MUL.U8 A8, V2, S1, SP4
NOP; NOP; MUL.U8 A8, V2, S1, !SP4
NOP; NOP; MUL.U8 A8, V2, S1, VP4
NOP; NOP; MUL.U8 A8, V2, S1, !VP4

NOP; NOP; MUL.U8 A8, V2, 123
NOP; NOP; MUL.U8 A8, V2, 123, SP4
NOP; NOP; MUL.U8 A8, V2, 123, !SP4
NOP; NOP; MUL.U8 A8, V2, 123, VP4
NOP; NOP; MUL.U8 A8, V2, 123, !VP4

// CHECK: nop;    nop;    mul.u8  A8, V2, V1, SP0;        nop
// CHECK: nop;    nop;    mul.u8  A8, V2, V1, SP4;        nop
// CHECK: nop;    nop;    mul.u8  A8, V2, V1, !SP4;       nop
// CHECK: nop;    nop;    mul.u8  A8, V2, V1, VP4;        nop
// CHECK: nop;    nop;    mul.u8  A8, V2, V1, !VP4;       nop
// CHECK: nop;    nop;    mul.u8  A8, V2, S1, SP0;        nop
// CHECK: nop;    nop;    mul.u8  A8, V2, S1, SP4;        nop
// CHECK: nop;    nop;    mul.u8  A8, V2, S1, !SP4;       nop
// CHECK: nop;    nop;    mul.u8  A8, V2, S1, VP4;        nop
// CHECK: nop;    nop;    mul.u8  A8, V2, S1, !VP4;       nop
// CHECK: nop;    nop;    mul.u8  A8, V2, 0x7b, SP0;      nop
// CHECK: nop;    nop;    mul.u8  A8, V2, 0x7b, SP4;      nop
// CHECK: nop;    nop;    mul.u8  A8, V2, 0x7b, !SP4;     nop
// CHECK: nop;    nop;    mul.u8  A8, V2, 0x7b, VP4;      nop
// CHECK: nop;    nop;    mul.u8  A8, V2, 0x7b, !VP4;     nop


NOP; NOP; MUL.U8 V8, V2, V1
NOP; NOP; MUL.U8 V8, V2, V1, SP4
NOP; NOP; MUL.U8 V8, V2, V1, !SP4
NOP; NOP; MUL.U8 V8, V2, V1, VP4
NOP; NOP; MUL.U8 V8, V2, V1, !VP4

NOP; NOP; MUL.U8 V8, V2, S1
NOP; NOP; MUL.U8 V8, V2, S1, SP4
NOP; NOP; MUL.U8 V8, V2, S1, !SP4
NOP; NOP; MUL.U8 V8, V2, S1, VP4
NOP; NOP; MUL.U8 V8, V2, S1, !VP4

NOP; NOP; MUL.U8 V8, V2, 123
NOP; NOP; MUL.U8 V8, V2, 123, SP4
NOP; NOP; MUL.U8 V8, V2, 123, !SP4
NOP; NOP; MUL.U8 V8, V2, 123, VP4
NOP; NOP; MUL.U8 V8, V2, 123, !VP4

// CHECK: nop;    nop;    mul.u8  A8, V2, V1, SP0;        nop
// CHECK: nop;    nop;    mul.u8  A8, V2, V1, SP4;        nop
// CHECK: nop;    nop;    mul.u8  A8, V2, V1, !SP4;       nop
// CHECK: nop;    nop;    mul.u8  A8, V2, V1, VP4;        nop
// CHECK: nop;    nop;    mul.u8  A8, V2, V1, !VP4;       nop
// CHECK: nop;    nop;    mul.u8  A8, V2, S1, SP0;        nop
// CHECK: nop;    nop;    mul.u8  A8, V2, S1, SP4;        nop
// CHECK: nop;    nop;    mul.u8  A8, V2, S1, !SP4;       nop
// CHECK: nop;    nop;    mul.u8  A8, V2, S1, VP4;        nop
// CHECK: nop;    nop;    mul.u8  A8, V2, S1, !VP4;       nop
// CHECK: nop;    nop;    mul.u8  A8, V2, 0x7b, SP0;      nop
// CHECK: nop;    nop;    mul.u8  A8, V2, 0x7b, SP4;      nop
// CHECK: nop;    nop;    mul.u8  A8, V2, 0x7b, !SP4;     nop
// CHECK: nop;    nop;    mul.u8  A8, V2, 0x7b, VP4;      nop
// CHECK: nop;    nop;    mul.u8  A8, V2, 0x7b, !VP4;     nop
