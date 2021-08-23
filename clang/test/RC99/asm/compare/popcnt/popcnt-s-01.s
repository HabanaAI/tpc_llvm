// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s



NOP; POPCNT.F32 0 S1, S2; NOP; NOP
NOP; POPCNT.F32 0 S1, S2, SP3; NOP; NOP
NOP; POPCNT.F32 0 S1, S2, !SP3; NOP; NOP
NOP; POPCNT.F32 1 S1, S2; NOP; NOP
NOP; POPCNT.F32 1 S1, S2, SP3; NOP; NOP
NOP; POPCNT.F32 1 S1, S2, !SP3; NOP; NOP

// CHECK: nop;    popcnt.f32 set=0 S1, S2, SP0;   nop;    nop
// CHECK: nop;    popcnt.f32 set=0 S1, S2, SP3;   nop;    nop
// CHECK: nop;    popcnt.f32 set=0 S1, S2, !SP3;  nop;    nop
// CHECK: nop;    popcnt.f32  set=1 S1, S2, SP0;  nop;    nop
// CHECK: nop;    popcnt.f32  set=1 S1, S2, SP3;  nop;    nop
// CHECK: nop;    popcnt.f32  set=1 S1, S2, !SP3;         nop;    nop


NOP; POPCNT.I32 0 S1, S2; NOP; NOP
NOP; POPCNT.I32 0 S1, S2, SP3; NOP; NOP
NOP; POPCNT.I32 0 S1, S2, !SP3; NOP; NOP
NOP; POPCNT.I32 1 S1, S2; NOP; NOP
NOP; POPCNT.I32 1 S1, S2, SP3; NOP; NOP
NOP; POPCNT.I32 1 S1, S2, !SP3; NOP; NOP

// CHECK: nop;    popcnt.i32 set=0 S1, S2, SP0;   nop;    nop
// CHECK: nop;    popcnt.i32 set=0 S1, S2, SP3;   nop;    nop
// CHECK: nop;    popcnt.i32 set=0 S1, S2, !SP3;  nop;    nop
// CHECK: nop;    popcnt.i32  set=1 S1, S2, SP0;  nop;    nop
// CHECK: nop;    popcnt.i32  set=1 S1, S2, SP3;  nop;    nop
// CHECK: nop;    popcnt.i32  set=1 S1, S2, !SP3;         nop;    nop


NOP; POPCNT.U32 0 S1, S2; NOP; NOP
NOP; POPCNT.U32 0 S1, S2, SP3; NOP; NOP
NOP; POPCNT.U32 0 S1, S2, !SP3; NOP; NOP
NOP; POPCNT.U32 1 S1, S2; NOP; NOP
NOP; POPCNT.U32 1 S1, S2, SP3; NOP; NOP
NOP; POPCNT.U32 1 S1, S2, !SP3; NOP; NOP

// CHECK: nop;    popcnt.u32 set=0 S1, S2, SP0;   nop;    nop
// CHECK: nop;    popcnt.u32 set=0 S1, S2, SP3;   nop;    nop
// CHECK: nop;    popcnt.u32 set=0 S1, S2, !SP3;  nop;    nop
// CHECK: nop;    popcnt.u32  set=1 S1, S2, SP0;  nop;    nop
// CHECK: nop;    popcnt.u32  set=1 S1, S2, SP3;  nop;    nop
// CHECK: nop;    popcnt.u32  set=1 S1, S2, !SP3;         nop;    nop


NOP; POPCNT.I16 0 S1, S2; NOP; NOP
NOP; POPCNT.I16 0 S1, S2, SP3; NOP; NOP
NOP; POPCNT.I16 0 S1, S2, !SP3; NOP; NOP
NOP; POPCNT.I16 1 S1, S2; NOP; NOP
NOP; POPCNT.I16 1 S1, S2, SP3; NOP; NOP
NOP; POPCNT.I16 1 S1, S2, !SP3; NOP; NOP

// CHECK: nop;    popcnt.i16 set=0 S1, S2, SP0;   nop;    nop
// CHECK: nop;    popcnt.i16 set=0 S1, S2, SP3;   nop;    nop
// CHECK: nop;    popcnt.i16 set=0 S1, S2, !SP3;  nop;    nop
// CHECK: nop;    popcnt.i16  set=1 S1, S2, SP0;  nop;    nop
// CHECK: nop;    popcnt.i16  set=1 S1, S2, SP3;  nop;    nop
// CHECK: nop;    popcnt.i16  set=1 S1, S2, !SP3;         nop;    nop


NOP; POPCNT.U16 0 S1, S2; NOP; NOP
NOP; POPCNT.U16 0 S1, S2, SP3; NOP; NOP
NOP; POPCNT.U16 0 S1, S2, !SP3; NOP; NOP
NOP; POPCNT.U16 1 S1, S2; NOP; NOP
NOP; POPCNT.U16 1 S1, S2, SP3; NOP; NOP
NOP; POPCNT.U16 1 S1, S2, !SP3; NOP; NOP

// CHECK: nop;    popcnt.u16 set=0 S1, S2, SP0;   nop;    nop
// CHECK: nop;    popcnt.u16 set=0 S1, S2, SP3;   nop;    nop
// CHECK: nop;    popcnt.u16 set=0 S1, S2, !SP3;  nop;    nop
// CHECK: nop;    popcnt.u16  set=1 S1, S2, SP0;  nop;    nop
// CHECK: nop;    popcnt.u16  set=1 S1, S2, SP3;  nop;    nop
// CHECK: nop;    popcnt.u16  set=1 S1, S2, !SP3;         nop;    nop


NOP; POPCNT.I8 0 S1, S2; NOP; NOP
NOP; POPCNT.I8 0 S1, S2, SP3; NOP; NOP
NOP; POPCNT.I8 0 S1, S2, !SP3; NOP; NOP
NOP; POPCNT.I8 1 S1, S2; NOP; NOP
NOP; POPCNT.I8 1 S1, S2, SP3; NOP; NOP
NOP; POPCNT.I8 1 S1, S2, !SP3; NOP; NOP

// CHECK: nop;    popcnt.i8 set=0 S1, S2, SP0;    nop;    nop
// CHECK: nop;    popcnt.i8 set=0 S1, S2, SP3;    nop;    nop
// CHECK: nop;    popcnt.i8 set=0 S1, S2, !SP3;   nop;    nop
// CHECK: nop;    popcnt.i8  set=1 S1, S2, SP0;   nop;    nop
// CHECK: nop;    popcnt.i8  set=1 S1, S2, SP3;   nop;    nop
// CHECK: nop;    popcnt.i8  set=1 S1, S2, !SP3;  nop;    nop


NOP; POPCNT.U8 0 S1, S2; NOP; NOP
NOP; POPCNT.U8 0 S1, S2, SP3; NOP; NOP
NOP; POPCNT.U8 0 S1, S2, !SP3; NOP; NOP
NOP; POPCNT.U8 1 S1, S2; NOP; NOP
NOP; POPCNT.U8 1 S1, S2, SP3; NOP; NOP
NOP; POPCNT.U8 1 S1, S2, !SP3; NOP; NOP

// CHECK: nop;    popcnt.u8 set=0 S1, S2, SP0;    nop;    nop
// CHECK: nop;    popcnt.u8 set=0 S1, S2, SP3;    nop;    nop
// CHECK: nop;    popcnt.u8 set=0 S1, S2, !SP3;   nop;    nop
// CHECK: nop;    popcnt.u8  set=1 S1, S2, SP0;   nop;    nop
// CHECK: nop;    popcnt.u8  set=1 S1, S2, SP3;   nop;    nop
// CHECK: nop;    popcnt.u8  set=1 S1, S2, !SP3;  nop;    nop