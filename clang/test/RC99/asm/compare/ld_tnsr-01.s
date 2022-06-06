// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s


LD_TNSR V4, 0, I3
LD_TNSR V4, 0, I3, SP0
LD_TNSR V4, 0, I3, SP1
LD_TNSR V4, 0, I3, !SP1

// CHECK: ld_tnsr  V4, 0x0, I3;      nop;    nop;    nop
// CHECK: ld_tnsr  V4, 0x0, I3;      nop;    nop;    nop
// CHECK: ld_tnsr  V4, 0x0, I3, SP1;      nop;    nop;    nop
// CHECK: ld_tnsr  V4, 0x0, I3, !SP1;     nop;    nop;    nop


LD_TNSR VP4, 0, I3
LD_TNSR VP4, 0, I3, SP0
LD_TNSR VP4, 0, I3, SP1
LD_TNSR VP4, 0, I3, !SP1

// CHECK: ld_tnsr  VP4, 0x0, I3;     nop;    nop;    nop
// CHECK: ld_tnsr  VP4, 0x0, I3;     nop;    nop;    nop
// CHECK: ld_tnsr  VP4, 0x0, I3, SP1;     nop;    nop;    nop
// CHECK: ld_tnsr  VP4, 0x0, I3, !SP1;    nop;    nop;    nop
