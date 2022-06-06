// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, int x2a, int x3a, float xs, int dest, _Bool pred, int vpreda) {
  float128 __local *x0ptr = (float128 __local *)x0a;
  float64 __local *x1ptr = (float64 __local *)x1a;
  float64 __local *x2ptr = (float64 __local *)x2a;
  float64 __local *x3ptr = (float64 __local *)x3a;
  float128  __local *dptr  = (float128  __local *)dest;
  bool128 __local *vpptr  = (bool128 __local *)vpreda;
  float128 res = { 0.0 };
  float128 x0 = *x0ptr;
  float64 x1 = *x1ptr;
  float64 x2 = *x2ptr;
  float64 x3 = *x3ptr;
  bool128 vpred = *vpptr;

  res = v_f32_mac_x2_b(x0, x1, x2, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_f32_mac_x2_b(x0, 1.0, 1.0, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, 0x3f800000, 0x3f800000, %SP{{[0-9]+}}

  res = v_f32_mac_x2_b(x0, xs, xs, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_f32_mac_x2_b(x0, 1.0, x2, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, 0x3f800000, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_f32_mac_x2_b(x0, xs, x2, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_f32_mac_x2_b(x0, x1, 1.0, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3f800000, %SP{{[0-9]+}}

  res = v_f32_mac_x2_b(x0, x1, xs, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_f32_mac_x2_svv_b(1.0, x1, x2, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 x2 %D{{[0-9]+}}, 0x3f800000, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_f32_mac_x2_svv_b(xs, x1, x2, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 x2 %D{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_f32_mac_x2_vb(x0, x1, x2, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_f32_mac_x2_b(x0, x1, x2, res, SW_NEG, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 neg x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

}
