// RUN: %tpc_clang -c -x assembler -march=goya2 %s -o %ttest.o
// RUN: %disasm --mcpu=goya2 %ttest.o | FileCheck %s

ld_g  V2, AD0, SP0; set_indx  I4, b00001, S15, SP0; nop; gen_addr dt=fp32 AD0, 0x0, I8, SP0
// CHECK: gen_addr dt=fp32 AD0, 0x0, I8

ld_g  V2, AD0, SP0; set_indx  I4, b00001, S15, SP0; nop; gen_addr dt=fp32 AD0, 0x0, I16, SP0
// CHECK: gen_addr dt=fp32 AD0, 0x0, I16

gen_addr dt=fp32 AD0, 0x0, I8, SP0; nop; nop; nop
// CHECK: gen_addr dt=fp32 AD0, 0x0, I8

gen_addr dt=fp32 AD0, 0x0, I16, SP0; nop; nop; nop
// CHECK: gen_addr dt=fp32 AD0, 0x0, I16

