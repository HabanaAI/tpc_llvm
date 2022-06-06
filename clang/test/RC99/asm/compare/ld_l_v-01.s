// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

ld_l_v V1, S2, S3
ld_l_v V1, S2, S3, SP4
ld_l_v V1, S2, S3, !SP4
ld_l_v V1, S2, S3, VP4
ld_l_v V1, S2, S3, !VP4

// CHECK: ld_l_v V1, S2, S3;         nop;    nop;    nop
// CHECK: ld_l_v V1, S2, S3, SP4;         nop;    nop;    nop
// CHECK: ld_l_v V1, S2, S3, !SP4;        nop;    nop;    nop
// CHECK: ld_l_v V1, S2, S3, VP4;         nop;    nop;    nop
// CHECK: ld_l_v V1, S2, S3, !VP4;        nop;    nop;    nop


ld_l_v V1, 100, S2
ld_l_v V1, 104, S2, SP5
ld_l_v V1, 32000, S2, !SP5
ld_l_v V1, 2000000, S2, VP6
ld_l_v V1, 2000000000, S2, !VP9

// CHECK: ld_l_v V1, 0x64, S2;       nop;    nop;    nop
// CHECK: ld_l_v V1, 0x68, S2, SP5;       nop;    nop;    nop
// CHECK: ld_l_v V1, 0x7d00, S2, !SP5;    nop;    nop;    nop
// CHECK: ld_l_v V1, 0x1e8480, S2, VP6;   nop;    nop;    nop
// CHECK: ld_l_v V1, 0x77359400, S2, !VP9;        nop;    nop;    nop


ld_l_v VP1, S2, S3
ld_l_v VP1, S2, S3, SP4
ld_l_v VP1, S2, S3, !SP4
ld_l_v VP1, S2, S3, VP4
ld_l_v VP1, S2, S3, !VP4

// CHECK: ld_l_v VP1, S2, S3;        nop;    nop;    nop
// CHECK: ld_l_v VP1, S2, S3, SP4;        nop;    nop;    nop
// CHECK: ld_l_v VP1, S2, S3, !SP4;       nop;    nop;    nop
// CHECK: ld_l_v VP1, S2, S3, VP4;        nop;    nop;    nop
// CHECK: ld_l_v VP1, S2, S3, !VP4;       nop;    nop;    nop


ld_l_v VP1, 123, S3
ld_l_v VP1, 123, S3, SP4
ld_l_v VP1, 123, S3, !SP4
ld_l_v VP1, 123, S3, VP4
ld_l_v VP1, 123, S3, !VP4

// CHECK: ld_l_v VP1, 0x7b, S3;      nop;    nop;    nop
// CHECK: ld_l_v VP1, 0x7b, S3, SP4;      nop;    nop;    nop
// CHECK: ld_l_v VP1, 0x7b, S3, !SP4;     nop;    nop;    nop
// CHECK: ld_l_v VP1, 0x7b, S3, VP4;      nop;    nop;    nop
// CHECK: ld_l_v VP1, 0x7b, S3, !VP4;     nop;    nop;    nop