// RUN: %tpc_clang -c -x assembler -march=goya2 %s -o %ttest.o
// RUN: %disasm --mcpu=goya2 %ttest.o | FileCheck %s

mov.f32  V15, 0xff800000, SP0; 	nop; and.i32  V7, V7, 0x1, SP0; nop
// CHECK: mov.f32 V15, 0xff800000; nop; and.i32 V7, V7, 0x1; nop

mov VP6, 0x0, SP0; nop; mul.f32 V14, V10, 0x3ee2a8ed, SP0; nop
// CHECK: mov VP6, 0x0; nop; mul.f32 V14, V10, 0x3ee2a8ed; nop
