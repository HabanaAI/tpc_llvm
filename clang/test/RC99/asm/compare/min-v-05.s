// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu doron1 %s -o %t.o
// RUN: %disasm --mcpu doron1 %t.o | FileCheck %s

NOP; NOP; MIN.F32.SUP_NAN V5, V1, V2
NOP; NOP; MIN.F32.SUP_NAN V5, V1, V2, SP3
NOP; NOP; MIN.F32.SUP_NAN V5, V1, V2, !SP3
NOP; NOP; MIN.F32.SUP_NAN V5, V1, V2, VP3
NOP; NOP; MIN.F32.SUP_NAN V5, V1, V2, !VP3

// CHECK: nop;    nop;    min.f32 sup_nan  V5, V1, V2;       nop
// CHECK: nop;    nop;    min.f32 sup_nan  V5, V1, V2, SP3;       nop
// CHECK: nop;    nop;    min.f32 sup_nan  V5, V1, V2, !SP3;      nop
// CHECK: nop;    nop;    min.f32 sup_nan  V5, V1, V2, VP3;       nop
// CHECK: nop;    nop;    min.f32 sup_nan  V5, V1, V2, !VP3;      nop

NOP; NOP; MIN.F32.SUP_NAN V5, V1, S2
NOP; NOP; MIN.F32.SUP_NAN V5, V1, S2, SP3
NOP; NOP; MIN.F32.SUP_NAN V5, V1, S2, !SP3
NOP; NOP; MIN.F32.SUP_NAN V5, V1, S2, VP3
NOP; NOP; MIN.F32.SUP_NAN V5, V1, S2, !VP3

// CHECK: nop;    nop;    min.f32 sup_nan  V5, V1, S2;       nop
// CHECK: nop;    nop;    min.f32 sup_nan  V5, V1, S2, SP3;       nop
// CHECK: nop;    nop;    min.f32 sup_nan  V5, V1, S2, !SP3;      nop
// CHECK: nop;    nop;    min.f32 sup_nan  V5, V1, S2, VP3;       nop
// CHECK: nop;    nop;    min.f32 sup_nan  V5, V1, S2, !VP3;      nop

NOP; NOP; MIN.F32.SUP_NAN V5, V1, 0x3f4ccccd
NOP; NOP; MIN.F32.SUP_NAN V5, V1, 0x3f4ccccd, SP3
NOP; NOP; MIN.F32.SUP_NAN V5, V1, 0x3f4ccccd, !SP3
NOP; NOP; MIN.F32.SUP_NAN V5, V1, 0x3f4ccccd, VP3
NOP; NOP; MIN.F32.SUP_NAN V5, V1, 0x3f4ccccd, !VP3

// CHECK: nop;    nop;    min.f32 sup_nan  V5, V1, 0x3f4ccccd;       nop
// CHECK: nop;    nop;    min.f32 sup_nan  V5, V1, 0x3f4ccccd, SP3;       nop
// CHECK: nop;    nop;    min.f32 sup_nan  V5, V1, 0x3f4ccccd, !SP3;      nop
// CHECK: nop;    nop;    min.f32 sup_nan  V5, V1, 0x3f4ccccd, VP3;       nop
// CHECK: nop;    nop;    min.f32 sup_nan  V5, V1, 0x3f4ccccd, !VP3;      nop
