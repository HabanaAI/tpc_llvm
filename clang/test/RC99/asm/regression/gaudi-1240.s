// RUN: %tpc_clang -c -x assembler -march=gaudi2 %s -o %ttest.o
// RUN: %disasm --mcpu=gaudi2 %ttest.o | FileCheck %s

nop; nop; nop; ld_tnsr  V10, 0x0, I19, SP0
// CHECK: nop; nop; nop; ld_tnsr  V10, 0x0, I19

