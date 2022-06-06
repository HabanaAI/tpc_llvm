// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

	.text
	.globl	main

NOP; MOV_IRF_DIM 0 S2, I4
NOP; MOV_IRF_DIM 1 S2, I4
NOP; MOV_IRF_DIM 2 S2, I4
NOP; MOV_IRF_DIM 3 S2, I4
NOP; MOV_IRF_DIM 4 S2, I4

// CHECK: nop; mov_irf_dim 0x0 S2, I4; nop; nop
// CHECK: nop; mov_irf_dim 0x1 S2, I4; nop; nop
// CHECK: nop; mov_irf_dim 0x2 S2, I4; nop; nop
// CHECK: nop; mov_irf_dim 0x3 S2, I4; nop; nop
// CHECK: nop; mov_irf_dim 0x4 S2, I4; nop; nop

NOP; MOV_IRF_DIM 0 S1, I31
NOP; MOV_IRF_DIM 0 S2, I30
NOP; MOV_IRF_DIM 0 S3, I29
NOP; MOV_IRF_DIM 0 S4, I28
NOP; MOV_IRF_DIM 0 S5, I27
NOP; MOV_IRF_DIM 0 S6, I26
NOP; MOV_IRF_DIM 0 S7, I25

// CHECK: nop; mov_irf_dim 0x0 S1, I31; nop; nop
// CHECK: nop; mov_irf_dim 0x0 S2, I30; nop; nop
// CHECK: nop; mov_irf_dim 0x0 S3, I29; nop; nop
// CHECK: nop; mov_irf_dim 0x0 S4, I28; nop; nop
// CHECK: nop; mov_irf_dim 0x0 S5, I27; nop; nop
// CHECK: nop; mov_irf_dim 0x0 S6, I26; nop; nop
// CHECK: nop; mov_irf_dim 0x0 S7, I25; nop; nop

NOP; MOV_IRF_DIM 1 S35, I0
NOP; MOV_IRF_DIM 1 S34, I1
NOP; MOV_IRF_DIM 1 S33, I2
NOP; MOV_IRF_DIM 1 S32, I3
NOP; MOV_IRF_DIM 1 S31, I4
NOP; MOV_IRF_DIM 1 S30, I5
NOP; MOV_IRF_DIM 1 S29, I6
NOP; MOV_IRF_DIM 1 S28, I7

// CHECK: nop; mov_irf_dim 0x1 S35, I0; nop; nop
// CHECK: nop; mov_irf_dim 0x1 S34, I1; nop; nop
// CHECK: nop; mov_irf_dim 0x1 S33, I2; nop; nop
// CHECK: nop; mov_irf_dim 0x1 S32, I3; nop; nop
// CHECK: nop; mov_irf_dim 0x1 S31, I4; nop; nop
// CHECK: nop; mov_irf_dim 0x1 S30, I5; nop; nop
// CHECK: nop; mov_irf_dim 0x1 S29, I6; nop; nop
// CHECK: nop; mov_irf_dim 0x1 S28, I7; nop; nop

NOP; MOV_IRF_DIM 2 S2, I3, SP0
NOP; MOV_IRF_DIM 2 S2, I3, SP1
NOP; MOV_IRF_DIM 2 S2, I3, SP2
NOP; MOV_IRF_DIM 2 S2, I3, SP3
NOP; MOV_IRF_DIM 2 S2, I3, SP12
NOP; MOV_IRF_DIM 2 S2, I3, SP13
NOP; MOV_IRF_DIM 2 S2, I3, SP14
NOP; MOV_IRF_DIM 2 S2, I3, SP15

// CHECK: nop; mov_irf_dim 0x2 S2, I3; nop; nop
// CHECK: nop; mov_irf_dim 0x2 S2, I3, SP1; nop; nop
// CHECK: nop; mov_irf_dim 0x2 S2, I3, SP2; nop; nop
// CHECK: nop; mov_irf_dim 0x2 S2, I3, SP3; nop; nop
// CHECK: nop; mov_irf_dim 0x2 S2, I3, SP12; nop; nop
// CHECK: nop; mov_irf_dim 0x2 S2, I3, SP13; nop; nop
// CHECK: nop; mov_irf_dim 0x2 S2, I3, SP14; nop; nop
// CHECK: nop; mov_irf_dim 0x2 S2, I3, SP15; nop; nop

NOP; MOV_IRF_DIM 2 S2, I3, !SP0
NOP; MOV_IRF_DIM 2 S2, I3, !SP1
NOP; MOV_IRF_DIM 2 S2, I3, !SP2
NOP; MOV_IRF_DIM 2 S2, I3, !SP3
NOP; MOV_IRF_DIM 2 S2, I3, !SP12
NOP; MOV_IRF_DIM 2 S2, I3, !SP13
NOP; MOV_IRF_DIM 2 S2, I3, !SP14
NOP; MOV_IRF_DIM 2 S2, I3, !SP15

// CHECK: nop; mov_irf_dim 0x2 S2, I3, !SP0; nop; nop
// CHECK: nop; mov_irf_dim 0x2 S2, I3, !SP1; nop; nop
// CHECK: nop; mov_irf_dim 0x2 S2, I3, !SP2; nop; nop
// CHECK: nop; mov_irf_dim 0x2 S2, I3, !SP3; nop; nop
// CHECK: nop; mov_irf_dim 0x2 S2, I3, !SP12; nop; nop
// CHECK: nop; mov_irf_dim 0x2 S2, I3, !SP13; nop; nop
// CHECK: nop; mov_irf_dim 0x2 S2, I3, !SP14; nop; nop
// CHECK: nop; mov_irf_dim 0x2 S2, I3, !SP15; nop; nop
