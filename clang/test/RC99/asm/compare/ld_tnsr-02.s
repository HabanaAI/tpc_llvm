// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s


LD_TNSR V4, S27, I3
LD_TNSR V4, S27, I3, SP0
LD_TNSR V4, S27, I3, SP1
LD_TNSR V4, S27, I3, !SP1

LD_TNSR VP4, S27, I3
LD_TNSR VP4, S27, I3, SP0
LD_TNSR VP4, S27, I3, SP1
LD_TNSR VP4, S27, I3, !SP1

// CHECK: ld_tnsr  V4, S27, I3;      nop;    nop;    nop
// CHECK: ld_tnsr  V4, S27, I3;      nop;    nop;    nop
// CHECK: ld_tnsr  V4, S27, I3, SP1;      nop;    nop;    nop
// CHECK: ld_tnsr  V4, S27, I3, !SP1;     nop;    nop;    nop
// CHECK: ld_tnsr  VP4, S27, I3;     nop;    nop;    nop
// CHECK: ld_tnsr  VP4, S27, I3;     nop;    nop;    nop
// CHECK: ld_tnsr  VP4, S27, I3, SP1;     nop;    nop;    nop
// CHECK: ld_tnsr  VP4, S27, I3, !SP1;    nop;    nop;    nop


LD_TNSR.PARTIAL V4, 0, I3, S30
LD_TNSR.PARTIAL V4, 0, I3, S30, SP0
LD_TNSR.PARTIAL V4, 0, I3, S30, SP1
LD_TNSR.PARTIAL V4, 0, I3, S30, !SP1

LD_TNSR.PARTIAL VP4, 0, I3, S30
LD_TNSR.PARTIAL VP4, 0, I3, S30, SP0
LD_TNSR.PARTIAL VP4, 0, I3, S30, SP1
LD_TNSR.PARTIAL VP4, 0, I3, S30, !SP1

// CHECK: ld_tnsr  V4, 0x0, I3, S30;         nop;    nop;    nop
// CHECK: ld_tnsr  V4, 0x0, I3, S30;         nop;    nop;    nop
// CHECK: ld_tnsr  V4, 0x0, I3, S30, SP1;         nop;    nop;    nop
// CHECK: ld_tnsr  V4, 0x0, I3, S30, !SP1;        nop;    nop;    nop
// CHECK: ld_tnsr  VP4, 0x0, I3, S30;        nop;    nop;    nop
// CHECK: ld_tnsr  VP4, 0x0, I3, S30;        nop;    nop;    nop
// CHECK: ld_tnsr  VP4, 0x0, I3, S30, SP1;        nop;    nop;    nop
// CHECK: ld_tnsr  VP4, 0x0, I3, S30, !SP1;       nop;    nop;    nop


LD_TNSR.PARTIAL V4, S27, I3, S30
LD_TNSR.PARTIAL V4, S27, I3, S30, SP0
LD_TNSR.PARTIAL V4, S27, I3, S30, SP1
LD_TNSR.PARTIAL V4, S27, I3, S30, !SP1

LD_TNSR.PARTIAL VP4, S27, I3, S30
LD_TNSR.PARTIAL VP4, S27, I3, S30, SP0
LD_TNSR.PARTIAL VP4, S27, I3, S30, SP1
LD_TNSR.PARTIAL VP4, S27, I3, S30, !SP1

// CHECK: ld_tnsr  V4, S27, I3, S30;         nop;    nop;    nop
// CHECK: ld_tnsr  V4, S27, I3, S30;         nop;    nop;    nop
// CHECK: ld_tnsr  V4, S27, I3, S30, SP1;         nop;    nop;    nop
// CHECK: ld_tnsr  V4, S27, I3, S30, !SP1;        nop;    nop;    nop
// CHECK: ld_tnsr  VP4, S27, I3, S30;        nop;    nop;    nop
// CHECK: ld_tnsr  VP4, S27, I3, S30;        nop;    nop;    nop
// CHECK: ld_tnsr  VP4, S27, I3, S30, SP1;        nop;    nop;    nop
// CHECK: ld_tnsr  VP4, S27, I3, S30, !SP1;       nop;    nop;    nop
