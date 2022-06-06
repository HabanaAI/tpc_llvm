// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; SET_INDX I0, b00010, S35; NOP; NOP
NOP; SET_INDX I1, b00010, S34; NOP; NOP
NOP; SET_INDX I2, b00010, S33; NOP; NOP
NOP; SET_INDX I3, b00010, S32; NOP; NOP
NOP; SET_INDX I28, b00010, S31; NOP; NOP
NOP; SET_INDX I29, b00010, S30; NOP; NOP
NOP; SET_INDX I30, b00010, S29; NOP; NOP
NOP; SET_INDX I31, b00010, S28; NOP; NOP

// CHECK: nop; set_indx I0, b00010, S35; nop; nop
// CHECK: nop; set_indx I1, b00010, S34; nop; nop
// CHECK: nop; set_indx I2, b00010, S33; nop; nop
// CHECK: nop; set_indx I3, b00010, S32; nop; nop
// CHECK: nop; set_indx I28, b00010, S31; nop; nop
// CHECK: nop; set_indx I29, b00010, S30; nop; nop
// CHECK: nop; set_indx I30, b00010, S29; nop; nop
// CHECK: nop; set_indx I31, b00010, S28; nop; nop


NOP; SET_INDX I2, b00001, S3, SP0; NOP; NOP
NOP; SET_INDX I2, b00010, S3, SP1; NOP; NOP
NOP; SET_INDX I2, b00100, S3, SP2; NOP; NOP
NOP; SET_INDX I2, b01000, S3, SP3; NOP; NOP
NOP; SET_INDX I2, b10000, S3, SP4; NOP; NOP
NOP; SET_INDX I2, b11110, S3, SP5; NOP; NOP
NOP; SET_INDX I2, b11101, S3, SP6; NOP; NOP
NOP; SET_INDX I2, b11011, S3, SP7; NOP; NOP

// CHECK: nop; set_indx I2, b00001, S3; nop; nop
// CHECK: nop; set_indx I2, b00010, S3, SP1; nop; nop
// CHECK: nop; set_indx I2, b00100, S3, SP2; nop; nop
// CHECK: nop; set_indx I2, b01000, S3, SP3; nop; nop
// CHECK: nop; set_indx I2, b10000, S3, SP4; nop; nop
// CHECK: nop; set_indx I2, b11110, S3, SP5; nop; nop
// CHECK: nop; set_indx I2, b11101, S3, SP6; nop; nop
// CHECK: nop; set_indx I2, b11011, S3, SP7; nop; nop


NOP; SET_INDX I2, b00001, S3, !SP0; NOP; NOP
NOP; SET_INDX I2, b00010, S3, !SP1; NOP; NOP
NOP; SET_INDX I2, b00100, S3, !SP2; NOP; NOP
NOP; SET_INDX I2, b01000, S3, !SP3; NOP; NOP
NOP; SET_INDX I2, b10000, S3, !SP4; NOP; NOP
NOP; SET_INDX I2, b11110, S3, !SP5; NOP; NOP
NOP; SET_INDX I2, b11101, S3, !SP6; NOP; NOP
NOP; SET_INDX I2, b11011, S3, !SP7; NOP; NOP

// CHECK: nop; set_indx I2, b00001, S3, !SP0; nop; nop
// CHECK: nop; set_indx I2, b00010, S3, !SP1; nop; nop
// CHECK: nop; set_indx I2, b00100, S3, !SP2; nop; nop
// CHECK: nop; set_indx I2, b01000, S3, !SP3; nop; nop
// CHECK: nop; set_indx I2, b10000, S3, !SP4; nop; nop
// CHECK: nop; set_indx I2, b11110, S3, !SP5; nop; nop
// CHECK: nop; set_indx I2, b11101, S3, !SP6; nop; nop
// CHECK: nop; set_indx I2, b11011, S3, !SP7; nop; nop


NOP; SET_INDX I0, b00010, 123; NOP; NOP
NOP; SET_INDX I1, b00011, -123; NOP; NOP
NOP; SET_INDX I2, b00100, 0xffffffff; NOP; NOP
NOP; SET_INDX I3, b00110, 0x7fffffff; NOP; NOP
NOP; SET_INDX I4, b00111, 0x80000000; NOP; NOP

// CHECK: nop; set_indx I0, b00010, 0x7b; nop; nop
// CHECK: nop; set_indx I1, b00011, 0xffffff85; nop; nop
// CHECK: nop; set_indx I2, b00100, 0xffffffff; nop; nop
// CHECK: nop; set_indx I3, b00110, 0x7fffffff; nop; nop
// CHECK: nop; set_indx I4, b00111, 0x80000000; nop; nop


NOP; SET_INDX I0, b00010, 123, SP0; NOP; NOP
NOP; SET_INDX I1, b00011, -123, SP1; NOP; NOP
NOP; SET_INDX I2, b00100, 0xffffffff, SP2; NOP; NOP
NOP; SET_INDX I3, b00101, 0x7fffffff, SP3; NOP; NOP
NOP; SET_INDX I4, b00110, 0x80000000, SP4; NOP; NOP

// CHECK: nop; set_indx I0, b00010, 0x7b; nop; nop
// CHECK: nop; set_indx I1, b00011, 0xffffff85, SP1; nop; nop
// CHECK: nop; set_indx I2, b00100, 0xffffffff, SP2; nop; nop
// CHECK: nop; set_indx I3, b00101, 0x7fffffff, SP3; nop; nop
// CHECK: nop; set_indx I4, b00110, 0x80000000, SP4; nop; nop


NOP; SET_INDX I0, b00010, 123, !SP0; NOP; NOP
NOP; SET_INDX I1, b00011, -123, !SP1; NOP; NOP
NOP; SET_INDX I2, b00100, 0xffffffff, !SP2; NOP; NOP
NOP; SET_INDX I3, b00101, 0x7fffffff, !SP3; NOP; NOP
NOP; SET_INDX I4, b00110, 0x80000000, !SP4; NOP; NOP

// CHECK: nop; set_indx I0, b00010, 0x7b, !SP0; nop; nop
// CHECK: nop; set_indx I1, b00011, 0xffffff85, !SP1; nop; nop
// CHECK: nop; set_indx I2, b00100, 0xffffffff, !SP2; nop; nop
// CHECK: nop; set_indx I3, b00101, 0x7fffffff, !SP3; nop; nop
// CHECK: nop; set_indx I4, b00110, 0x80000000, !SP4; nop; nop
