// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; NOP; NOP; SET_INDX I0, b00010, S35
NOP; NOP; NOP; SET_INDX I1, b00010, S34
NOP; NOP; NOP; SET_INDX I2, b00010, S33
NOP; NOP; NOP; SET_INDX I3, b00010, S32
NOP; NOP; NOP; SET_INDX I28, b00010, S31
NOP; NOP; NOP; SET_INDX I29, b00010, S30
NOP; NOP; NOP; SET_INDX I30, b00010, S29
NOP; NOP; NOP; SET_INDX I31, b00010, S28

// CHECK: nop;    nop;    nop;    set_indx I0, b00010, S35
// CHECK: nop;    nop;    nop;    set_indx I1, b00010, S34
// CHECK: nop;    nop;    nop;    set_indx I2, b00010, S33
// CHECK: nop;    nop;    nop;    set_indx I3, b00010, S32
// CHECK: nop;    nop;    nop;    set_indx I28, b00010, S31
// CHECK: nop;    nop;    nop;    set_indx I29, b00010, S30
// CHECK: nop;    nop;    nop;    set_indx I30, b00010, S29
// CHECK: nop;    nop;    nop;    set_indx I31, b00010, S28


NOP; NOP; NOP; SET_INDX I2, b00001, S3, SP0
NOP; NOP; NOP; SET_INDX I2, b00010, S3, SP1
NOP; NOP; NOP; SET_INDX I2, b00100, S3, SP2
NOP; NOP; NOP; SET_INDX I2, b01000, S3, SP3
NOP; NOP; NOP; SET_INDX I2, b10000, S3, SP4
NOP; NOP; NOP; SET_INDX I2, b11110, S3, SP5
NOP; NOP; NOP; SET_INDX I2, b11101, S3, SP6
NOP; NOP; NOP; SET_INDX I2, b11011, S3, SP7

// CHECK: nop;    nop;    nop;    set_indx I2, b00001, S3
// CHECK: nop;    nop;    nop;    set_indx I2, b00010, S3, SP1
// CHECK: nop;    nop;    nop;    set_indx I2, b00100, S3, SP2
// CHECK: nop;    nop;    nop;    set_indx I2, b01000, S3, SP3
// CHECK: nop;    nop;    nop;    set_indx I2, b10000, S3, SP4
// CHECK: nop;    nop;    nop;    set_indx I2, b11110, S3, SP5
// CHECK: nop;    nop;    nop;    set_indx I2, b11101, S3, SP6
// CHECK: nop;    nop;    nop;    set_indx I2, b11011, S3, SP7


NOP; NOP; NOP; SET_INDX I2, b00001, S3, !SP0
NOP; NOP; NOP; SET_INDX I2, b00010, S3, !SP1
NOP; NOP; NOP; SET_INDX I2, b00100, S3, !SP2
NOP; NOP; NOP; SET_INDX I2, b01000, S3, !SP3
NOP; NOP; NOP; SET_INDX I2, b10000, S3, !SP4
NOP; NOP; NOP; SET_INDX I2, b11110, S3, !SP5
NOP; NOP; NOP; SET_INDX I2, b11101, S3, !SP6
NOP; NOP; NOP; SET_INDX I2, b11011, S3, !SP7

// CHECK: nop;    nop;    nop;    set_indx I2, b00001, S3, !SP0
// CHECK: nop;    nop;    nop;    set_indx I2, b00010, S3, !SP1
// CHECK: nop;    nop;    nop;    set_indx I2, b00100, S3, !SP2
// CHECK: nop;    nop;    nop;    set_indx I2, b01000, S3, !SP3
// CHECK: nop;    nop;    nop;    set_indx I2, b10000, S3, !SP4
// CHECK: nop;    nop;    nop;    set_indx I2, b11110, S3, !SP5
// CHECK: nop;    nop;    nop;    set_indx I2, b11101, S3, !SP6
// CHECK: nop;    nop;    nop;    set_indx I2, b11011, S3, !SP7


NOP; NOP; NOP; SET_INDX I0, b00010, 123
NOP; NOP; NOP; SET_INDX I1, b00011, -123
NOP; NOP; NOP; SET_INDX I2, b00100, 0xffffffff
NOP; NOP; NOP; SET_INDX I3, b00110, 0x7fffffff
NOP; NOP; NOP; SET_INDX I4, b00111, 0x80000000

// CHECK: nop;    nop;    nop;    set_indx I0, b00010, 0x7b
// CHECK: nop;    nop;    nop;    set_indx I1, b00011, 0xffffff85
// CHECK: nop;    nop;    nop;    set_indx I2, b00100, 0xffffffff
// CHECK: nop;    nop;    nop;    set_indx I3, b00110, 0x7fffffff
// CHECK: nop;    nop;    nop;    set_indx I4, b00111, 0x80000000


NOP; NOP; NOP; SET_INDX I0, b00010, 123, SP0
NOP; NOP; NOP; SET_INDX I1, b00011, -123, SP1
NOP; NOP; NOP; SET_INDX I2, b00100, 0xffffffff, SP2
NOP; NOP; NOP; SET_INDX I3, b00101, 0x7fffffff, SP3
NOP; NOP; NOP; SET_INDX I4, b00110, 0x80000000, SP4

// CHECK: nop;    nop;    nop;    set_indx I0, b00010, 0x7b
// CHECK: nop;    nop;    nop;    set_indx I1, b00011, 0xffffff85, SP1
// CHECK: nop;    nop;    nop;    set_indx I2, b00100, 0xffffffff, SP2
// CHECK: nop;    nop;    nop;    set_indx I3, b00101, 0x7fffffff, SP3
// CHECK: nop;    nop;    nop;    set_indx I4, b00110, 0x80000000, SP4


NOP; NOP; NOP; SET_INDX I0, b00010, 123, !SP0
NOP; NOP; NOP; SET_INDX I1, b00011, -123, !SP1
NOP; NOP; NOP; SET_INDX I2, b00100, 0xffffffff, !SP2
NOP; NOP; NOP; SET_INDX I3, b00101, 0x7fffffff, !SP3
NOP; NOP; NOP; SET_INDX I4, b00110, 0x80000000, !SP4

// CHECK: nop;    nop;    nop;    set_indx I0, b00010, 0x7b, !SP0
// CHECK: nop;    nop;    nop;    set_indx I1, b00011, 0xffffff85, !SP1
// CHECK: nop;    nop;    nop;    set_indx I2, b00100, 0xffffffff, !SP2
// CHECK: nop;    nop;    nop;    set_indx I3, b00101, 0x7fffffff, !SP3
// CHECK: nop;    nop;    nop;    set_indx I4, b00110, 0x80000000, !SP4
