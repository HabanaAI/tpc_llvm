// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s


NOP; NOP; FCLASS.F32 V6, V7;       NOP;
NOP; NOP; FCLASS.F32 V6, V7, SP1;  NOP;
NOP; NOP; FCLASS.F32 V6, V7, !SP1; NOP;
NOP; NOP; FCLASS.F32 V6, V7, VP1;  NOP;
NOP; NOP; FCLASS.F32 V6, V7, !VP1; NOP;

// CHECK: nop;    nop;    fclass.f32  V6, V7;        nop
// CHECK: nop;    nop;    fclass.f32  V6, V7, SP1;        nop
// CHECK: nop;    nop;    fclass.f32  V6, V7, !SP1;       nop
// CHECK: nop;    nop;    fclass.f32  V6, V7, VP1;        nop
// CHECK: nop;    nop;    fclass.f32  V6, V7, !VP1;       nop


NOP; NOP; FCLASS.F32 V6, S7;       NOP;
NOP; NOP; FCLASS.F32 V6, S7, SP1;  NOP;
NOP; NOP; FCLASS.F32 V6, S7, !SP1; NOP;
NOP; NOP; FCLASS.F32 V6, S7, VP1;  NOP;
NOP; NOP; FCLASS.F32 V6, S7, !VP1; NOP;

// CHECK: nop;    nop;    fclass.f32  V6, S7;        nop
// CHECK: nop;    nop;    fclass.f32  V6, S7, SP1;        nop
// CHECK: nop;    nop;    fclass.f32  V6, S7, !SP1;       nop
// CHECK: nop;    nop;    fclass.f32  V6, S7, VP1;        nop
// CHECK: nop;    nop;    fclass.f32  V6, S7, !VP1;       nop


NOP; NOP; FCLASS.BF16 V6, V7;       NOP;
NOP; NOP; FCLASS.BF16 V6, V7, SP1;  NOP;
NOP; NOP; FCLASS.BF16 V6, V7, !SP1; NOP;
NOP; NOP; FCLASS.BF16 V6, V7, VP1;  NOP;
NOP; NOP; FCLASS.BF16 V6, V7, !VP1; NOP;

// CHECK: nop;    nop;    fclass.bf16  V6, V7;       nop
// CHECK: nop;    nop;    fclass.bf16  V6, V7, SP1;       nop
// CHECK: nop;    nop;    fclass.bf16  V6, V7, !SP1;      nop
// CHECK: nop;    nop;    fclass.bf16  V6, V7, VP1;       nop
// CHECK: nop;    nop;    fclass.bf16  V6, V7, !VP1;      nop


NOP; NOP; FCLASS.BF16 V6, S7;       NOP;
NOP; NOP; FCLASS.BF16 V6, S7, SP1;  NOP;
NOP; NOP; FCLASS.BF16 V6, S7, !SP1; NOP;
NOP; NOP; FCLASS.BF16 V6, S7, VP1;  NOP;
NOP; NOP; FCLASS.BF16 V6, S7, !VP1; NOP;

// CHECK: nop;    nop;    fclass.bf16  V6, S7;       nop
// CHECK: nop;    nop;    fclass.bf16  V6, S7, SP1;       nop
// CHECK: nop;    nop;    fclass.bf16  V6, S7, !SP1;      nop
// CHECK: nop;    nop;    fclass.bf16  V6, S7, VP1;       nop
// CHECK: nop;    nop;    fclass.bf16  V6, S7, !VP1;      nop