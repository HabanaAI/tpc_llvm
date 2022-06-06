// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s

NOP; NOP; CALC_FP_SPECIAL.F32 FUNC=RECIP V1, V2, V0; NOP;

// CHECK: nop; nop; calc_fp_special.f32 func=recip V1, V2, V0; nop

NOP; NOP; CALC_FP_SPECIAL.BF16 FUNC=RECIP V1, V2, V0; NOP;
NOP; NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT V1, V2, V0; NOP;
NOP; NOP; CALC_FP_SPECIAL.BF16 FUNC=SQRT V1, V2, V0; NOP;
NOP; NOP; CALC_FP_SPECIAL.BF16 FUNC=LOG V1, V2, V0; NOP;
NOP; NOP; CALC_FP_SPECIAL.BF16 FUNC=EXP V1, V2, V0; NOP;
NOP; NOP; CALC_FP_SPECIAL.BF16 FUNC=TANH V1, V2, V0; NOP;
NOP; NOP; CALC_FP_SPECIAL.BF16 FUNC=DIV V1, V2, V3; NOP;
NOP; NOP; CALC_FP_SPECIAL.BF16 FUNC=POW V1, V2, V3; NOP;

// CHECK: nop; nop; calc_fp_special.bf16 func=recip V1, V2, V0; nop
// CHECK: nop; nop; calc_fp_special.bf16 func=rsqrt V1, V2, V0; nop
// CHECK: nop; nop; calc_fp_special.bf16 func=sqrt V1, V2, V0; nop
// CHECK: nop; nop; calc_fp_special.bf16 func=log V1, V2, V0; nop
// CHECK: nop; nop; calc_fp_special.bf16 func=exp V1, V2, V0; nop
// CHECK: nop; nop; calc_fp_special.bf16 func=tanh V1, V2, V0; nop
// CHECK: nop; nop; calc_fp_special.bf16 func=div V1, V2, V3; nop
// CHECK: nop; nop; calc_fp_special.bf16 func=pow V1, V2, V3; nop

NOP; NOP; CALC_FP_SPECIAL.BF16 FUNC=RECIP V1, V2, V0, SP1; NOP;
NOP; NOP; CALC_FP_SPECIAL.BF16 FUNC=RECIP V1, V2, V0, !SP1; NOP;
NOP; NOP; CALC_FP_SPECIAL.BF16 FUNC=RECIP V1, V2, V0, VP1; NOP;
NOP; NOP; CALC_FP_SPECIAL.BF16 FUNC=RECIP V1, V2, V0, !VP1; NOP;

// CHECK: nop; nop; calc_fp_special.bf16 func=recip V1, V2, V0, SP1; nop
// CHECK: nop; nop; calc_fp_special.bf16 func=recip V1, V2, V0, !SP1; nop
// CHECK: nop; nop; calc_fp_special.bf16 func=recip V1, V2, V0, VP1; nop
// CHECK: nop; nop; calc_fp_special.bf16 func=recip V1, V2, V0, !VP1; nop

