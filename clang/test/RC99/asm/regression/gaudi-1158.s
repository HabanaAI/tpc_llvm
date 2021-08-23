// RUN: %tpc_clang -c -x assembler -march=gaudi %s -o %ttest.o
// RUN: %disasm -mcpu=gaudi %ttest.o | FileCheck %s

ld_g  V1, AD1, SP2
// CHECK: ld_g  V1, AD1, SP2
nop; nop; form_fp_num.bf16 force_sign0 exp_is_num V11, 0x7e, V19, V19, SP0; nop
// CHECK: form_fp_num.bf16 force_sign0 exp_is_num V11, 0x7e, V19, V19, SP0
ld_g  V1, AD1, SP2; nop; form_fp_num.bf16 force_sign0 exp_is_num V11, 0x7e, V19, V19, SP0; nop
// CHECK: ld_g  V1, AD1, SP2; nop; form_fp_num.bf16 force_sign0 exp_is_num V11, 0x7e, V19, V19, SP0; nop
