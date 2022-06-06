// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu doron1 %s -o %t.o
// RUN: %disasm --mcpu=doron1 %t.o | FileCheck %s

CALC_FP_SPECIAL.F32 FUNC=RECIP V1, V2, V2; NOP; NOP; NOP;
CALC_FP_SPECIAL.F16 FUNC=RECIP V1, V2, V2; NOP; NOP; NOP;

// CHECK: calc_fp_special.f32 func=recip V1, V2, V2;   nop;   nop;   nop
// CHECK: calc_fp_special.f16 func=recip V1, V2, V2;   nop;   nop;   nop

CALC_FP_SPECIAL.BF16 FUNC=RECIP V1, V2, V2; NOP; NOP; NOP;
CALC_FP_SPECIAL.BF16 FUNC=RSQRT V1, V2, V2; NOP; NOP; NOP;
CALC_FP_SPECIAL.BF16 FUNC=SQRT V1, V2, V2; NOP; NOP; NOP;
CALC_FP_SPECIAL.BF16 FUNC=LOG V1, V2, V2; NOP; NOP; NOP;
CALC_FP_SPECIAL.BF16 FUNC=EXP V1, V2, V2;  NOP; NOP; NOP;
CALC_FP_SPECIAL.BF16 FUNC=TANH V1, V2, V2; NOP; NOP; NOP;

// CHECK: calc_fp_special.bf16 func=recip V1, V2, V2;   nop;   nop;   nop
// CHECK: calc_fp_special.bf16 func=rsqrt V1, V2, V2;   nop;   nop;   nop
// CHECK: calc_fp_special.bf16 func=sqrt V1, V2, V2;   nop;   nop;   nop
// CHECK: calc_fp_special.bf16 func=log V1, V2, V2;   nop;   nop;   nop
// CHECK: calc_fp_special.bf16 func=exp V1, V2, V2;   nop;   nop;   nop
// CHECK: calc_fp_special.bf16 func=tanh V1, V2, V2;   nop;   nop;   nop

CALC_FP_SPECIAL.BF16 FUNC=RECIP V1, V2, V2, SP1; NOP; NOP; NOP;
CALC_FP_SPECIAL.BF16 FUNC=RECIP V1, V2, V2, !SP1; NOP; NOP; NOP;
CALC_FP_SPECIAL.BF16 FUNC=RECIP V1, V2, V2, VP1; NOP; NOP; NOP;
CALC_FP_SPECIAL.BF16 FUNC=RECIP V1, V2, V2, !VP1; NOP; NOP; NOP;;

// CHECK: calc_fp_special.bf16 func=recip V1, V2, V2, SP1;   nop;   nop;   nop
// CHECK: calc_fp_special.bf16 func=recip V1, V2, V2, !SP1;   nop;   nop;   nop
// CHECK: calc_fp_special.bf16 func=recip V1, V2, V2, VP1;   nop;   nop;   nop
// CHECK: calc_fp_special.bf16 func=recip V1, V2, V2, !VP1;   nop;   nop;   nop
