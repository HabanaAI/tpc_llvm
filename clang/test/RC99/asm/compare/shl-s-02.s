// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s


NOP; SHL.BF16 S5, S1, S2; NOP; NOP
NOP; SHL.BF16 S5, S1, S2, SP3; NOP; NOP
NOP; SHL.BF16 S5, S1, S2, !SP3; NOP; NOP
NOP; SHL.BF16 S5, S1, 0x3f4c; NOP; NOP
NOP; SHL.BF16 S5, S1, 0x3f4c, SP3; NOP; NOP
NOP; SHL.BF16 S5, S1, 0x3f4c, !SP3; NOP; NOP

// CHECK: nop;    shl.bf16  S5, S1, S2;      nop;    nop
// CHECK: nop;    shl.bf16  S5, S1, S2, SP3;      nop;    nop
// CHECK: nop;    shl.bf16  S5, S1, S2, !SP3;     nop;    nop
// CHECK: nop;    shl.bf16  S5, S1, 0x3f4c;  nop;    nop
// CHECK: nop;    shl.bf16  S5, S1, 0x3f4c, SP3;  nop;    nop
// CHECK: nop;    shl.bf16  S5, S1, 0x3f4c, !SP3;         nop;    nop


NOP; SHL.I32 b00101 I1, I2, I3; NOP; NOP
NOP; SHL.I32 b00101 I1, I2, I3, SP4; NOP; NOP
NOP; SHL.I32 b00101 I1, I2, I3, !SP4; NOP; NOP

// CHECK: nop;    shl.i32  b00101 I1, I2, I3;        nop;    nop
// CHECK: nop;    shl.i32  b00101 I1, I2, I3, SP4;        nop;    nop
// CHECK: nop;    shl.i32  b00101 I1, I2, I3, !SP4;       nop;    nop


NOP; SHL.I32 b00101 I1, I1, S3; NOP; NOP
NOP; SHL.I32 b00101 I1, I1, S3, SP4; NOP; NOP
NOP; SHL.I32 b00101 I1, I1, S3, !SP4; NOP; NOP

// CHECK: nop;    shl.i32  b00101 I1, I1, S3;        nop;    nop
// CHECK: nop;    shl.i32  b00101 I1, I1, S3, SP4;        nop;    nop
// CHECK: nop;    shl.i32  b00101 I1, I1, S3, !SP4;       nop;    nop
