// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s


LD_TNSR_HIGH V4, 0, I3
LD_TNSR_HIGH V4, 0, I3, SP0
LD_TNSR_HIGH V4, 0, I3, SP1
LD_TNSR_HIGH V4, 0, I3, !SP1

// CHECK: ld_tnsr_high  V4, 0x0, I3, SP0;         nop;    nop;    nop
// CHECK: ld_tnsr_high  V4, 0x0, I3, SP0;         nop;    nop;    nop
// CHECK: ld_tnsr_high  V4, 0x0, I3, SP1;         nop;    nop;    nop
// CHECK: ld_tnsr_high  V4, 0x0, I3, !SP1;        nop;    nop;    nop


LD_TNSR_HIGH VP4, 0, I3
LD_TNSR_HIGH VP4, 0, I3, SP0
LD_TNSR_HIGH VP4, 0, I3, SP1
LD_TNSR_HIGH VP4, 0, I3, !SP1

// CHECK: ld_tnsr_high  VP4, 0x0, I3, SP0;        nop;    nop;    nop
// CHECK: ld_tnsr_high  VP4, 0x0, I3, SP0;        nop;    nop;    nop
// CHECK: ld_tnsr_high  VP4, 0x0, I3, SP1;        nop;    nop;    nop
// CHECK: ld_tnsr_high  VP4, 0x0, I3, !SP1;       nop;    nop;    nop
