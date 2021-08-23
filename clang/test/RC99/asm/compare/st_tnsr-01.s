// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s


NOP; NOP; NOP; ST_TNSR 0, I3, V4
NOP; NOP; NOP; ST_TNSR 0, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR 0, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR 0, I3, V4, !SP1

// CHECK: nop;    nop;    nop;    st_tnsr  0x0, I3, V4, SP0
// CHECK: nop;    nop;    nop;    st_tnsr  0x0, I3, V4, SP0
// CHECK: nop;    nop;    nop;    st_tnsr  0x0, I3, V4, SP1
// CHECK: nop;    nop;    nop;    st_tnsr  0x0, I3, V4, !SP1


NOP; NOP; NOP; ST_TNSR  0, I3, VP4
NOP; NOP; NOP; ST_TNSR  0, I3, VP4,  SP0
NOP; NOP; NOP; ST_TNSR  0, I3, VP4,  SP1
NOP; NOP; NOP; ST_TNSR  0, I3, VP4,  !SP1

// CHECK: nop;    nop;    nop;    st_tnsr  0x0, I3, VP4, SP0
// CHECK: nop;    nop;    nop;    st_tnsr  0x0, I3, VP4, SP0
// CHECK: nop;    nop;    nop;    st_tnsr  0x0, I3, VP4, SP1
// CHECK: nop;    nop;    nop;    st_tnsr  0x0, I3, VP4, !SP1
