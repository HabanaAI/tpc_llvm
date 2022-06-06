// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, int x2a, int zpa, float xs, int dest, int vpreda,  _Bool pred) {
  char256 __local *x0ptr = (char256 __local *)x0a;
  char256 __local *x1ptr = (char256 __local *)x1a;
  short256 __local *x2ptr = (short256 __local *)x2a;
  char256 __local *zpptr = (char256 __local *)zpa;
  short256 __local *dptr = (short256 __local *)dest;
  bool256 __local *vpptr  = (bool256 __local *)vpreda;

  char256 x0 = *x0ptr;
  char256 x1 = *x1ptr;
  short256 x2 = *x2ptr;
  char256 zp = *zpptr;
  bool256 vpred = *vpptr;

  short256 res = {0};

  {
    res = v_i8_madd_zp_acc16_b(x0, x1, x2, zp, 0, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.i8 acc_i16 zp %ZP_REG %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_i8_madd_zp_acc16_b(x0, x1, x2, zp, SW_SAT, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.i8 st acc_i16 zp %ZP_REG %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_i8_madd_zp_acc16_b(x0, x1, x2, zp, SW_NEG, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.i8 neg acc_i16 zp %ZP_REG %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_i8_madd_zp_acc16_b(x0, x1, x2, zp, SW_NEG_ZP, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.i8 acc_i16 zp neg_zp %ZP_REG %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_i8_madd_zp_acc16_b(x0, x1, x2, zp, SW_SAT | SW_NEG | SW_NEG_ZP, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.i8 st neg acc_i16 zp neg_zp %ZP_REG %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %SP{{[0-9]+}}
  }

  {
    res = v_i8_madd_zp_acc16_vb(x0, x1, x2, zp, 0, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.i8 acc_i16 zp %ZP_REG %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_i8_madd_zp_acc16_vb(x0, x1, x2, zp, SW_SAT, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.i8 st acc_i16 zp %ZP_REG %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_i8_madd_zp_acc16_vb(x0, x1, x2, zp, SW_NEG, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.i8 neg acc_i16 zp %ZP_REG %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_i8_madd_zp_acc16_vb(x0, x1, x2, zp, SW_NEG_ZP, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.i8 acc_i16 zp neg_zp %ZP_REG %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_i8_madd_zp_acc16_vb(x0, x1, x2, zp, SW_SAT | SW_NEG | SW_NEG_ZP, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.i8 st neg acc_i16 zp neg_zp %ZP_REG %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %VP{{[0-9]+}}
  }
}
