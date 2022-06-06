// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

	.text
	.globl	main
main:

GEN_ADDR AD0, 7, I0; NOP; NOP; NOP
GEN_ADDR AD1, 6, I1; NOP; NOP; NOP
GEN_ADDR AD2, 5, I2; NOP; NOP; NOP
GEN_ADDR AD3, 4, I3; NOP; NOP; NOP
GEN_ADDR AD4, 3, I15; NOP; NOP; NOP
GEN_ADDR AD5, 2, I29; NOP; NOP; NOP
GEN_ADDR AD6, 1, I30; NOP; NOP; NOP
GEN_ADDR AD7, 0, I31; NOP; NOP; NOP

// CHECK: gen_addr AD0, 0x7, I0;  nop;    nop;    nop
// CHECK: gen_addr AD1, 0x6, I1;  nop;    nop;    nop
// CHECK: gen_addr AD2, 0x5, I2;  nop;    nop;    nop
// CHECK: gen_addr AD3, 0x4, I3;  nop;    nop;    nop
// CHECK: gen_addr AD4, 0x3, I15;         nop;    nop;    nop
// CHECK: gen_addr AD5, 0x2, I29;         nop;    nop;    nop
// CHECK: gen_addr AD6, 0x1, I30;         nop;    nop;    nop
// CHECK: gen_addr AD7, 0x0, I31;         nop;    nop;    nop


GEN_ADDR AD1, 2, I3, SP0; NOP; NOP; NOP
GEN_ADDR AD1, 2, I3, SP1; NOP; NOP; NOP
GEN_ADDR AD1, 2, I3, SP2; NOP; NOP; NOP
GEN_ADDR AD1, 2, I3, SP3; NOP; NOP; NOP
GEN_ADDR AD1, 2, I3, SP4; NOP; NOP; NOP
GEN_ADDR AD1, 2, I3, SP13; NOP; NOP; NOP
GEN_ADDR AD1, 2, I3, SP14; NOP; NOP; NOP
GEN_ADDR AD1, 2, I3, SP15; NOP; NOP; NOP

// CHECK: gen_addr AD1, 0x2, I3;  nop;    nop;    nop
// CHECK: gen_addr AD1, 0x2, I3, SP1;  nop;    nop;    nop
// CHECK: gen_addr AD1, 0x2, I3, SP2;  nop;    nop;    nop
// CHECK: gen_addr AD1, 0x2, I3, SP3;  nop;    nop;    nop
// CHECK: gen_addr AD1, 0x2, I3, SP4;  nop;    nop;    nop
// CHECK: gen_addr AD1, 0x2, I3, SP13;         nop;    nop;    nop
// CHECK: gen_addr AD1, 0x2, I3, SP14;         nop;    nop;    nop
// CHECK: gen_addr AD1, 0x2, I3, SP15;         nop;    nop;    nop


GEN_ADDR AD1, 2, I3, !SP0; NOP; NOP; NOP
GEN_ADDR AD1, 2, I3, !SP1; NOP; NOP; NOP
GEN_ADDR AD1, 2, I3, !SP2; NOP; NOP; NOP
GEN_ADDR AD1, 2, I3, !SP3; NOP; NOP; NOP
GEN_ADDR AD1, 2, I3, !SP4; NOP; NOP; NOP
GEN_ADDR AD1, 2, I3, !SP13; NOP; NOP; NOP
GEN_ADDR AD1, 2, I3, !SP14; NOP; NOP; NOP
GEN_ADDR AD1, 2, I3, !SP15; NOP; NOP; NOP

// CHECK: gen_addr AD1, 0x2, I3, !SP0;         nop;    nop;    nop
// CHECK: gen_addr AD1, 0x2, I3, !SP1;         nop;    nop;    nop
// CHECK: gen_addr AD1, 0x2, I3, !SP2;         nop;    nop;    nop
// CHECK: gen_addr AD1, 0x2, I3, !SP3;         nop;    nop;    nop
// CHECK: gen_addr AD1, 0x2, I3, !SP4;         nop;    nop;    nop
// CHECK: gen_addr AD1, 0x2, I3, !SP13;        nop;    nop;    nop
// CHECK: gen_addr AD1, 0x2, I3, !SP14;        nop;    nop;    nop
// CHECK: gen_addr AD1, 0x2, I3, !SP15;        nop;    nop;    nop
