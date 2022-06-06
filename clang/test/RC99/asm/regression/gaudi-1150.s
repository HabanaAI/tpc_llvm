// RUN: %tpc_clang -c -x assembler -march=gaudi %s -o %ttest.o
// RUN: %disasm --mcpu=gaudi %ttest.o | FileCheck %s

NOP; NOP; MAC.BF16.ACC_FP32 D6, V1, V2, VP3
// CHECK: nop; nop; mac.bf16 acc_fp32 D6, V1, V2, VP3; nop

nop; nop; mac.bf16 acc_fp32 D6, V1, V2, VP3; nop
// CHECK: nop; nop; mac.bf16 acc_fp32 D6, V1, V2, VP3; nop
