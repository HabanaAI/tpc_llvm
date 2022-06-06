// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, int x2a, unsigned short xs, int dest, _Bool pred, int vpreda) {
  ushort128 __local *x0ptr = (ushort128 __local *)x0a;
  ushort128 __local *x1ptr = (ushort128 __local *)x1a;
  uint64   __local *x2ptr = (uint64 __local *)x2a;
  uint128  __local *dptr  = (uint128  __local *)dest;
  bool128 __local *vpptr  = (bool128 __local *)vpreda;
  uint128 res = { 0 };
  ushort128 x0 = *x0ptr;
  ushort128 x1 = *x1ptr;
  uint128 x2 = {*x2ptr, *x2ptr};
  bool128 vpred = *vpptr;

  // Vector + Vector

  res = v_u16_madd_b(x0, x1, x2, 0,  res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}

  res = v_u16_madd_b(x0, x1, x2, SW_SAT,  res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}

  res = v_u16_madd_b(x0, x1, x2, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: madd.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u16_madd_b(x0, x1, x2, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: madd.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u16_madd_vb(x0, x1, x2, 0, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u16_madd_vb(x0, x1, x2, SW_SAT, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u16_madd_vb(x0, x1, x2, 0, res, vpred, 1);
  *dptr++ = res;
  // CHECK: madd.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

  res = v_u16_madd_b(x0, xs, x2, 0,  res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}

  res = v_u16_madd_b(x0, xs, x2, SW_SAT,  res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}

  res = v_u16_madd_b(x0, xs, x2, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: madd.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u16_madd_b(x0, xs, x2, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: madd.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u16_madd_vb(x0, xs, x2, 0, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u16_madd_vb(x0, xs, x2, SW_SAT, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u16_madd_vb(x0, xs, x2, 0, res, vpred, 1);
  *dptr++ = res;
  // CHECK: madd.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S3, %D{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Immediate

  res = v_u16_madd_b(x0, 123, x2, 0,  res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %D{{[0-9]+}}

  res = v_u16_madd_b(x0, 123, x2, SW_SAT,  res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %D{{[0-9]+}}

  res = v_u16_madd_b(x0, 123, x2, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: madd.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %D{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u16_madd_b(x0, 123, x2, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: madd.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %D{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u16_madd_vb(x0, 123, x2, 0, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %D{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u16_madd_vb(x0, 123, x2, SW_SAT, res, vpred, 0);
  *dptr++ = res;
  // CHECK: madd.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %D{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u16_madd_vb(x0, 123, x2, 0, res, vpred, 1);
  *dptr++ = res;
  // CHECK: madd.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %D{{[0-9]+}}, !%VP{{[0-9]+}}

}
