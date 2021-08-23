// RUN: %clang -cc1as -triple tpc -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

// CHECK: ld_g  V2, AD0, SP0; 	set_indx  I4, b00001, S15, SP0; 	nop; 	gen_addr  AD0, 0x0, I8, SP0
ld_g V2, AD0, SP0; set_indx I4, b00001, S15, SP0; nop; gen_addr AD0, 0x0, I8, SP0