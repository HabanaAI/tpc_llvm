// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu greco %s -o %t.o
// RUN: %disasm --mcpu=greco %t.o | FileCheck %s

NOP; NOP; CALC_FP_SPECIAL.F16 FUNC=RECIP V1, V2, V0; NOP;

// CHECK: nop; nop; calc_fp_special.f16 func=recip V1, V2, V0; nop
