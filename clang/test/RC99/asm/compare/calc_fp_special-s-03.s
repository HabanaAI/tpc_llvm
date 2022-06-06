// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu greco %s -o %t.o
// RUN: %disasm --mcpu=greco %t.o | FileCheck %s

NOP; CALC_FP_SPECIAL.F16 FUNC=RECIP S1, S2, S0; NOP; NOP;

// CHECK: nop; calc_fp_special.f16 func=recip S1, S2, S0; nop; nop
