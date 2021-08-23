// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s


NOP; NOP; NOP; ST_TNSR_LOW 0, I31, V0
NOP; NOP; NOP; ST_TNSR_LOW 1, I30, V1
NOP; NOP; NOP; ST_TNSR_LOW 2, I29, V2
NOP; NOP; NOP; ST_TNSR_LOW 3, I28, V3
NOP; NOP; NOP; ST_TNSR_LOW 4, I3, V39
NOP; NOP; NOP; ST_TNSR_LOW 5, I2, V38
NOP; NOP; NOP; ST_TNSR_LOW 6, I1, V37
NOP; NOP; NOP; ST_TNSR_LOW 7, I0, V36

// CHECK: nop;    nop;    nop;    st_tnsr_low  0x0, I31, V0, SP0
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x1, I30, V1, SP0
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x2, I29, V2, SP0
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x3, I28, V3, SP0
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x4, I3, V39, SP0
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x5, I2, V38, SP0
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x6, I1, V37, SP0
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x7, I0, V36, SP0


NOP; NOP; NOP; ST_TNSR_LOW 0, I31, V0, SP7
NOP; NOP; NOP; ST_TNSR_LOW 1, I30, V1, SP6
NOP; NOP; NOP; ST_TNSR_LOW 2, I29, V2, SP5
NOP; NOP; NOP; ST_TNSR_LOW 3, I28, V3, SP4
NOP; NOP; NOP; ST_TNSR_LOW 4, I3, V39, SP3
NOP; NOP; NOP; ST_TNSR_LOW 5, I2, V38, SP2
NOP; NOP; NOP; ST_TNSR_LOW 6, I1, V37, SP1
NOP; NOP; NOP; ST_TNSR_LOW 7, I0, V36, SP0

// CHECK: nop;    nop;    nop;    st_tnsr_low  0x0, I31, V0, SP7
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x1, I30, V1, SP6
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x2, I29, V2, SP5
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x3, I28, V3, SP4
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x4, I3, V39, SP3
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x5, I2, V38, SP2
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x6, I1, V37, SP1
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x7, I0, V36, SP0


NOP; NOP; NOP; ST_TNSR_LOW 0, I31, V0, !SP15
NOP; NOP; NOP; ST_TNSR_LOW 1, I30, V1, !SP14
NOP; NOP; NOP; ST_TNSR_LOW 2, I29, V2, !SP13
NOP; NOP; NOP; ST_TNSR_LOW 3, I28, V3, !SP12
NOP; NOP; NOP; ST_TNSR_LOW 4, I3, V39, !SP11
NOP; NOP; NOP; ST_TNSR_LOW 5, I2, V38, !SP10
NOP; NOP; NOP; ST_TNSR_LOW 6, I1, V37, !SP9
NOP; NOP; NOP; ST_TNSR_LOW 7, I0, V36, !SP8

// CHECK: nop;    nop;    nop;    st_tnsr_low  0x0, I31, V0, !SP15
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x1, I30, V1, !SP14
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x2, I29, V2, !SP13
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x3, I28, V3, !SP12
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x4, I3, V39, !SP11
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x5, I2, V38, !SP10
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x6, I1, V37, !SP9
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x7, I0, V36, !SP8


NOP; NOP; NOP; ST_TNSR_LOW 0, I31, VP0
NOP; NOP; NOP; ST_TNSR_LOW 1, I30, VP1
NOP; NOP; NOP; ST_TNSR_LOW 2, I29, VP2
NOP; NOP; NOP; ST_TNSR_LOW 3, I28, VP3
NOP; NOP; NOP; ST_TNSR_LOW 4, I3, VP12
NOP; NOP; NOP; ST_TNSR_LOW 5, I2, VP13
NOP; NOP; NOP; ST_TNSR_LOW 6, I1, VP14
NOP; NOP; NOP; ST_TNSR_LOW 7, I0, VP15

// CHECK: nop;    nop;    nop;    st_tnsr_low  0x0, I31, VP0, SP0
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x1, I30, VP1, SP0
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x2, I29, VP2, SP0
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x3, I28, VP3, SP0
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x4, I3, VP12, SP0
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x5, I2, VP13, SP0
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x6, I1, VP14, SP0
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x7, I0, VP15, SP0


NOP; NOP; NOP; ST_TNSR_LOW 0, I31, VP0, SP7
NOP; NOP; NOP; ST_TNSR_LOW 1, I30, VP1, SP6
NOP; NOP; NOP; ST_TNSR_LOW 2, I29, VP2, SP5
NOP; NOP; NOP; ST_TNSR_LOW 3, I28, VP3, SP4
NOP; NOP; NOP; ST_TNSR_LOW 4, I3, VP12, SP3
NOP; NOP; NOP; ST_TNSR_LOW 5, I2, VP13, SP2
NOP; NOP; NOP; ST_TNSR_LOW 6, I1, VP14, SP1
NOP; NOP; NOP; ST_TNSR_LOW 7, I0, VP15, SP0

// CHECK: nop;    nop;    nop;    st_tnsr_low  0x0, I31, VP0, SP7
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x1, I30, VP1, SP6
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x2, I29, VP2, SP5
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x3, I28, VP3, SP4
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x4, I3, VP12, SP3
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x5, I2, VP13, SP2
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x6, I1, VP14, SP1
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x7, I0, VP15, SP0


NOP; NOP; NOP; ST_TNSR_LOW 0, I31, VP0, !SP15
NOP; NOP; NOP; ST_TNSR_LOW 1, I30, VP1, !SP14
NOP; NOP; NOP; ST_TNSR_LOW 2, I29, VP2, !SP13
NOP; NOP; NOP; ST_TNSR_LOW 3, I28, VP3, !SP12
NOP; NOP; NOP; ST_TNSR_LOW 4, I3, VP12, !SP11
NOP; NOP; NOP; ST_TNSR_LOW 5, I2, VP13, !SP10
NOP; NOP; NOP; ST_TNSR_LOW 6, I1, VP14, !SP9
NOP; NOP; NOP; ST_TNSR_LOW 7, I0, VP15, !SP8

// CHECK: nop;    nop;    nop;    st_tnsr_low  0x0, I31, VP0, !SP15
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x1, I30, VP1, !SP14
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x2, I29, VP2, !SP13
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x3, I28, VP3, !SP12
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x4, I3, VP12, !SP11
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x5, I2, VP13, !SP10
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x6, I1, VP14, !SP9
// CHECK: nop;    nop;    nop;    st_tnsr_low  0x7, I0, VP15, !SP8
