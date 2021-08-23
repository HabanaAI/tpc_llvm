// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm -no-common-header %t.o | %clang -cc1as -triple tpc-none-none -filetype obj -o %t2.o
// RUN: %disasm %t2.o | FileCheck %s

NOP; NOP; NOP; ST_G AD1, S1
NOP; NOP; NOP; ST_G AD1, S1, SP2
NOP; NOP; NOP; ST_G AD1, S1, !SP2

// CHECK: nop;    nop;    nop;    st_g AD1, S1, SP0
// CHECK: nop;    nop;    nop;    st_g AD1, S1, SP2
// CHECK: nop;    nop;    nop;    st_g AD1, S1, !SP2

NOP; NOP; NOP; ST_G AD1, SP1
NOP; NOP; NOP; ST_G AD1, SP1, SP2
NOP; NOP; NOP; ST_G AD1, SP1, !SP2

// CHECK: nop;    nop;    nop;    st_g AD1, SP1, SP0
// CHECK: nop;    nop;    nop;    st_g AD1, SP1, SP2
// CHECK: nop;    nop;    nop;    st_g AD1, SP1, !SP2
