// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s


NOP; NOP; EXTRACT_EXP.F32 V1, V2; NOP;
NOP; NOP; EXTRACT_EXP.F32 V1, V2, SP3; NOP
NOP; NOP; EXTRACT_EXP.F32 V1, V2, !SP3; NOP
NOP; NOP; EXTRACT_EXP.F32 V1, V2, VP3; NOP
NOP; NOP; EXTRACT_EXP.F32 V1, V2, !VP3; NOP

// CHECK: nop;    nop;    extract_exp.f32  V1, V2;   nop
// CHECK: nop;    nop;    extract_exp.f32  V1, V2, SP3;   nop
// CHECK: nop;    nop;    extract_exp.f32  V1, V2, !SP3;  nop
// CHECK: nop;    nop;    extract_exp.f32  V1, V2, VP3;   nop
// CHECK: nop;    nop;    extract_exp.f32  V1, V2, !VP3;  nop


NOP; NOP; EXTRACT_EXP.F32 V1, V2, SUBTRACT_BIAS; NOP
NOP; NOP; EXTRACT_EXP.F32 V1, V2, SUBTRACT_BIAS, SP3; NOP
NOP; NOP; EXTRACT_EXP.F32 V1, V2, SUBTRACT_BIAS, !SP3; NOP
NOP; NOP; EXTRACT_EXP.F32 V1, V2, SUBTRACT_BIAS, VP3; NOP
NOP; NOP; EXTRACT_EXP.F32 V1, V2, SUBTRACT_BIAS, !VP3; NOP

// CHECK: nop;    nop;    extract_exp.f32  subtract_bias V1, V2;     nop
// CHECK: nop;    nop;    extract_exp.f32  subtract_bias V1, V2, SP3;     nop
// CHECK: nop;    nop;    extract_exp.f32  subtract_bias V1, V2, !SP3;    nop
// CHECK: nop;    nop;    extract_exp.f32  subtract_bias V1, V2, VP3;     nop
// CHECK: nop;    nop;    extract_exp.f32  subtract_bias V1, V2, !VP3;    nop


NOP; NOP; EXTRACT_EXP.F32 V1, 0x41000000; NOP;
NOP; NOP; EXTRACT_EXP.F32 V1, F32_8.000000, SP3; NOP
NOP; NOP; EXTRACT_EXP.F32 V1, 0x41000000, !SP3; NOP
NOP; NOP; EXTRACT_EXP.F32 V1, 0x41000000, VP3; NOP
NOP; NOP; EXTRACT_EXP.F32 V1, 0x41000000, !VP3; NOP

// CHECK: nop;    nop;    extract_exp.f32  V1, 0x41000000;   nop
// CHECK: nop;    nop;    extract_exp.f32  V1, 0x41000000, SP3;   nop
// CHECK: nop;    nop;    extract_exp.f32  V1, 0x41000000, !SP3;  nop
// CHECK: nop;    nop;    extract_exp.f32  V1, 0x41000000, VP3;   nop
// CHECK: nop;    nop;    extract_exp.f32  V1, 0x41000000, !VP3;  nop


NOP; NOP; EXTRACT_EXP.F32 V1, 0x41000000, SUBTRACT_BIAS; NOP
NOP; NOP; EXTRACT_EXP.F32 V1, 0x41000000, SUBTRACT_BIAS, SP3; NOP
NOP; NOP; EXTRACT_EXP.F32 V1, 0x41000000, SUBTRACT_BIAS, !SP3; NOP
NOP; NOP; EXTRACT_EXP.F32 V1, 0x41000000, SUBTRACT_BIAS, VP3; NOP
NOP; NOP; EXTRACT_EXP.F32 V1, 0x41000000, SUBTRACT_BIAS, !VP3; NOP

// CHECK: nop;    nop;    extract_exp.f32  subtract_bias V1, 0x41000000;     nop
// CHECK: nop;    nop;    extract_exp.f32  subtract_bias V1, 0x41000000, SP3;     nop
// CHECK: nop;    nop;    extract_exp.f32  subtract_bias V1, 0x41000000, !SP3;    nop
// CHECK: nop;    nop;    extract_exp.f32  subtract_bias V1, 0x41000000, VP3;     nop
// CHECK: nop;    nop;    extract_exp.f32  subtract_bias V1, 0x41000000, !VP3;    nop
