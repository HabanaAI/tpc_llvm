// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s


NOP; NOP; ADD.I32 V3, V2, V1
NOP; NOP; ADD.U32 V3, V2, V1, SP1
NOP; NOP; ADD.I16 V3, V2, V1, !SP1
NOP; NOP; ADD.U16 V3, V2, V1, VP1
NOP; NOP; ADD.I8  V3, V2, V1, !VP1

// CHECK: nop;    nop;    add.i32  V3, V2, V1;       nop
// CHECK: nop;    nop;    add.u32  V3, V2, V1, SP1;       nop
// CHECK: nop;    nop;    add.i16  V3, V2, V1, !SP1;      nop
// CHECK: nop;    nop;    add.u16  V3, V2, V1, VP1;       nop
// CHECK: nop;    nop;    add.i8  V3, V2, V1, !VP1;       nop


NOP; NOP; ADD.U8  V3, V2, u8_234
NOP; NOP; ADD.F32 V3, V2, f32_2.25
NOP; NOP; ADD.I32 V3, V2, i32_-144, VP1
NOP; NOP; ADD.I8  V3, V2, i8_-4, !VP1

// CHECK: nop;    nop;    add.u8  V3, V2, 0xea;      nop
// CHECK: nop;    nop;    add.f32  V3, V2, 0x40100000;       nop
// CHECK: nop;    nop;    add.i32  V3, V2, 0xffffff70, VP1;       nop
// CHECK: nop;    nop;    add.i8  V3, V2, 0xfffffffc, !VP1;       nop


NOP; NOP; ADD.I32 V3, V2, S1
NOP; NOP; ADD.I32 V3, V2, S1, SP6
NOP; NOP; ADD.I32 V3, V2, S1, !SP6
NOP; NOP; ADD.I32 V3, V2, S1, VP6
NOP; NOP; ADD.I32 V3, V2, S1, !VP6

// CHECK: nop;    nop;    add.i32  V3, V2, S1;       nop
// CHECK: nop;    nop;    add.i32  V3, V2, S1, SP6;       nop
// CHECK: nop;    nop;    add.i32  V3, V2, S1, !SP6;      nop
// CHECK: nop;    nop;    add.i32  V3, V2, S1, VP6;       nop
// CHECK: nop;    nop;    add.i32  V3, V2, S1, !VP6;      nop


NOP; NOP; ADD.I32.SAT V3, V2, V1
NOP; NOP; ADD.I32.SAT V3, V2, V1, SP6
NOP; NOP; ADD.I32.SAT V3, V2, V1, !SP6
NOP; NOP; ADD.I32.SAT V3, V2, V1, VP6
NOP; NOP; ADD.I32.SAT V3, V2, V1, !VP6

// CHECK: nop;    nop;    add.i32  st V3, V2, V1;    nop
// CHECK: nop;    nop;    add.i32  st V3, V2, V1, SP6;    nop
// CHECK: nop;    nop;    add.i32  st V3, V2, V1, !SP6;   nop
// CHECK: nop;    nop;    add.i32  st V3, V2, V1, VP6;    nop
// CHECK: nop;    nop;    add.i32  st V3, V2, V1, !VP6;   nop


NOP; NOP; ADD.I32.SAT V3, V2, I32_1234
NOP; NOP; ADD.I32.SAT V3, V2, I32_1234, SP6
NOP; NOP; ADD.I32.SAT V3, V2, I32_1234, !SP6
NOP; NOP; ADD.I32.SAT V3, V2, I32_1234, VP6
NOP; NOP; ADD.I32.SAT V3, V2, I32_1234, !VP6

// CHECK: nop;    nop;    add.i32  st V3, V2, 0x4d2;         nop
// CHECK: nop;    nop;    add.i32  st V3, V2, 0x4d2, SP6;         nop
// CHECK: nop;    nop;    add.i32  st V3, V2, 0x4d2, !SP6;        nop
// CHECK: nop;    nop;    add.i32  st V3, V2, 0x4d2, VP6;         nop
// CHECK: nop;    nop;    add.i32  st V3, V2, 0x4d2, !VP6;        nop


NOP; NOP; ADD.I32.SAT V3, V2, S1
NOP; NOP; ADD.I32.SAT V3, V2, S1, SP6
NOP; NOP; ADD.I32.SAT V3, V2, S1, !SP6
NOP; NOP; ADD.I32.SAT V3, V2, S1, VP6
NOP; NOP; ADD.I32.SAT V3, V2, S1, !VP6

// CHECK: nop;    nop;    add.i32  st V3, V2, S1;    nop
// CHECK: nop;    nop;    add.i32  st V3, V2, S1, SP6;    nop
// CHECK: nop;    nop;    add.i32  st V3, V2, S1, !SP6;   nop
// CHECK: nop;    nop;    add.i32  st V3, V2, S1, VP6;    nop
// CHECK: nop;    nop;    add.i32  st V3, V2, S1, !VP6;   nop
