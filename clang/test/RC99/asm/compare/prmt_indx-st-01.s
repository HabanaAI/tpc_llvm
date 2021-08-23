// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

	.text
	.globl	main
main:


NOP; NOP; NOP; PRMT_INDX I0, I31, S16
NOP; NOP; NOP; PRMT_INDX I1, I30, S17
NOP; NOP; NOP; PRMT_INDX I2, I29, S18
NOP; NOP; NOP; PRMT_INDX I3, I28, S19
NOP; NOP; NOP; PRMT_INDX I4, I27, S20
NOP; NOP; NOP; PRMT_INDX I5, I26, S21
NOP; NOP; NOP; PRMT_INDX I6, I25, S22
NOP; NOP; NOP; PRMT_INDX I7, I24, S23
NOP; NOP; NOP; PRMT_INDX I8, I25, S24
NOP; NOP; NOP; PRMT_INDX I9, I26, S25

// CHECK: nop;    nop;    nop;    prmt_indx I0, I31, S16, SP0
// CHECK: nop;    nop;    nop;    prmt_indx I1, I30, S17, SP0
// CHECK: nop;    nop;    nop;    prmt_indx I2, I29, S18, SP0
// CHECK: nop;    nop;    nop;    prmt_indx I3, I28, S19, SP0
// CHECK: nop;    nop;    nop;    prmt_indx I4, I27, S20, SP0
// CHECK: nop;    nop;    nop;    prmt_indx I5, I26, S21, SP0
// CHECK: nop;    nop;    nop;    prmt_indx I6, I25, S22, SP0
// CHECK: nop;    nop;    nop;    prmt_indx I7, I24, S23, SP0
// CHECK: nop;    nop;    nop;    prmt_indx I8, I25, S24, SP0
// CHECK: nop;    nop;    nop;    prmt_indx I9, I26, S25, SP0


NOP; NOP; NOP; PRMT_INDX I3, I4, S5, SP0
NOP; NOP; NOP; PRMT_INDX I3, I4, S5, SP1
NOP; NOP; NOP; PRMT_INDX I3, I4, S5, SP2
NOP; NOP; NOP; PRMT_INDX I3, I4, S5, SP3
NOP; NOP; NOP; PRMT_INDX I3, I4, S5, SP12
NOP; NOP; NOP; PRMT_INDX I3, I4, S5, SP13
NOP; NOP; NOP; PRMT_INDX I3, I4, S5, SP14
NOP; NOP; NOP; PRMT_INDX I3, I4, S5, SP15

// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, SP0
// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, SP1
// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, SP2
// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, SP3
// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, SP12
// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, SP13
// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, SP14
// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, SP15


NOP; NOP; NOP; PRMT_INDX I3, I4, S5, !SP0
NOP; NOP; NOP; PRMT_INDX I3, I4, S5, !SP1
NOP; NOP; NOP; PRMT_INDX I3, I4, S5, !SP2
NOP; NOP; NOP; PRMT_INDX I3, I4, S5, !SP3
NOP; NOP; NOP; PRMT_INDX I3, I4, S5, !SP12
NOP; NOP; NOP; PRMT_INDX I3, I4, S5, !SP13
NOP; NOP; NOP; PRMT_INDX I3, I4, S5, !SP14
NOP; NOP; NOP; PRMT_INDX I3, I4, S5, !SP15

// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, !SP0
// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, !SP1
// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, !SP2
// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, !SP3
// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, !SP12
// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, !SP13
// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, !SP14
// CHECK: nop;    nop;    nop;    prmt_indx I3, I4, S5, !SP15
