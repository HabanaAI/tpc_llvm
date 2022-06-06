// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s


NOP; NOP; MUL.BF16 V6, V2, V1
NOP; NOP; MUL.BF16 V6, V2, V1, SP4
NOP; NOP; MUL.BF16 V6, V2, V1, !SP4
NOP; NOP; MUL.BF16 V6, V2, V1, VP4
NOP; NOP; MUL.BF16 V6, V2, V1, !VP4

// CHECK: nop;    nop;    mul.bf16  V6, V2, V1;      nop
// CHECK: nop;    nop;    mul.bf16  V6, V2, V1, SP4;      nop
// CHECK: nop;    nop;    mul.bf16  V6, V2, V1, !SP4;     nop
// CHECK: nop;    nop;    mul.bf16  V6, V2, V1, VP4;      nop
// CHECK: nop;    nop;    mul.bf16  V6, V2, V1, !VP4;     nop


NOP; NOP; MUL.BF16.ACC_FP32 D6, V2, V1
NOP; NOP; MUL.BF16.ACC_FP32 D6, V2, V1, SP4
NOP; NOP; MUL.BF16.ACC_FP32 D6, V2, V1, !SP4
NOP; NOP; MUL.BF16.ACC_FP32 D6, V2, V1, VP4
NOP; NOP; MUL.BF16.ACC_FP32 D6, V2, V1, !VP4

// CHECK: nop;    nop;    mul.bf16  acc_fp32 D6, V2, V1;     nop
// CHECK: nop;    nop;    mul.bf16  acc_fp32 D6, V2, V1, SP4;     nop
// CHECK: nop;    nop;    mul.bf16  acc_fp32 D6, V2, V1, !SP4;    nop
// CHECK: nop;    nop;    mul.bf16  acc_fp32 D6, V2, V1, VP4;     nop
// CHECK: nop;    nop;    mul.bf16  acc_fp32 D6, V2, V1, !VP4;    nop

NOP; NOP; MUL.BF16.ACC_FP32 V6, V2, V1
NOP; NOP; MUL.BF16.ACC_FP32 V6, V2, V1, SP4
NOP; NOP; MUL.BF16.ACC_FP32 V6, V2, V1, !SP4
NOP; NOP; MUL.BF16.ACC_FP32 V6, V2, V1, VP4
NOP; NOP; MUL.BF16.ACC_FP32 V6, V2, V1, !VP4

// CHECK: nop;    nop;    mul.bf16  acc_fp32 D6, V2, V1;     nop
// CHECK: nop;    nop;    mul.bf16  acc_fp32 D6, V2, V1, SP4;     nop
// CHECK: nop;    nop;    mul.bf16  acc_fp32 D6, V2, V1, !SP4;    nop
// CHECK: nop;    nop;    mul.bf16  acc_fp32 D6, V2, V1, VP4;     nop
// CHECK: nop;    nop;    mul.bf16  acc_fp32 D6, V2, V1, !VP4;    nop
