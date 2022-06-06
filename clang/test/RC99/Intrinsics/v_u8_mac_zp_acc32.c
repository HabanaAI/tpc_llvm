// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, int zpa, unsigned char xs, int dest, _Bool pred, int vpreda) {
  uchar256 __local *x0ptr = (uchar256 __local *)x0a;
  uchar256 __local *x1ptr = (uchar256 __local *)x1a;
  uchar256 __local *zpptr = (uchar256 __local *)zpa;
  int256  __local *dptr  = (int256  __local *)dest;
  bool256 __local *vpptr  = (bool256 __local *)vpreda;
  int256 res = { 0 };
  uchar256 x0 = *x0ptr;
  uchar256 x1 = *x1ptr;
  uchar256 zp = *zpptr;
  bool256 vpred = *vpptr;

  // Vector + Vector

  res = v_u8_mac_zp_acc32_b(x0, x1, zp, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u8_mac_zp_acc32_b(x0, x1, zp, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_u8_mac_zp_acc32_b(x0, x1, zp, res, SW_NEG_ZP, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 acc_i32 zp neg_zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_u8_mac_zp_acc32_b(x0, x1, zp, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_u8_mac_zp_acc32_vb(x0, x1, zp, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_mac_zp_acc32_vb(x0, x1, zp, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_u8_mac_zp_acc32_vb(x0, x1, zp, res, SW_NEG_ZP, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 acc_i32 zp neg_zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

  res = v_u8_mac_zp_acc32_b(x0, xs, zp, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %S3

  res = v_u8_mac_zp_acc32_b(x0, xs, zp, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %S3

  res = v_u8_mac_zp_acc32_b(x0, xs, zp, res, SW_NEG_ZP, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 acc_i32 zp neg_zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %S3, %SP{{[0-9]+}}

  res = v_u8_mac_zp_acc32_b(x0, xs, zp, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %S3, !%SP{{[0-9]+}}

  res = v_u8_mac_zp_acc32_vb(x0, xs, zp, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %S3, %VP{{[0-9]+}}

  res = v_u8_mac_zp_acc32_vb(x0, xs, zp, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %S3, %VP{{[0-9]+}}

  res = v_u8_mac_zp_acc32_vb(x0, xs, zp, res, SW_NEG_ZP, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 acc_i32 zp neg_zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %S3, !%VP{{[0-9]+}}


  // Vector + Immediate

  res = v_u8_mac_zp_acc32_b(x0, 123, zp, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b

  res = v_u8_mac_zp_acc32_b(x0, 123, zp, res, SW_SAT, 1, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b

  res = v_u8_mac_zp_acc32_b(x0, 123, zp, res, SW_NEG_ZP, pred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 acc_i32 zp neg_zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %SP{{[0-9]+}}

  res = v_u8_mac_zp_acc32_b(x0, 123, zp, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%SP{{[0-9]+}}

  res = v_u8_mac_zp_acc32_vb(x0, 123, zp, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %VP{{[0-9]+}}

  res = v_u8_mac_zp_acc32_vb(x0, 123, zp, res, SW_SAT, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.u8 st acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, %VP{{[0-9]+}}

  res = v_u8_mac_zp_acc32_vb(x0, 123, zp, res, SW_NEG_ZP, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.u8 acc_i32 zp neg_zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, 0x7b, !%VP{{[0-9]+}}
}
