// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, int x2a, unsigned char xs, int dest, _Bool pred, int vpreda) {
  uchar256 __local *x0ptr = (uchar256 __local *)x0a;
  uchar256 __local *x1ptr = (uchar256 __local *)x1a;
  ushort128 __local *x2ptr = (ushort128 __local *)x2a;
  ushort256  __local *dptr  = (ushort256 __local *)dest;
  bool256 __local *vpptr  = (bool256 __local *)vpreda;
  ushort256 res = { 0 };
  uchar256 x0 = *x0ptr;
  uchar256 x1 = *x1ptr;
  ushort256 x2 = {*x2ptr, *x2ptr };
  bool256 vpred = *vpptr;

  // Vector + Vector

  res = v_u8_madd_acc16_b(x0, x1, x2, 0,  res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.u8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}

  res = v_u8_madd_acc16_b(x0, x1, x2, SW_SAT,  res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.u8 st acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}

  res = v_u8_madd_acc16_b(x0, x1, x2, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: madd.u8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u8_madd_acc16_b(x0, x1, x2, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: madd.u8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u8_madd_acc16_vb(x0, x1, x2, 0, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.u8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_madd_acc16_vb(x0, x1, x2, SW_SAT, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.u8 st acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_madd_acc16_vb(x0, x1, x2, 0, res, vpred, 1);
  *dptr++ = res;
  // CHECK: madd.u8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

  res = v_u8_madd_acc16_b(x0, xs, x2, 0,  res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.u8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}

  res = v_u8_madd_acc16_b(x0, xs, x2, SW_SAT,  res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.u8 st acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}

  res = v_u8_madd_acc16_b(x0, xs, x2, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: madd.u8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u8_madd_acc16_b(x0, xs, x2, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: madd.u8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u8_madd_acc16_vb(x0, xs, x2, 0, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.u8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_madd_acc16_vb(x0, xs, x2, SW_SAT, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.u8 st acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_madd_acc16_vb(x0, xs, x2, 0, res, vpred, 1);
  *dptr++ = res;
  // CHECK: madd.u8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Immediate

  res = v_u8_madd_acc16_b(x0, 123, x2, 0,  res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.u8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %D{{[0-9]+}}

  res = v_u8_madd_acc16_b(x0, 123, x2, SW_SAT,  res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.u8 st acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %D{{[0-9]+}}

  res = v_u8_madd_acc16_b(x0, 123, x2, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: madd.u8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %D{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u8_madd_acc16_b(x0, 123, x2, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: madd.u8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %D{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u8_madd_acc16_vb(x0, 123, x2, 0, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.u8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %D{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_madd_acc16_vb(x0, 123, x2, SW_SAT, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.u8 st acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %D{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_madd_acc16_vb(x0, 123, x2, 0, res, vpred, 1);
  *dptr++ = res;
  // CHECK: madd.u8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %D{{[0-9]+}}, !%VP{{[0-9]+}}
}
