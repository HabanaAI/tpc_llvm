// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2  %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1  %s -o - | FileCheck %s

void main(int x0a, int x1a, int x2a, minihalf xs, int dest, _Bool pred, int vpreda) {
  minihalf256 __local *x0ptr = (minihalf256 __local *)x0a;
  minihalf256 __local *x1ptr = (minihalf256 __local *)x1a;
  float256 __local *x2ptr  = (float256 __local *)x2a;
  float256 __local *dptr  = (float256 __local *)dest;
  bool256 __local *vpptr  = (bool256 __local *)vpreda;
  float256 res = { 0 };
  minihalf256 x0 = *x0ptr;
  minihalf256 x1 = *x1ptr;
  float256 x2 = *x2ptr;
  bool256 vpred = *vpptr;

  // Vector + Vector

 res = v_h8_madd_acc32_b(x0, x1, x2, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}
#if 1

 res = v_h8_madd_acc32_b(x0, x1, x2, SW_NEG, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.f8_152 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}

 res = v_h8_madd_acc32_b(x0, x1, x2, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: madd.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}, %SP{{[0-9]+}}

 res = v_h8_madd_acc32_b(x0, x1, x2, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: madd.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}, !%SP{{[0-9]+}}

 res = v_h8_madd_acc32_vb(x0, x1, x2, 0, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_h8_madd_acc32_vb(x0, x1, x2, SW_NEG, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.f8_152 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_h8_madd_acc32_vb(x0, x1, x2, 0, res, vpred, 1);
  *dptr++ = res;
  // CHECK: madd.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

 res = v_h8_madd_acc32_b(x0, xs, x2, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S3, %A{{[0-9]+}}

 res = v_h8_madd_acc32_b(x0, xs, x2, SW_NEG, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.f8_152 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S3, %A{{[0-9]+}}

 res = v_h8_madd_acc32_b(x0, xs, x2, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: madd.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S3, %A{{[0-9]+}}, %SP{{[0-9]+}}

 res = v_h8_madd_acc32_b(x0, xs, x2, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: madd.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S3, %A{{[0-9]+}}, !%SP{{[0-9]+}}

 res = v_h8_madd_acc32_vb(x0, xs, x2, 0, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S3, %A{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_h8_madd_acc32_vb(x0, xs, x2, SW_NEG, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.f8_152 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S3, %A{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_h8_madd_acc32_vb(x0, xs, x2, 0, res, vpred, 1);
  *dptr++ = res;
  // CHECK: madd.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, %S3, %A{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Immediate
 res = v_h8_madd_acc32_b(x0, (minihalf)1.5, x2, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e, %A{{[0-9]+}}

 res = v_h8_madd_acc32_b(x0, (minihalf)1.5, x2, SW_NEG, res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.f8_152 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e, %A{{[0-9]+}}

 res = v_h8_madd_acc32_b(x0, (minihalf)1.5, x2, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: madd.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e, %A{{[0-9]+}}, %SP{{[0-9]+}}

 res = v_h8_madd_acc32_b(x0, (minihalf)1.5, x2, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: madd.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e, %A{{[0-9]+}}, !%SP{{[0-9]+}}

 res = v_h8_madd_acc32_vb(x0, (minihalf)1.5, x2, 0, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e, %A{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_h8_madd_acc32_vb(x0, (minihalf)1.5, x2, SW_NEG, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.f8_152 neg acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e, %A{{[0-9]+}}, %VP{{[0-9]+}}

 res = v_h8_madd_acc32_vb(x0, (minihalf)1.5, x2, 0, res, vpred, 1);
  *dptr++ = res;
  // CHECK: madd.f8_152 acc_fp32 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x3e, %A{{[0-9]+}}, !%VP{{[0-9]+}}
#endif
}
