// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm -mcpu gaudi %t.o | FileCheck %s


NOP; MAC.BF16 S5, S1, S2
NOP; MAC.BF16 S5, S1, S2, SP3
NOP; MAC.BF16 S5, S1, S2, !SP3

// CHECK: nop;    mac.bf16  S5, S1, S2, SP0;      nop;    nop
// CHECK: nop;    mac.bf16  S5, S1, S2, SP3;      nop;    nop
// CHECK: nop;    mac.bf16  S5, S1, S2, !SP3;     nop;    nop


NOP; MAC.BF16.NEG S5, S1, S2
NOP; MAC.BF16.NEG S5, S1, S2, SP3
NOP; MAC.BF16.NEG S5, S1, S2, !SP3

// CHECK: nop;    mac.bf16  neg S5, S1, S2, SP0;  nop;    nop
// CHECK: nop;    mac.bf16  neg S5, S1, S2, SP3;  nop;    nop
// CHECK: nop;    mac.bf16  neg S5, S1, S2, !SP3;         nop;    nop


NOP; MAC.BF16.ACC_FP32 S5, S1, S2
NOP; MAC.BF16.ACC_FP32 S5, S1, S2, SP3
NOP; MAC.BF16.ACC_FP32 S5, S1, S2, !SP3

// CHECK: nop;    mac.bf16  acc_fp32 S5, S1, S2, SP0;     nop;    nop
// CHECK: nop;    mac.bf16  acc_fp32 S5, S1, S2, SP3;     nop;    nop
// CHECK: nop;    mac.bf16  acc_fp32 S5, S1, S2, !SP3;    nop;    nop


NOP; MAC.BF16.NEG.ACC_FP32 S5, S1, S2
NOP; MAC.BF16.NEG.ACC_FP32 S5, S1, S2, SP3
NOP; MAC.BF16.NEG.ACC_FP32 S5, S1, S2, !SP3

// CHECK: nop;    mac.bf16  neg acc_fp32 S5, S1, S2, SP0;         nop;    nop
// CHECK: nop;    mac.bf16  neg acc_fp32 S5, S1, S2, SP3;         nop;    nop
// CHECK: nop;    mac.bf16  neg acc_fp32 S5, S1, S2, !SP3;        nop;    nop
