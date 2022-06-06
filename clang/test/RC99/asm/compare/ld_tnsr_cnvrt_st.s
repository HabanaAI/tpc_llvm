// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu doron1 %s -o %t.o
// RUN: %disasm --mcpu doron1 %t.o | FileCheck %s

NOP; NOP; NOP; LD_TNSR_CNVRT V4, 0, I3
NOP; NOP; NOP; LD_TNSR_CNVRT V4, 0, I3, SP1
NOP; NOP; NOP; LD_TNSR_CNVRT V4, 0, I3, !SP1
NOP; NOP; NOP; LD_TNSR_CNVRT V4, 0, I3, VP1
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 low inc_dim0 V4, 0x0, I3
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 low inc_dim0 V4, 0x0, I3, SP1
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 low inc_dim0 V4, 0x0, I3, !SP1
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 low inc_dim0 V4, 0x0, I3, VP1

NOP; NOP; NOP; LD_TNSR_CNVRT V4, 1, I3
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 low inc_dim0 V4, 0x1, I3

NOP; NOP; NOP; LD_TNSR_CNVRT FP32_TO_BF16 V4, 0, I3
NOP; NOP; NOP; LD_TNSR_CNVRT FP32_TO_FP16 V4, 0, I3
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 low inc_dim0 V4, 0x0, I3
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_fp16 low inc_dim0 V4, 0x0, I3

NOP; NOP; NOP; LD_TNSR_CNVRT CLIP_FP V4, 0, I3
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 low inc_dim0 clip_fp V4, 0x0, I3

NOP; NOP; NOP; LD_TNSR_CNVRT LOW V4, 0, I3
NOP; NOP; NOP; LD_TNSR_CNVRT HIGH V4, 0, I3
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 low inc_dim0 V4, 0x0, I3
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 high inc_dim0 V4, 0x0, I3

NOP; NOP; NOP; LD_TNSR_CNVRT L0CS V4, 0, I3
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 low l0cs inc_dim0 V4, 0x0, I3

NOP; NOP; NOP; LD_TNSR_CNVRT V4, LD_TNSR_ID_REG, I3
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 low inc_dim0 V4, LD_TNSR_ID_REG, I3

NOP; NOP; NOP; LD_TNSR_CNVRT.DIRECT V4, S1, S2
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 low direct inc_dim0 V4, S1, S2

NOP; NOP; NOP; LD_TNSR_CNVRT INC_DIM2 V4, 0, I3
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 low inc_dim2 V4, 0x0, I3

NOP; NOP; NOP; LD_TNSR_CNVRT.PARTIAL V4, 0, I3, LD_PARTIAL_REG
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 low inc_dim0 V4, 0x0, I3, LD_PARTIAL_REG

NOP; NOP; NOP; LD_TNSR_CNVRT.PARTIAL V4, LD_TNSR_ID_REG, I3, LD_PARTIAL_REG
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 low inc_dim0 V4, LD_TNSR_ID_REG, I3, LD_PARTIAL_REG

NOP; NOP; NOP; LD_TNSR_CNVRT.PARTIAL.DIRECT V4, S1, S2, LD_PARTIAL_REG
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_bf16 low direct inc_dim0 V4, S1, S2, LD_PARTIAL_REG

NOP; NOP; NOP; LD_TNSR_CNVRT.PARTIAL.L0CS.CLIP_FP V4, LD_TNSR_ID_REG, I3, LD_PARTIAL_REG, HIGH, FP32_TO_FP16, INC_DIM3, VP2
// CHECK: nop;   nop;   nop;   ld_tnsr_cnvrt fp32_to_fp16 high l0cs inc_dim3 clip_fp V4, LD_TNSR_ID_REG, I3, LD_PARTIAL_REG, VP2
