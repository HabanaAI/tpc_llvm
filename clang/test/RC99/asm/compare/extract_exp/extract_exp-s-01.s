// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s


NOP; EXTRACT_EXP.F32 S1, S2; NOP; NOP
NOP; EXTRACT_EXP.F32 S1, S2, SP3; NOP; NOP
NOP; EXTRACT_EXP.F32 S1, S2, !SP3; NOP; NOP

// CHECK: nop; 	extract_exp.f32  S1, S2, SP0; 	nop; 	nop
// CHECK: nop; 	extract_exp.f32  S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	extract_exp.f32  S1, S2, !SP3; 	nop; 	nop


NOP; EXTRACT_EXP.F32 S1, S2, SUBTRACT_BIAS; NOP; NOP
NOP; EXTRACT_EXP.F32 S1, S2, SUBTRACT_BIAS, SP3; NOP; NOP
NOP; EXTRACT_EXP.F32 S1, S2, SUBTRACT_BIAS, !SP3; NOP; NOP

// CHECK: nop; 	extract_exp.f32  subtract_bias S1, S2, SP0; 	nop; 	nop
// CHECK: nop; 	extract_exp.f32  subtract_bias S1, S2, SP3; 	nop; 	nop
// CHECK: nop; 	extract_exp.f32  subtract_bias S1, S2, !SP3; 	nop; 	nop


NOP; EXTRACT_EXP.F32 S1, 0x41000000; NOP; NOP
NOP; EXTRACT_EXP.F32 S1, 0x41000000, SP3; NOP; NOP
NOP; EXTRACT_EXP.F32 S1, 0x41000000, !SP3; NOP; NOP

// CHECK: nop; 	extract_exp.f32  S1, 0x41000000, SP0; 	nop; 	nop
// CHECK: nop; 	extract_exp.f32  S1, 0x41000000, SP3; 	nop; 	nop
// CHECK: nop; 	extract_exp.f32  S1, 0x41000000, !SP3; 	nop; 	nop


NOP; EXTRACT_EXP.F32 S1, 0x41000000, SUBTRACT_BIAS; NOP; NOP
NOP; EXTRACT_EXP.F32 S1, 0x41000000, SUBTRACT_BIAS, SP3; NOP; NOP
NOP; EXTRACT_EXP.F32 S1, 0x41000000, SUBTRACT_BIAS, !SP3; NOP; NOP

// CHECK: nop; 	extract_exp.f32  subtract_bias S1, 0x41000000, SP0; 	nop; 	nop
// CHECK: nop; 	extract_exp.f32  subtract_bias S1, 0x41000000, SP3; 	nop; 	nop
// CHECK: nop; 	extract_exp.f32  subtract_bias S1, 0x41000000, !SP3; 	nop; 	nop
