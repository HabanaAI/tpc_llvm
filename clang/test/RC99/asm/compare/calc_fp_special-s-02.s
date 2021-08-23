// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s


NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S1, S34, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S2, S33, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S3, S32, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S4, S31, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S34, S15, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S33, S14, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S32, S13, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S31, S12, S0; NOP; NOP;

// CHECK: nop;    calc_fp_special.f32  func=rsqrt S1, S34, S0, SP0;   nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S2, S33, S0, SP0;   nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S3, S32, S0, SP0;   nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S4, S31, S0, SP0;   nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S34, S15, S0, SP0;  nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S33, S14, S0, SP0;  nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S32, S13, S0, SP0;  nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S31, S12, S0, SP0;  nop;    nop


NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S1, S34, S0, SP15; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S2, S33, S0, SP14; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S3, S32, S0, SP13; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S4, S31, S0, SP12; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S34, S15, S0, SP1; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S33, S14, S0, SP2; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S32, S13, S0, SP3; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S31, S12, S0, SP4; NOP; NOP;

// CHECK: nop;    calc_fp_special.f32  func=rsqrt S1, S34, S0, SP15;  nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S2, S33, S0, SP14;  nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S3, S32, S0, SP13;  nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S4, S31, S0, SP12;  nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S34, S15, S0, SP1;  nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S33, S14, S0, SP2;  nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S32, S13, S0, SP3;  nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S31, S12, S0, SP4;  nop;    nop


NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S1, S34, S0, !SP15; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S2, S33, S0, !SP14; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S3, S32, S0, !SP13; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S4, S31, S0, !SP12; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S34, S15, S0, !SP1; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S33, S14, S0, !SP2; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S32, S13, S0, !SP3; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S31, S12, S0, !SP4; NOP; NOP;

// CHECK: nop;    calc_fp_special.f32  func=rsqrt S1, S34, S0, !SP15;         nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S2, S33, S0, !SP14;         nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S3, S32, S0, !SP13;         nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S4, S31, S0, !SP12;         nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S34, S15, S0, !SP1;         nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S33, S14, S0, !SP2;         nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S32, S13, S0, !SP3;         nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S31, S12, S0, !SP4;         nop;    nop


NOP; CALC_FP_SPECIAL.F32 FUNC=RECIP S1, S34, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=RSQRT S2, S33, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=SQRT S3, S32, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=LOG S4, S31, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=EXP S34, S15, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=TANH S33, S14, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=DIV S32, S13, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.F32 FUNC=POW S31, S12, S0; NOP; NOP;

// CHECK: nop;    calc_fp_special.f32  func=recip S1, S34, S0, SP0;      nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=rsqrt S2, S33, S0, SP0;   nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=sqrt S3, S32, S0, SP0;    nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=log S4, S31, S0, SP0;     nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=exp S34, S15, S0, SP0;    nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=tanh S33, S14, S0, SP0;   nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=div S32, S13, S0, SP0;    nop;    nop
// CHECK: nop;    calc_fp_special.f32  func=pow S31, S12, S0, SP0;    nop;    nop



NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S1, S34, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S2, S33, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S3, S32, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S4, S31, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S34, S15, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S33, S14, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S32, S13, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S31, S12, S0; NOP; NOP;

// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S1, S34, S0, SP0;  nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S2, S33, S0, SP0;  nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S3, S32, S0, SP0;  nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S4, S31, S0, SP0;  nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S34, S15, S0, SP0;         nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S33, S14, S0, SP0;         nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S32, S13, S0, SP0;         nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S31, S12, S0, SP0;         nop;    nop


NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S1, S34, S0, SP15; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S2, S33, S0, SP14; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S3, S32, S0, SP13; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S4, S31, S0, SP12; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S34, S15, S0, SP1; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S33, S14, S0, SP2; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S32, S13, S0, SP3; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S31, S12, S0, SP4; NOP; NOP;

// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S1, S34, S0, SP15;         nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S2, S33, S0, SP14;         nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S3, S32, S0, SP13;         nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S4, S31, S0, SP12;         nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S34, S15, S0, SP1;         nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S33, S14, S0, SP2;         nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S32, S13, S0, SP3;         nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S31, S12, S0, SP4;         nop;    nop


NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S1, S34, S0, !SP15; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S2, S33, S0, !SP14; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S3, S32, S0, !SP13; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S4, S31, S0, !SP12; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S34, S15, S0, !SP1; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S33, S14, S0, !SP2; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S32, S13, S0, !SP3; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S31, S12, S0, !SP4; NOP; NOP;

// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S1, S34, S0, !SP15;        nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S2, S33, S0, !SP14;        nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S3, S32, S0, !SP13;        nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S4, S31, S0, !SP12;        nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S34, S15, S0, !SP1;        nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S33, S14, S0, !SP2;        nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S32, S13, S0, !SP3;        nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S31, S12, S0, !SP4;        nop;    nop


NOP; CALC_FP_SPECIAL.BF16 FUNC=RECIP S1, S34, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=RSQRT S2, S33, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=SQRT S3, S32, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=LOG S4, S31, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=EXP S34, S15, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=TANH S33, S14, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=DIV S32, S13, S0; NOP; NOP;
NOP; CALC_FP_SPECIAL.BF16 FUNC=POW S31, S12, S0; NOP; NOP;

// CHECK: nop;    calc_fp_special.bf16  func=recip S1, S34, S0, SP0;     nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=rsqrt S2, S33, S0, SP0;  nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=sqrt S3, S32, S0, SP0;   nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=log S4, S31, S0, SP0;    nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=exp S34, S15, S0, SP0;   nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=tanh S33, S14, S0, SP0;  nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=div S32, S13, S0, SP0;   nop;    nop
// CHECK: nop;    calc_fp_special.bf16  func=pow S31, S12, S0, SP0;   nop;    nop
