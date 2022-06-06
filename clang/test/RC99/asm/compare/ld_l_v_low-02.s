// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s

LD_L_V_LOW V1, S3
LD_L_V_LOW V1, S3, SP4
LD_L_V_LOW V1, S3, !SP4
LD_L_V_LOW V1, S3, VP4
LD_L_V_LOW V1, S3, !VP4

// CHECK: ld_l_v_low V1, S3;         nop;    nop;    nop
// CHECK: ld_l_v_low V1, S3, SP4;         nop;    nop;    nop
// CHECK: ld_l_v_low V1, S3, !SP4;        nop;    nop;    nop
// CHECK: ld_l_v_low V1, S3, VP4;         nop;    nop;    nop
// CHECK: ld_l_v_low V1, S3, !VP4;        nop;    nop;    nop


LD_L_V_LOW V1, 100
LD_L_V_LOW V1, 104, SP5
LD_L_V_LOW V1, 32000, !SP5
LD_L_V_LOW V1, 2000000, VP6
LD_L_V_LOW V1, 2000000, !VP6

// CHECK: ld_l_v_low V1, 0x64;       nop;    nop;    nop
// CHECK: ld_l_v_low V1, 0x68, SP5;       nop;    nop;    nop
// CHECK: ld_l_v_low V1, 0x7d00, !SP5;    nop;    nop;    nop
// CHECK: ld_l_v_low V1, 0x1e8480, VP6;   nop;    nop;    nop
// CHECK: ld_l_v_low V1, 0x1e8480, !VP6;  nop;    nop;    nop


LD_L_V_LOW VP1, S2
LD_L_V_LOW VP1, S2, SP3
LD_L_V_LOW VP1, S2, !SP3
LD_L_V_LOW VP1, S2, VP3
LD_L_V_LOW VP1, S2, !VP3

// CHECK: ld_l_v_low VP1, S2;        nop;    nop;    nop
// CHECK: ld_l_v_low VP1, S2, SP3;        nop;    nop;    nop
// CHECK: ld_l_v_low VP1, S2, !SP3;       nop;    nop;    nop
// CHECK: ld_l_v_low VP1, S2, VP3;        nop;    nop;    nop
// CHECK: ld_l_v_low VP1, S2, !VP3;       nop;    nop;    nop


LD_L_V_LOW VP1, 123
LD_L_V_LOW VP1, 123, SP3
LD_L_V_LOW VP1, 123, !SP3
LD_L_V_LOW VP1, 123, VP3
LD_L_V_LOW VP1, 123, !VP3

// CHECK: ld_l_v_low VP1, 0x7b;      nop;    nop;    nop
// CHECK: ld_l_v_low VP1, 0x7b, SP3;      nop;    nop;    nop
// CHECK: ld_l_v_low VP1, 0x7b, !SP3;     nop;    nop;    nop
// CHECK: ld_l_v_low VP1, 0x7b, VP3;      nop;    nop;    nop
// CHECK: ld_l_v_low VP1, 0x7b, !VP3;     nop;    nop;    nop