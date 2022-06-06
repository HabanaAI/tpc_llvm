// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, int x2a, int zpa, float xs, int dest, int vpreda,  _Bool pred) {
  uchar256 __local *x0ptr = (uchar256 __local *)x0a;
  uchar256 __local *x1ptr = (uchar256 __local *)x1a;
  int256 __local *x2ptr = (int256 __local *)x2a;
  uchar256 __local *zpptr = (uchar256 __local *)zpa;
  int256 __local *dptr = (int256 __local *)dest;
  bool256 __local *vpptr  = (bool256 __local *)vpreda;

  uchar256 x0 = *x0ptr;
  uchar256 x1 = *x1ptr;
  int256 x2 = *x2ptr;
  uchar256 zp = *zpptr;
  bool256 vpred = *vpptr;

  int256 res = {0};

  {
    res = v_u8_madd_zp_acc32_b(x0, x1, x2, zp, 0, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.u8 acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_u8_madd_zp_acc32_b(x0, x1, x2, zp, SW_SAT, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.u8 st acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_u8_madd_zp_acc32_b(x0, x1, x2, zp, SW_NEG, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.u8 neg acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_u8_madd_zp_acc32_b(x0, x1, x2, zp, SW_NEG_ZP, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.u8 acc_i32 zp neg_zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_u8_madd_zp_acc32_b(x0, x1, x2, zp, SW_SAT | SW_NEG | SW_NEG_ZP, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.u8 st neg acc_i32 zp neg_zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}, %SP{{[0-9]+}}
  }

  {
    res = v_u8_madd_zp_acc32_vb(x0, x1, x2, zp, 0, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.u8 acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_u8_madd_zp_acc32_vb(x0, x1, x2, zp, SW_SAT, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.u8 st acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_u8_madd_zp_acc32_vb(x0, x1, x2, zp, SW_NEG, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.u8 neg acc_i32 zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_u8_madd_zp_acc32_vb(x0, x1, x2, zp, SW_NEG_ZP, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.u8 acc_i32 zp neg_zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_u8_madd_zp_acc32_vb(x0, x1, x2, zp, SW_SAT | SW_NEG | SW_NEG_ZP, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.u8 st neg acc_i32 zp neg_zp %ZP_REG %A{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %A{{[0-9]+}}, %VP{{[0-9]+}}
  }
}
