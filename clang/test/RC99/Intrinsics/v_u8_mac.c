// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s


void main(int x0a, int x1a, unsigned char xs, int dest, _Bool pred, int vpreda) {
  uchar256 __local *x0ptr = (uchar256 __local *)x0a;
  uchar256 __local *x1ptr = (uchar256 __local *)x1a;
  uint256  __local *dptr  = (uint256  __local *)dest;
  bool256 __local *vpptr  = (bool256 __local *)vpreda;
  uint256 res = { 0 };
  uchar256 x0 = *x0ptr;
  uchar256 x1 = *x1ptr;
  bool256 vpred = *vpptr;

  // Vector + Vector

  res = v_u8_mac_b(x0, x1, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP0

  res = v_u8_mac_b(x0, x1, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP0

  res = v_u8_mac_b(x0, x1, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u8_mac_b(x0, x1, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u8_mac_vb(x0, x1, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_mac_vb(x0, x1, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_mac_vb(x0, x1, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

  res = v_u8_mac_b(x0, xs, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP0

  res = v_u8_mac_b(x0, xs, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP0

  res = v_u8_mac_b(x0, xs, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP{{[0-9]+}}

  res = v_u8_mac_b(x0, xs, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%SP{{[0-9]+}}

  res = v_u8_mac_vb(x0, xs, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_u8_mac_vb(x0, xs, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_u8_mac_vb(x0, xs, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%VP{{[0-9]+}}


  // Vector + Immediate

  res = v_u8_mac_b(x0, 123, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP0

  res = v_u8_mac_b(x0, 123, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP0

  res = v_u8_mac_b(x0, 123, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP{{[0-9]+}}

  res = v_u8_mac_b(x0, 123, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%SP{{[0-9]+}}

  res = v_u8_mac_vb(x0, 123, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %VP{{[0-9]+}}

  res = v_u8_mac_vb(x0, 123, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %VP{{[0-9]+}}

  res = v_u8_mac_vb(x0, 123, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%VP{{[0-9]+}}


  // Compatibility functions

  // Vector + Vector

  res = av_u8_mac_v_v(x0, x1, res, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP0

  res = av_u8_mac_v_v(x0, x1, res, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP0

  res = av_u8_mac_v_v_b(x0, x1, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = av_u8_mac_v_v_b(x0, x1, res, 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = av_u8_mac_v_v_vb(x0, x1, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = av_u8_mac_v_v_vb(x0, x1, res, 1, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}

  // Vector + Scalar

  res = av_u8_mac_v_s(x0, xs, res, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP0

  res = av_u8_mac_v_s(x0, xs, res, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP0

  res = av_u8_mac_v_s_b(x0, xs, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP{{[0-9]+}}

  res = av_u8_mac_v_s_b(x0, xs, res, 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%SP{{[0-9]+}}

  res = av_u8_mac_v_s_vb(x0, xs, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = av_u8_mac_v_s_vb(x0, xs, res, 1, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st %A{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%VP{{[0-9]+}}

  // Vector + Immediate

  res = av_u8_mac_v_s(x0, 123, res, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP0

  res = av_u8_mac_v_s(x0, 123, res, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP0

  res = av_u8_mac_v_s_b(x0, 123, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP{{[0-9]+}}

  res = av_u8_mac_v_s_b(x0, 123, res, 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%SP{{[0-9]+}}

  res = av_u8_mac_v_s_vb(x0, 123, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %VP{{[0-9]+}}

  res = av_u8_mac_v_s_vb(x0, 123, res, 1, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 st %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%VP{{[0-9]+}}
}
