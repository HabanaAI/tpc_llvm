// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu gaudi %t.o | FileCheck %s


LD_TNSR_LOW V4, S27, I3
LD_TNSR_LOW V4, S27, I3, SP0
LD_TNSR_LOW V4, S27, I3, SP1
LD_TNSR_LOW V4, S27, I3, !SP1

LD_TNSR_LOW VP4, S27, I3
LD_TNSR_LOW VP4, S27, I3, SP0
LD_TNSR_LOW VP4, S27, I3, SP1
LD_TNSR_LOW VP4, S27, I3, !SP1

// CHECK: ld_tnsr_low  V4, S27, I3, SP0;      nop;    nop;    nop
// CHECK: ld_tnsr_low  V4, S27, I3, SP0;      nop;    nop;    nop
// CHECK: ld_tnsr_low  V4, S27, I3, SP1;      nop;    nop;    nop
// CHECK: ld_tnsr_low  V4, S27, I3, !SP1;     nop;    nop;    nop
// CHECK: ld_tnsr_low  VP4, S27, I3, SP0;     nop;    nop;    nop
// CHECK: ld_tnsr_low  VP4, S27, I3, SP0;     nop;    nop;    nop
// CHECK: ld_tnsr_low  VP4, S27, I3, SP1;     nop;    nop;    nop
// CHECK: ld_tnsr_low  VP4, S27, I3, !SP1;    nop;    nop;    nop
