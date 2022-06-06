// RUN: %tpc_clang -c -x assembler -march=goya2 %s -o %ttest.o
// RUN: %disasm --mcpu=goya2 --tpc-encoding-info %ttest.o | FileCheck %s

ld_g V5, AD1, SP2; nop; nop; nop
// CHECK: LOAD_SWITCHES=0
