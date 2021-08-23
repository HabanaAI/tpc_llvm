// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s


void main(int x0a, int x1a, short xs, int dest, _Bool pred, int vpreda) {
  short128 __local *x0ptr = (short128 __local *)x0a;
  short128 __local *x1ptr = (short128 __local *)x1a;
  int128  __local *dptr  = (int128  __local *)dest;
  bool128 __local *vpptr  = (bool128 __local *)vpreda;
  int128 res = { 0 };
  short128 x0 = *x0ptr;
  short128 x1 = *x1ptr;
  bool128 vpred = *vpptr;

  // Vector + Vector

  res = v_i16_mac_b(x0, x1, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP0

  res = v_i16_mac_b(x0, x1, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP0

  res = v_i16_mac_b(x0, x1, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_i16_mac_b(x0, x1, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_i16_mac_vb(x0, x1, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_i16_mac_vb(x0, x1, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.i16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_i16_mac_vb(x0, x1, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

  res = v_i16_mac_b(x0, xs, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP0

  res = v_i16_mac_b(x0, xs, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP0

  res = v_i16_mac_b(x0, xs, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP{{[0-9]+}}

  res = v_i16_mac_b(x0, xs, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%SP{{[0-9]+}}

  res = v_i16_mac_vb(x0, xs, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_i16_mac_vb(x0, xs, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.i16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_i16_mac_vb(x0, xs, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%VP{{[0-9]+}}


  // Vector + Immediate

  res = v_i16_mac_b(x0, 123, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP0

  res = v_i16_mac_b(x0, 123, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.i16 st %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP0

  res = v_i16_mac_b(x0, 123, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP{{[0-9]+}}

  res = v_i16_mac_b(x0, 123, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%SP{{[0-9]+}}

  res = v_i16_mac_vb(x0, 123, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %VP{{[0-9]+}}

  res = v_i16_mac_vb(x0, 123, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.i16 st %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %VP{{[0-9]+}}

  res = v_i16_mac_vb(x0, 123, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%VP{{[0-9]+}}


  // Compatibility functions
  bool256 vpred_c = from_bool128(vpred);

  // Vector + Vector

  res = av_i16_mac_v_v(x0, x1, res, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP0

  res = av_i16_mac_v_v(x0, x1, res, 1);
  *dptr++ = res;
  // CHECK: mac.i16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP0

  res = av_i16_mac_v_v_b(x0, x1, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = av_i16_mac_v_v_b(x0, x1, res, 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.i16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = av_i16_mac_v_v_vb(x0, x1, res, 0, vpred_c, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = av_i16_mac_v_v_vb(x0, x1, res, 1, vpred_c, 1);
  *dptr++ = res;
  // CHECK: mac.i16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}

  // Vector + Scalar

  res = av_i16_mac_v_s(x0, xs, res, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP0

  res = av_i16_mac_v_s(x0, xs, res, 1);
  *dptr++ = res;
  // CHECK: mac.i16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP0

  res = av_i16_mac_v_s_b(x0, xs, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP{{[0-9]+}}

  res = av_i16_mac_v_s_b(x0, xs, res, 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.i16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%SP{{[0-9]+}}

  res = av_i16_mac_v_s_vb(x0, xs, res, 0, vpred_c, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = av_i16_mac_v_s_vb(x0, xs, res, 1, vpred_c, 1);
  *dptr++ = res;
  // CHECK: mac.i16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%VP{{[0-9]+}}

  // Vector + Immediate

  res = av_i16_mac_v_s(x0, 123, res, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP0

  res = av_i16_mac_v_s(x0, 123, res, 1);
  *dptr++ = res;
  // CHECK: mac.i16 st %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP0

  res = av_i16_mac_v_s_b(x0, 123, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP{{[0-9]+}}

  res = av_i16_mac_v_s_b(x0, 123, res, 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.i16 st %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%SP{{[0-9]+}}

  res = av_i16_mac_v_s_vb(x0, 123, res, 0, vpred_c, 0);
  *dptr++ = res;
  // CHECK: mac.i16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %VP{{[0-9]+}}

  res = av_i16_mac_v_s_vb(x0, 123, res, 1, vpred_c, 1);
  *dptr++ = res;
  // CHECK: mac.i16 st %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%VP{{[0-9]+}}
}
