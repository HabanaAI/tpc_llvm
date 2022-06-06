// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, SP5; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, !SP5; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, VP5; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, !VP5; NOP

// CHECK: nop;    nop;    form_fp_num.bf16  V1, V2, V3, V4;  nop
// CHECK: nop;    nop;    form_fp_num.bf16  V1, V2, V3, V4, SP5;  nop
// CHECK: nop;    nop;    form_fp_num.bf16  V1, V2, V3, V4, !SP5;         nop
// CHECK: nop;    nop;    form_fp_num.bf16  V1, V2, V3, V4, VP5;  nop
// CHECK: nop;    nop;    form_fp_num.bf16  V1, V2, V3, V4, !VP5;         nop


NOP; NOP; FORM_FP_NUM.BF16 V2, S1, V3, V4; NOP
NOP; NOP; FORM_FP_NUM.BF16 V2, S1, V3, V4, SP5; NOP
NOP; NOP; FORM_FP_NUM.BF16 V2, S1, V3, V4, !SP5; NOP
NOP; NOP; FORM_FP_NUM.BF16 V2, S1, V3, V4, VP5; NOP
NOP; NOP; FORM_FP_NUM.BF16 V2, S1, V3, V4, !VP5; NOP

// CHECK: nop;    nop;    form_fp_num.bf16  V2, S1, V3, V4;  nop
// CHECK: nop;    nop;    form_fp_num.bf16  V2, S1, V3, V4, SP5;  nop
// CHECK: nop;    nop;    form_fp_num.bf16  V2, S1, V3, V4, !SP5;         nop
// CHECK: nop;    nop;    form_fp_num.bf16  V2, S1, V3, V4, VP5;  nop
// CHECK: nop;    nop;    form_fp_num.bf16  V2, S1, V3, V4, !VP5;         nop


NOP; NOP; FORM_FP_NUM.BF16 V2, bf16_1.0, V3, V4; NOP
NOP; NOP; FORM_FP_NUM.BF16 V2, bf16_1.0, V3, V4, SP5; NOP
NOP; NOP; FORM_FP_NUM.BF16 V2, bf16_1.0, V3, V4, !SP5; NOP
NOP; NOP; FORM_FP_NUM.BF16 V2, bf16_1.0, V3, V4, VP5; NOP
NOP; NOP; FORM_FP_NUM.BF16 V2, bf16_1.0, V3, V4, !VP5; NOP

// CHECK: nop;    nop;    form_fp_num.bf16  V2, 0x3f80, V3, V4;  nop
// CHECK: nop;    nop;    form_fp_num.bf16  V2, 0x3f80, V3, V4, SP5;  nop
// CHECK: nop;    nop;    form_fp_num.bf16  V2, 0x3f80, V3, V4, !SP5;         nop
// CHECK: nop;    nop;    form_fp_num.bf16  V2, 0x3f80, V3, V4, VP5;  nop
// CHECK: nop;    nop;    form_fp_num.bf16  V2, 0x3f80, V3, V4, !VP5;         nop


NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, EXP_ADD_BIAS; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, EXP_ADD_BIAS, SP5; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, EXP_ADD_BIAS, !SP5; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, EXP_ADD_BIAS, VP5; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, EXP_ADD_BIAS, !VP5; NOP

// CHECK: nop;    nop;    form_fp_num.bf16  exp_add_bias V1, V2, V3, V4;     nop
// CHECK: nop;    nop;    form_fp_num.bf16  exp_add_bias V1, V2, V3, V4, SP5;     nop
// CHECK: nop;    nop;    form_fp_num.bf16  exp_add_bias V1, V2, V3, V4, !SP5;    nop
// CHECK: nop;    nop;    form_fp_num.bf16  exp_add_bias V1, V2, V3, V4, VP5;     nop
// CHECK: nop;    nop;    form_fp_num.bf16  exp_add_bias V1, V2, V3, V4, !VP5;    nop


NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, EXP_ADD_BIAS, FORCE_SIGN0, EXP_IS_NUM; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, EXP_ADD_BIAS, FORCE_SIGN1, EXP_IS_NUM; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, EXP_ADD_BIAS, EXP_IS_NUM, SIGN_LSB; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, FORCE_SIGN0, EXP_IS_NUM; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, FORCE_SIGN1, EXP_IS_NUM; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, EXP_IS_NUM, SIGN_LSB; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, EXP_ADD_BIAS, FORCE_SIGN0; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, EXP_ADD_BIAS, FORCE_SIGN1; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, EXP_ADD_BIAS, SIGN_LSB; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, FORCE_SIGN0; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, FORCE_SIGN1; NOP
NOP; NOP; FORM_FP_NUM.BF16 V1, V2, V3, V4, SIGN_LSB; NOP

// CHECK: nop;    nop;    form_fp_num.bf16  exp_add_bias force_sign0 exp_is_num V1, V2, V3, V4;      nop
// CHECK: nop;    nop;    form_fp_num.bf16  exp_add_bias force_sign1 exp_is_num V1, V2, V3, V4;      nop
// CHECK: nop;    nop;    form_fp_num.bf16  exp_add_bias exp_is_num sign_lsb V1, V2, V3, V4;         nop
// CHECK: nop;    nop;    form_fp_num.bf16  force_sign0 exp_is_num V1, V2, V3, V4;   nop
// CHECK: nop;    nop;    form_fp_num.bf16  force_sign1 exp_is_num V1, V2, V3, V4;   nop
// CHECK: nop;    nop;    form_fp_num.bf16  exp_is_num sign_lsb V1, V2, V3, V4;      nop
// CHECK: nop;    nop;    form_fp_num.bf16  exp_add_bias force_sign0 V1, V2, V3, V4;         nop
// CHECK: nop;    nop;    form_fp_num.bf16  exp_add_bias force_sign1 V1, V2, V3, V4;         nop
// CHECK: nop;    nop;    form_fp_num.bf16  exp_add_bias sign_lsb V1, V2, V3, V4;    nop
// CHECK: nop;    nop;    form_fp_num.bf16  force_sign0 V1, V2, V3, V4;      nop
// CHECK: nop;    nop;    form_fp_num.bf16  force_sign1 V1, V2, V3, V4;      nop
// CHECK: nop;    nop;    form_fp_num.bf16  sign_lsb V1, V2, V3, V4;         nop
