// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s


NOP; NOP; NOP; ST_G AD0, S31
NOP; NOP; NOP; ST_G AD1, S30
NOP; NOP; NOP; ST_G AD2, S29
NOP; NOP; NOP; ST_G AD3, S28
NOP; NOP; NOP; ST_G AD4, S27
NOP; NOP; NOP; ST_G AD5, S26
NOP; NOP; NOP; ST_G AD6, S25
NOP; NOP; NOP; ST_G AD7, S24

// CHECK: nop;    nop;    nop;    st_g AD0, S31, SP0
// CHECK: nop;    nop;    nop;    st_g AD1, S30, SP0
// CHECK: nop;    nop;    nop;    st_g AD2, S29, SP0
// CHECK: nop;    nop;    nop;    st_g AD3, S28, SP0
// CHECK: nop;    nop;    nop;    st_g AD4, S27, SP0
// CHECK: nop;    nop;    nop;    st_g AD5, S26, SP0
// CHECK: nop;    nop;    nop;    st_g AD6, S25, SP0
// CHECK: nop;    nop;    nop;    st_g AD7, S24, SP0


NOP; NOP; NOP; ST_G AD0, S16, SP0
NOP; NOP; NOP; ST_G AD1, S17, SP1
NOP; NOP; NOP; ST_G AD2, S18, SP2
NOP; NOP; NOP; ST_G AD3, S19, SP3
NOP; NOP; NOP; ST_G AD4, S20, SP12
NOP; NOP; NOP; ST_G AD5, S21, SP13
NOP; NOP; NOP; ST_G AD6, S22, SP14
NOP; NOP; NOP; ST_G AD7, S23, SP15

// CHECK: nop;    nop;    nop;    st_g AD0, S16, SP0
// CHECK: nop;    nop;    nop;    st_g AD1, S17, SP1
// CHECK: nop;    nop;    nop;    st_g AD2, S18, SP2
// CHECK: nop;    nop;    nop;    st_g AD3, S19, SP3
// CHECK: nop;    nop;    nop;    st_g AD4, S20, SP12
// CHECK: nop;    nop;    nop;    st_g AD5, S21, SP13
// CHECK: nop;    nop;    nop;    st_g AD6, S22, SP14
// CHECK: nop;    nop;    nop;    st_g AD7, S23, SP15


NOP; NOP; NOP; ST_G AD0, S16, SP0
NOP; NOP; NOP; ST_G AD1, S17, SP1
NOP; NOP; NOP; ST_G AD2, S18, SP2
NOP; NOP; NOP; ST_G AD3, S19, SP3
NOP; NOP; NOP; ST_G AD4, S20, SP12
NOP; NOP; NOP; ST_G AD5, S21, SP13
NOP; NOP; NOP; ST_G AD6, S22, SP14
NOP; NOP; NOP; ST_G AD7, S23, SP15

// CHECK: nop;    nop;    nop;    st_g AD0, S16, SP0
// CHECK: nop;    nop;    nop;    st_g AD1, S17, SP1
// CHECK: nop;    nop;    nop;    st_g AD2, S18, SP2
// CHECK: nop;    nop;    nop;    st_g AD3, S19, SP3
// CHECK: nop;    nop;    nop;    st_g AD4, S20, SP12
// CHECK: nop;    nop;    nop;    st_g AD5, S21, SP13
// CHECK: nop;    nop;    nop;    st_g AD6, S22, SP14
// CHECK: nop;    nop;    nop;    st_g AD7, S23, SP15
