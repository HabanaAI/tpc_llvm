// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi  -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2  -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s



NOP; NOP; NOP; ST_L S1, S2
NOP; NOP; NOP; ST_L S1, S2, SP3
NOP; NOP; NOP; ST_L S1, S2, !SP3

// CHECK: nop;    nop;    nop;    st_l  S1, S2
// CHECK: nop;    nop;    nop;    st_l  S1, S2, SP3
// CHECK: nop;    nop;    nop;    st_l  S1, S2, !SP3

 
NOP; NOP; NOP; ST_L 123, S2
NOP; NOP; NOP; ST_L 123, S2, SP3
NOP; NOP; NOP; ST_L 123, S2, !SP3

// CHECK: nop;    nop;    nop;    st_l  0x7b, S2
// CHECK: nop;    nop;    nop;    st_l  0x7b, S2, SP3
// CHECK: nop;    nop;    nop;    st_l  0x7b, S2, !SP3


NOP; NOP; NOP; ST_L S1, SP2
NOP; NOP; NOP; ST_L S1, SP2, SP3
NOP; NOP; NOP; ST_L S1, SP2, !SP3

// CHECK: nop;    nop;    nop;    st_l  S1, SP2
// CHECK: nop;    nop;    nop;    st_l  S1, SP2, SP3
// CHECK: nop;    nop;    nop;    st_l  S1, SP2, !SP3


NOP; NOP; NOP; ST_L 123, SP2
NOP; NOP; NOP; ST_L 123, SP2, SP3
NOP; NOP; NOP; ST_L 123, SP2, !SP3

// CHECK: nop;    nop;    nop;    st_l  0x7b, SP2
// CHECK: nop;    nop;    nop;    st_l  0x7b, SP2, SP3
// CHECK: nop;    nop;    nop;    st_l  0x7b, SP2, !SP3


NOP; NOP; NOP; ST_L S1, S2, MMIO
NOP; NOP; NOP; ST_L S1, S2, MMIO, SP3
NOP; NOP; NOP; ST_L S1, S2, MMIO, !SP3

// CHECK: nop;    nop;    nop;    st_l  mmio S1, S2
// CHECK: nop;    nop;    nop;    st_l  mmio S1, S2, SP3
// CHECK: nop;    nop;    nop;    st_l  mmio S1, S2, !SP3

 
NOP; NOP; NOP; ST_L 123, S2, MMIO
NOP; NOP; NOP; ST_L 123, S2, MMIO, SP3
NOP; NOP; NOP; ST_L 123, S2, MMIO, !SP3

// CHECK: nop;    nop;    nop;    st_l  mmio 0x7b, S2
// CHECK: nop;    nop;    nop;    st_l  mmio 0x7b, S2, SP3
// CHECK: nop;    nop;    nop;    st_l  mmio 0x7b, S2, !SP3


NOP; NOP; NOP; ST_L S1, SP2, MMIO
NOP; NOP; NOP; ST_L S1, SP2, MMIO, SP3
NOP; NOP; NOP; ST_L S1, SP2, MMIO, !SP3

// CHECK: nop;    nop;    nop;    st_l  mmio S1, SP2
// CHECK: nop;    nop;    nop;    st_l  mmio S1, SP2, SP3
// CHECK: nop;    nop;    nop;    st_l  mmio S1, SP2, !SP3


NOP; NOP; NOP; ST_L 123, SP2, MMIO
NOP; NOP; NOP; ST_L 123, SP2, MMIO, SP3
NOP; NOP; NOP; ST_L 123, SP2, MMIO, !SP3

// CHECK: nop;    nop;    nop;    st_l  mmio 0x7b, SP2
// CHECK: nop;    nop;    nop;    st_l  mmio 0x7b, SP2, SP3
// CHECK: nop;    nop;    nop;    st_l  mmio 0x7b, SP2, !SP3
