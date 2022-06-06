// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s


NOP; ADD.I32 b00010 I2, i32_1, I2; ADD.F32 V9, V1, V5; ST_TNSR 2, I2, V8
// CHECK: nop; add.i32 b00010 I2, 0x1, I2; add.f32 V9, V1, V5; st_tnsr 0x2, I2, V8

NOP; add.i32 b00010 I2, 0xfffffffd, I2, SP0; NOP; st_tnsr  0x2, I2, V9, SP0
// CHECK: nop; add.i32 b00010 I2, 0xfffffffd, I2; nop; st_tnsr 0x2, I2, V9

NOP; ADD.I32 b00010 I2, i32_1, I2; ADD.I8.SAT V9, V5, V1; ST_TNSR 2, I2, V8
// CHECK: nop; add.i32 b00010 I2, 0x1, I2; add.i8 st V9, V5, V1; st_tnsr 0x2, I2, V8

NOP; CMP_LESS.I32 SP1, S35, S6; ADD.I8.SAT V8, V4, V0; NOP
// CHECK: nop; cmp_less.i32 SP1, S35, S6; add.i8 st V8, V4, V0; nop

NOP; ADD.I32 b00010 I2, i32_-3, I2; NOP; ST_TNSR 2, I2, V9
// CHECK: nop; add.i32 b00010 I2, 0xfffffffd, I2; nop; st_tnsr 0x2, I2, V9

SET_INDX I2, b00010, S3; ADD.I32 b00010 I4, i32_1, I4; ADD.F32 V0, V1, V0; SET_INDX I3, b00010, S3
// CHECK: set_indx I2, b00010, S3; add.i32 b00010 I4, 0x1, I4; add.f32 V0, V1, V0; set_indx I3, b00010, S3

NOP; ADD.I32 b00010 I4, i32_1, I4; ADD.F32 V1, V4, V2; ST_TNSR 2, I4, V3
// CHECK: nop; add.i32 b00010 I4, 0x1, I4; add.f32 V1, V4, V2; st_tnsr 0x2, I4, V3

NOP; ADD.I32 b00010 I3, i32_1, I3, !SP6; ADD.I16.SAT V6, V4, V5; ST_TNSR 2, I4, V6
// CHECK: nop; add.i32 b00010 I3, 0x1, I3, !SP6; add.i16 st V6, V4, V5; st_tnsr 0x2, I4, V6

NOP; ADD.I32 b00010 I4, i32_1, I4; ADD.I16.SAT V0, V4, V5; ST_TNSR 2, I4, V1
// CHECK: nop; add.i32  b00010 I4, 0x1, I4; add.i16 st V0, V4, V5; st_tnsr 0x2, I4, V1

NOP; ADD.I32 b00010 I4, i32_1, I4; ADD.I16.SAT V1, V1, V3; ST_TNSR 2, I4, V0
// CHECK: nop; add.i32 b00010 I4, 0x1, I4; add.i16 st V1, V1, V3; st_tnsr 0x2, I4, V0

NOP; NOP; ADD.I16.SAT V2, V0, V1; GEN_ADDR AD0, 0, I2
// CHECK: nop; nop; add.i16 st V2, V0, V1; gen_addr AD0, 0x0, I2

NOP; ADD.I32 b00010 I3, i32_1, I3; CONVERT_INT32 V9, V26, V1, LANE_SEL=2, RNE; ST_TNSR 2, I3, V8
// CHECK: nop; add.i32 b00010 I3, 0x1, I3; convert_int32 lane_sel=2 rhne to_8 V9, V26, V1; st_tnsr 0x2, I3, V8

SET_INDX I3, b00010, S7; NOP; CONVERT_INT32 V1, V15, V3, TO_16, LANE_SEL=0, RNE; NOP
// CHECK: set_indx I3, b00010, S7; nop; convert_int32 lane_sel=0 rhne to_16 V1, V15, V3; nop
