// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi  -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2  -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s


LD_L S1, S2
LD_L S1, S2, SP3
LD_L S1, S2, !SP3

// CHECK: ld_l  S1, S2;      nop;    nop;    nop
// CHECK: ld_l  S1, S2, SP3;      nop;    nop;    nop
// CHECK: ld_l  S1, S2, !SP3;     nop;    nop;    nop


LD_L S1, 123
LD_L S1, 123, SP3
LD_L S1, 123, !SP3

// CHECK: ld_l  S1, 0x7b;    nop;    nop;    nop
// CHECK: ld_l  S1, 0x7b, SP3;    nop;    nop;    nop
// CHECK: ld_l  S1, 0x7b, !SP3;   nop;    nop;    nop


LD_L SP1, S2
LD_L SP1, S2, SP3
LD_L SP1, S2, !SP3

// CHECK: ld_l  SP1, S2;     nop;    nop;    nop
// CHECK: ld_l  SP1, S2, SP3;     nop;    nop;    nop
// CHECK: ld_l  SP1, S2, !SP3;    nop;    nop;    nop


LD_L SP1, 123
LD_L SP1, 123, SP3
LD_L SP1, 123, !SP3

// CHECK: ld_l  SP1, 0x7b;   nop;    nop;    nop
// CHECK: ld_l  SP1, 0x7b, SP3;   nop;    nop;    nop
// CHECK: ld_l  SP1, 0x7b, !SP3;  nop;    nop;    nop


LD_L S1, S2, MMIO
LD_L S1, S2, MMIO, SP3
LD_L S1, S2, MMIO, !SP3

// CHECK: ld_l  mmio S1, S2;         nop;    nop;    nop
// CHECK: ld_l  mmio S1, S2, SP3;         nop;    nop;    nop
// CHECK: ld_l  mmio S1, S2, !SP3;        nop;    nop;    nop


LD_L S1, 123, MMIO
LD_L S1, 123, MMIO, SP3
LD_L S1, 123, MMIO, !SP3

// CHECK: ld_l  mmio S1, 0x7b;       nop;    nop;    nop
// CHECK: ld_l  mmio S1, 0x7b, SP3;       nop;    nop;    nop
// CHECK: ld_l  mmio S1, 0x7b, !SP3;      nop;    nop;    nop


LD_L SP1, S2, MMIO
LD_L SP1, S2, MMIO, SP3
LD_L SP1, S2, MMIO, !SP3

// CHECK: ld_l  mmio SP1, S2;        nop;    nop;    nop
// CHECK: ld_l  mmio SP1, S2, SP3;        nop;    nop;    nop
// CHECK: ld_l  mmio SP1, S2, !SP3;       nop;    nop;    nop


LD_L SP1, 123, MMIO
LD_L SP1, 123, MMIO, SP3
LD_L SP1, 123, MMIO, !SP3

// CHECK: ld_l  mmio SP1, 0x7b;      nop;    nop;    nop
// CHECK: ld_l  mmio SP1, 0x7b, SP3;      nop;    nop;    nop
// CHECK: ld_l  mmio SP1, 0x7b, !SP3;     nop;    nop;    nop
