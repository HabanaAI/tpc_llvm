// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s


NOP; MUL.BF16 S5, S1, S2
NOP; MUL.BF16 S5, S1, S2, SP3
NOP; MUL.BF16 S5, S1, S2, !SP3

// CHECK: nop;    mul.bf16  S5, S1, S2;      nop;    nop
// CHECK: nop;    mul.bf16  S5, S1, S2, SP3;      nop;    nop
// CHECK: nop;    mul.bf16  S5, S1, S2, !SP3;     nop;    nop

NOP; MUL.BF16.ACC_FP32 S5, S1, S2
NOP; MUL.BF16.ACC_FP32 S5, S1, S2, SP3
NOP; MUL.BF16.ACC_FP32 S5, S1, S2, !SP3

// CHECK: nop;    mul.bf16  acc_fp32 S5, S1, S2;     nop;    nop
// CHECK: nop;    mul.bf16  acc_fp32 S5, S1, S2, SP3;     nop;    nop
// CHECK: nop;    mul.bf16  acc_fp32 S5, S1, S2, !SP3;    nop;    nop
