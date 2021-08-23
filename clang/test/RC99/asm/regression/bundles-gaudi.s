// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm -mcpu gaudi %t.o | FileCheck %s


// GAUDI-898

LD_L S1, S2, MMIO; NOP; MAC.BF16.ACC_FP32 D6, V1, V2
// CHECK: ld_l  mmio S1, S2, SP0; nop; mac.bf16  acc_fp32 D6, V1, V2, SP0; nop
