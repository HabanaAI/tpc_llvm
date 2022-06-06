// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s

NOP; BREV.F32  S1, S2; NOP; NOP
NOP; BREV.F32  S1, S2, SP3; NOP; NOP
NOP; BREV.F32  S1, S2, !SP3; NOP; NOP

// CHECK: nop;    brev.f32  S1, S2;  nop;    nop
// CHECK: nop;    brev.f32  S1, S2, SP3;  nop;    nop
// CHECK: nop;    brev.f32  S1, S2, !SP3;         nop;    nop


NOP; BREV.BF16 S1, S2; NOP; NOP
NOP; BREV.BF16 S1, S2, SP3; NOP; NOP
NOP; BREV.BF16 S1, S2, !SP3; NOP; NOP

// CHECK: nop;    brev.bf16  S1, S2;         nop;    nop
// CHECK: nop;    brev.bf16  S1, S2, SP3;         nop;    nop
// CHECK: nop;    brev.bf16  S1, S2, !SP3;        nop;    nop


NOP; BREV.I32  S1, S2; NOP; NOP
NOP; BREV.I32  S1, S2, SP3; NOP; NOP
NOP; BREV.I32  S1, S2, !SP3; NOP; NOP

// CHECK: nop;    brev.i32  S1, S2;  nop;    nop
// CHECK: nop;    brev.i32  S1, S2, SP3;  nop;    nop
// CHECK: nop;    brev.i32  S1, S2, !SP3;         nop;    nop


NOP; BREV.U32  S1, S2; NOP; NOP
NOP; BREV.U32  S1, S2, SP3; NOP; NOP
NOP; BREV.U32  S1, S2, !SP3; NOP; NOP

// CHECK: nop;    brev.u32  S1, S2;  nop;    nop
// CHECK: nop;    brev.u32  S1, S2, SP3;  nop;    nop
// CHECK: nop;    brev.u32  S1, S2, !SP3;         nop;    nop


NOP; BREV.I16  S1, S2; NOP; NOP
NOP; BREV.I16  S1, S2, SP3; NOP; NOP
NOP; BREV.I16  S1, S2, !SP3; NOP; NOP

// CHECK: nop;    brev.i16  S1, S2;  nop;    nop
// CHECK: nop;    brev.i16  S1, S2, SP3;  nop;    nop
// CHECK: nop;    brev.i16  S1, S2, !SP3;         nop;    nop


NOP; BREV.U16  S1, S2; NOP; NOP
NOP; BREV.U16  S1, S2, SP3; NOP; NOP
NOP; BREV.U16  S1, S2, !SP3; NOP; NOP

// CHECK: nop;    brev.u16  S1, S2;  nop;    nop
// CHECK: nop;    brev.u16  S1, S2, SP3;  nop;    nop
// CHECK: nop;    brev.u16  S1, S2, !SP3;         nop;    nop


NOP; BREV.I8  S1, S2; NOP; NOP
NOP; BREV.I8  S1, S2, SP3; NOP; NOP
NOP; BREV.I8  S1, S2, !SP3; NOP; NOP

// CHECK: nop;    brev.i8  S1, S2;   nop;    nop
// CHECK: nop;    brev.i8  S1, S2, SP3;   nop;    nop
// CHECK: nop;    brev.i8  S1, S2, !SP3;  nop;    nop


NOP; BREV.U8  S1, S2; NOP; NOP
NOP; BREV.U8  S1, S2, SP3; NOP; NOP
NOP; BREV.U8  S1, S2, !SP3; NOP; NOP

// CHECK: nop;    brev.u8  S1, S2;   nop;    nop
// CHECK: nop;    brev.u8  S1, S2, SP3;   nop;    nop
// CHECK: nop;    brev.u8  S1, S2, !SP3;  nop;    nop
