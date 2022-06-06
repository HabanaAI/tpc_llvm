// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, unsigned short xs, int dest, _Bool pred, int vpreda) {
  ushort128 __local *x0ptr = (ushort128 __local *)x0a;
  ushort128 __local *x1ptr = (ushort128 __local *)x1a;
  uint128  __local *dptr  = (uint128  __local *)dest;
  bool128 __local *vpptr  = (bool128 __local *)vpreda;
  uint128 res = { 0 };
  ushort128 x0 = *x0ptr;
  ushort128 x1 = *x1ptr;
  bool128 vpred = *vpptr;

  // Vector + Vector

  res = v_u16_mac_b(x0, x1, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u16_mac_b(x0, x1, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u16_mac_b(x0, x1, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u16_mac_b(x0, x1, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u16_mac_vb(x0, x1, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u16_mac_vb(x0, x1, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u16_mac_vb(x0, x1, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

  res = v_u16_mac_b(x0, xs, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_u16_mac_b(x0, xs, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_u16_mac_b(x0, xs, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP{{[0-9]+}}

  res = v_u16_mac_b(x0, xs, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%SP{{[0-9]+}}

  res = v_u16_mac_vb(x0, xs, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_u16_mac_vb(x0, xs, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_u16_mac_vb(x0, xs, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%VP{{[0-9]+}}


  // Vector + Immediate

  res = v_u16_mac_b(x0, 123, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b

  res = v_u16_mac_b(x0, 123, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b

  res = v_u16_mac_b(x0, 123, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP{{[0-9]+}}

  res = v_u16_mac_b(x0, 123, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%SP{{[0-9]+}}

  res = v_u16_mac_vb(x0, 123, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %VP{{[0-9]+}}

  res = v_u16_mac_vb(x0, 123, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %VP{{[0-9]+}}

  res = v_u16_mac_vb(x0, 123, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%VP{{[0-9]+}}


  // Compatibility functions
  bool256 vpred_c = from_bool128(vpred);

  // Vector + Vector

  res = v_u16_mac_b(x0, x1, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u16_mac_b(x0, x1, res, 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u16_mac_b(x0, x1, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u16_mac_b(x0, x1, res, 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u16_mac_vb(x0, x1, res, 0, to_bool128(vpred_c), 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u16_mac_vb(x0, x1, res, 1, to_bool128(vpred_c), 1);
  *dptr++ = res;
  // CHECK: mac.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}

  // Vector + Scalar

  res = v_u16_mac_b(x0, xs, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_u16_mac_b(x0, xs, res, 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_u16_mac_b(x0, xs, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP{{[0-9]+}}

  res = v_u16_mac_b(x0, xs, res, 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%SP{{[0-9]+}}

  res = v_u16_mac_vb(x0, xs, res, 0, to_bool128(vpred_c), 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_u16_mac_vb(x0, xs, res, 1, to_bool128(vpred_c), 1);
  *dptr++ = res;
  // CHECK: mac.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%VP{{[0-9]+}}

  // Vector + Immediate

  res = v_u16_mac_b(x0, 123, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b

  res = v_u16_mac_b(x0, 123, res, 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b

  res = v_u16_mac_b(x0, 123, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP{{[0-9]+}}

  res = v_u16_mac_b(x0, 123, res, 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%SP{{[0-9]+}}

  res = v_u16_mac_vb(x0, 123, res, 0, to_bool128(vpred_c), 0);
  *dptr++ = res;
  // CHECK: mac.u16 %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %VP{{[0-9]+}}

  res = v_u16_mac_vb(x0, 123, res, 1, to_bool128(vpred_c), 1);
  *dptr++ = res;
  // CHECK: mac.u16 st %D{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%VP{{[0-9]+}}
}
