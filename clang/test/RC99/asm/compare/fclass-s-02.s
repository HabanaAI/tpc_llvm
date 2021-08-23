// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu gaudi %t.o | FileCheck %s

NOP; FCLASS.F32 S6, S7
NOP; FCLASS.F32 S6, S7, SP1;  NOP; NOP
NOP; FCLASS.F32 S6, S7, !SP1; NOP; NOP

// CHECK: nop;    fclass.f32  S6, S7, SP0;        nop;    nop
// CHECK: nop;    fclass.f32  S6, S7, SP1;        nop;    nop
// CHECK: nop;    fclass.f32  S6, S7, !SP1;       nop;    nop


NOP; FCLASS.BF16 S6, S7; NOP; NOP
NOP; FCLASS.BF16 S6, S7, SP1; NOP; NOP
NOP; FCLASS.BF16 S6, S7, !SP1; NOP; NOP

// CHECK: nop;    fclass.bf16  S6, S7, SP0;       nop;    nop
// CHECK: nop;    fclass.bf16  S6, S7, SP1;       nop;    nop
// CHECK: nop;    fclass.bf16  S6, S7, !SP1;      nop;    nop
