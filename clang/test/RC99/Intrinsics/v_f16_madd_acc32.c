// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, int x2a, half xs, int dest, _Bool pred, int vpreda) {
  half128 __local *x0ptr = (half128 __local *)x0a;
  half128 __local *x1ptr = (half128 __local *)x1a;
  float128 __local *x2ptr  = (float128 __local *)x2a;
  float128 __local *dptr  = (float128 __local *)dest;
  bool128 __local *vpptr  = (bool128 __local *)vpreda;
  float128 res = { 0 };
  half128 x0 = *x0ptr;
  half128 x1 = *x1ptr;
  float128 x2 = *x2ptr;
  bool128 vpred = *vpptr;

  // Vector + Vector

 res = v_f16_madd_acc32_b(x0, x1, x2, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}

 res = v_f16_madd_acc32_b(x0, x1, x2, SW_NEG, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.f16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}

 res = v_f16_madd_acc32_b(x0, x1, x2, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: madd.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %SP{{[0-9]+}}

 res = v_f16_madd_acc32_b(x0, x1, x2, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: madd.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, !%SP{{[0-9]+}}

 res = v_f16_madd_acc32_vb(x0, x1, x2, 0, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_f16_madd_acc32_vb(x0, x1, x2, SW_NEG, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.f16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_f16_madd_acc32_vb(x0, x1, x2, 0, res, vpred, 1);
  *dptr++ = res;
  // CHECK: madd.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

 res = v_f16_madd_acc32_b(x0, xs, x2, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}

 res = v_f16_madd_acc32_b(x0, xs, x2, SW_NEG, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.f16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}

 res = v_f16_madd_acc32_b(x0, xs, x2, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: madd.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, %SP{{[0-9]+}}

 res = v_f16_madd_acc32_b(x0, xs, x2, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: madd.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, !%SP{{[0-9]+}}

 res = v_f16_madd_acc32_vb(x0, xs, x2, 0, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_f16_madd_acc32_vb(x0, xs, x2, SW_NEG, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.f16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_f16_madd_acc32_vb(x0, xs, x2, 0, res, vpred, 1);
  *dptr++ = res;
  // CHECK: madd.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Immediate

 res = v_f16_madd_acc32_b(x0, 1.5, x2, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %D{{[0-9]+}}

 res = v_f16_madd_acc32_b(x0, 1.5, x2, SW_NEG, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.f16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %D{{[0-9]+}}

 res = v_f16_madd_acc32_b(x0, 1.5, x2, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: madd.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %D{{[0-9]+}}, %SP{{[0-9]+}}

 res = v_f16_madd_acc32_b(x0, 1.5, x2, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: madd.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %D{{[0-9]+}}, !%SP{{[0-9]+}}

 res = v_f16_madd_acc32_vb(x0, 1.5, x2, 0, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %D{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_f16_madd_acc32_vb(x0, 1.5, x2, SW_NEG, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.f16 neg acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %D{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_f16_madd_acc32_vb(x0, 1.5, x2, 0, res, vpred, 1);
  *dptr++ = res;
  // CHECK: madd.f16 acc_fp32 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3e00, %D{{[0-9]+}}, !%VP{{[0-9]+}}
}
