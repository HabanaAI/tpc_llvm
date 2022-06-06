// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, int x2a, _BFloat16 xs, int dest, _Bool pred, int vpreda) {
  bfloat128 __local *x0ptr = (bfloat128 __local *)x0a;
  bfloat128 __local *x1ptr = (bfloat128 __local *)x1a;
  float128 __local *x2ptr  = (float128 __local *)x2a;
  float128 __local *dptr  = (float128 __local *)dest;
  bool128 __local *vpptr  = (bool128 __local *)vpreda;
  float128 res = { 0 };
  bfloat128 x0 = *x0ptr;
  bfloat128 x1 = *x1ptr;
  float128 x2 = *x2ptr;
  bool128 vpred = *vpptr;

  // Vector + Vector

 res = v_bf16_madd_acc32_b(x0, x1, x2, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}

 res = v_bf16_madd_acc32_b(x0, x1, x2, SW_NEG, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}

 res = v_bf16_madd_acc32_b(x0, x1, x2, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: madd.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %SP{{[0-9]+}}

 res = v_bf16_madd_acc32_b(x0, x1, x2, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: madd.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, !%SP{{[0-9]+}}

 res = v_bf16_madd_acc32_vb(x0, x1, x2, 0, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_bf16_madd_acc32_vb(x0, x1, x2, SW_NEG, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_bf16_madd_acc32_vb(x0, x1, x2, 0, res, vpred, 1);
  *dptr++ = res;
  // CHECK: madd.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

 res = v_bf16_madd_acc32_b(x0, xs, x2, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}

 res = v_bf16_madd_acc32_b(x0, xs, x2, SW_NEG, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}

 res = v_bf16_madd_acc32_b(x0, xs, x2, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: madd.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, %SP{{[0-9]+}}

 res = v_bf16_madd_acc32_b(x0, xs, x2, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: madd.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, !%SP{{[0-9]+}}

 res = v_bf16_madd_acc32_vb(x0, xs, x2, 0, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_bf16_madd_acc32_vb(x0, xs, x2, SW_NEG, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_bf16_madd_acc32_vb(x0, xs, x2, 0, res, vpred, 1);
  *dptr++ = res;
  // CHECK: madd.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Immediate

 res = v_bf16_madd_acc32_b(x0, 1.5, x2, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %D{{[0-9]+}}

 res = v_bf16_madd_acc32_b(x0, 1.5, x2, SW_NEG, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %D{{[0-9]+}}

 res = v_bf16_madd_acc32_b(x0, 1.5, x2, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: madd.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %D{{[0-9]+}}, %SP{{[0-9]+}}

 res = v_bf16_madd_acc32_b(x0, 1.5, x2, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: madd.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %D{{[0-9]+}}, !%SP{{[0-9]+}}

 res = v_bf16_madd_acc32_vb(x0, 1.5, x2, 0, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %D{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_bf16_madd_acc32_vb(x0, 1.5, x2, SW_NEG, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.bf16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %D{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_bf16_madd_acc32_vb(x0, 1.5, x2, 0, res, vpred, 1);
  *dptr++ = res;
  // CHECK: madd.bf16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc0, %D{{[0-9]+}}, !%VP{{[0-9]+}}
}
