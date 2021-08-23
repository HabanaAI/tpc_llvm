// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu gaudi %t.o | FileCheck %s

NOP; NOP; MAC.BF16 V5, V1, V2
NOP; NOP; MAC.BF16 V5, V1, V2, SP3
NOP; NOP; MAC.BF16 V5, V1, V2, !SP3
NOP; NOP; MAC.BF16 V5, V1, V2, VP3
NOP; NOP; MAC.BF16 V5, V1, V2, !VP3
NOP; NOP; MAC.BF16 V5, V1, 0X3F800000
NOP; NOP; MAC.BF16 V5, V1, 0X3F800000, SP3
NOP; NOP; MAC.BF16 V5, V1, 0X3F800000, !SP3
NOP; NOP; MAC.BF16 V5, V1, 0X3F800000, VP3
NOP; NOP; MAC.BF16 V5, V1, 0X3F800000, !VP3

// CHECK: 	nop; 	nop; 	mac.bf16  V5, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  V5, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  V5, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  V5, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  V5, V1, V2, !VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  V5, V1, 0x3f800000, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  V5, V1, 0x3f800000, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  V5, V1, 0x3f800000, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  V5, V1, 0x3f800000, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  V5, V1, 0x3f800000, !VP3; 	nop


NOP; NOP; MAC.BF16.NEG V5, V1, V2
NOP; NOP; MAC.BF16.NEG V5, V1, V2, SP3
NOP; NOP; MAC.BF16.NEG V5, V1, V2, !SP3
NOP; NOP; MAC.BF16.NEG V5, V1, V2, VP3
NOP; NOP; MAC.BF16.NEG V5, V1, V2, !VP3
NOP; NOP; MAC.BF16.NEG V5, V1, 0X3F800000
NOP; NOP; MAC.BF16.NEG V5, V1, 0X3F800000, SP3
NOP; NOP; MAC.BF16.NEG V5, V1, 0X3F800000, !SP3
NOP; NOP; MAC.BF16.NEG V5, V1, 0X3F800000, VP3
NOP; NOP; MAC.BF16.NEG V5, V1, 0X3F800000, !VP3

// CHECK: 	nop; 	nop; 	mac.bf16  neg V5, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg V5, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg V5, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg V5, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg V5, V1, V2, !VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg V5, V1, 0x3f800000, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg V5, V1, 0x3f800000, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg V5, V1, 0x3f800000, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg V5, V1, 0x3f800000, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg V5, V1, 0x3f800000, !VP3; 	nop


NOP; NOP; MAC.BF16.ACC_FP32 D6, V1, V2
NOP; NOP; MAC.BF16.ACC_FP32 D6, V1, V2, SP3
NOP; NOP; MAC.BF16.ACC_FP32 D6, V1, V2, !SP3
NOP; NOP; MAC.BF16.ACC_FP32 D6, V1, V2, VP3
NOP; NOP; MAC.BF16.ACC_FP32 D6, V1, V2, !VP3
NOP; NOP; MAC.BF16.ACC_FP32 D6, V1, 0X3F800000
NOP; NOP; MAC.BF16.ACC_FP32 D6, V1, 0X3F800000, SP3
NOP; NOP; MAC.BF16.ACC_FP32 D6, V1, 0X3F800000, !SP3
NOP; NOP; MAC.BF16.ACC_FP32 D6, V1, 0X3F800000, VP3
NOP; NOP; MAC.BF16.ACC_FP32 D6, V1, 0X3F800000, !VP3

// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, V2, !VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, 0x3f800000, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, 0x3f800000, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, 0x3f800000, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, 0x3f800000, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, 0x3f800000, !VP3; 	nop


NOP; NOP; MAC.BF16.NEG.ACC_FP32 D6, V1, V2
NOP; NOP; MAC.BF16.NEG.ACC_FP32 D6, V1, V2, SP3
NOP; NOP; MAC.BF16.NEG.ACC_FP32 D6, V1, V2, !SP3
NOP; NOP; MAC.BF16.NEG.ACC_FP32 D6, V1, V2, VP3
NOP; NOP; MAC.BF16.NEG.ACC_FP32 D6, V1, V2, !VP3
NOP; NOP; MAC.BF16.NEG.ACC_FP32 D6, V1, 0X3F800000
NOP; NOP; MAC.BF16.NEG.ACC_FP32 D6, V1, 0X3F800000, SP3
NOP; NOP; MAC.BF16.NEG.ACC_FP32 D6, V1, 0X3F800000, !SP3
NOP; NOP; MAC.BF16.NEG.ACC_FP32 D6, V1, 0X3F800000, VP3
NOP; NOP; MAC.BF16.NEG.ACC_FP32 D6, V1, 0X3F800000, !VP3

// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, V2, !VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, 0x3f800000, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, 0x3f800000, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, 0x3f800000, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, 0x3f800000, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, 0x3f800000, !VP3; 	nop


NOP; NOP; MAC.BF16.ACC_FP32 V6, V1, V2
NOP; NOP; MAC.BF16.ACC_FP32 V6, V1, V2, SP3
NOP; NOP; MAC.BF16.ACC_FP32 V6, V1, V2, !SP3
NOP; NOP; MAC.BF16.ACC_FP32 V6, V1, V2, VP3
NOP; NOP; MAC.BF16.ACC_FP32 V6, V1, V2, !VP3
NOP; NOP; MAC.BF16.ACC_FP32 V6, V1, 0X3F800000
NOP; NOP; MAC.BF16.ACC_FP32 V6, V1, 0X3F800000, SP3
NOP; NOP; MAC.BF16.ACC_FP32 V6, V1, 0X3F800000, !SP3
NOP; NOP; MAC.BF16.ACC_FP32 V6, V1, 0X3F800000, VP3
NOP; NOP; MAC.BF16.ACC_FP32 V6, V1, 0X3F800000, !VP3

// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, V2, !VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, 0x3f800000, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, 0x3f800000, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, 0x3f800000, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, 0x3f800000, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  acc_fp32 D6, V1, 0x3f800000, !VP3; 	nop


NOP; NOP; MAC.BF16.NEG.ACC_FP32 V6, V1, V2
NOP; NOP; MAC.BF16.NEG.ACC_FP32 V6, V1, V2, SP3
NOP; NOP; MAC.BF16.NEG.ACC_FP32 V6, V1, V2, !SP3
NOP; NOP; MAC.BF16.NEG.ACC_FP32 V6, V1, V2, VP3
NOP; NOP; MAC.BF16.NEG.ACC_FP32 V6, V1, V2, !VP3
NOP; NOP; MAC.BF16.NEG.ACC_FP32 V6, V1, 0X3F800000
NOP; NOP; MAC.BF16.NEG.ACC_FP32 V6, V1, 0X3F800000, SP3
NOP; NOP; MAC.BF16.NEG.ACC_FP32 V6, V1, 0X3F800000, !SP3
NOP; NOP; MAC.BF16.NEG.ACC_FP32 V6, V1, 0X3F800000, VP3
NOP; NOP; MAC.BF16.NEG.ACC_FP32 V6, V1, 0X3F800000, !VP3

// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, V2, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, V2, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, V2, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, V2, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, V2, !VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, 0x3f800000, SP0; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, 0x3f800000, SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, 0x3f800000, !SP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, 0x3f800000, VP3; 	nop
// CHECK: 	nop; 	nop; 	mac.bf16  neg acc_fp32 D6, V1, 0x3f800000, !VP3; 	nop


