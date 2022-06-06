// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, int x2a, int x3a, float xs, int dest, int vpreda,  _Bool pred) {
  float128 __local *x0ptr = (float128 __local *)x0a;
  float64 __local *x1ptr = (float64 __local *)x1a;
  float128 __local *x2ptr = (float128 __local *)x2a;
  float64 __local *x3ptr = (float64 __local *)x3a;
  float128 __local *dptr = (float128 __local *)dest;
  bool128 __local *vpptr  = (bool128 __local *)vpreda;

  float128 x0 = *x0ptr;
  float64 x1 = *x1ptr;
  float128 x2 = *x2ptr;
  float64 x3 = *x3ptr;
  bool128 vpred = *vpptr;

  float128 res = {0};

  {
    res = v_f32_madd_x2_b(x0, x1, x2, x3, 0, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_b(x0, 1.0, x2, 1.0, 0, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, 0x3f800000, %D{{[0-9]+}}, 0x3f800000, %SP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_b(x0, xs, x2, xs, 0, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %S{{[0-9]+}}, %D{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_b(x0, 1.0, x2, x3, 0, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, 0x3f800000, %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_b(x0, xs, x2, x3, 0, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %S{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_b(x0, x1, x2, 1.0, 0, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, 0x3f800000, %SP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_b(x0, x1, x2, xs, 0, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_vvsv_b(x0, x1, 1.0, x3, 0, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3f800000, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_vvsv_b(x0, x1, xs, x3, 0, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_svvv_b(1.0, x1, x2, x3, 0, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, 0x3f800000, %V{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_svvv_b(xs, x1, x2, x3, 0, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }

  {
    res = v_f32_madd_x2_vb(x0, x1, x2, x3, 0, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_vb(x0, 1.0, x2, 1.0, 0, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, 0x3f800000, %D{{[0-9]+}}, 0x3f800000, %VP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_vb(x0, xs, x2, xs, 0, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %S{{[0-9]+}}, %D{{[0-9]+}}, %S{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_vb(x0, 1.0, x2, x3, 0, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, 0x3f800000, %D{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_vb(x0, xs, x2, x3, 0, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %S{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_vb(x0, x1, x2, 1.0, 0, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, 0x3f800000, %VP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_vb(x0, x1, x2, xs, 0, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %S{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_vvsv_vb(x0, x1, 1.0, x3, 0, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, 0x3f800000, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_vvsv_vb(x0, x1, xs, x3, 0, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_svvv_vb(1.0, x1, x2, x3, 0, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, 0x3f800000, %V{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_svvv_vb(xs, x1, x2, x3, 0, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 x2 %D{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }

  {
    res = v_f32_madd_x2_b(x0, x1, x2, x3, SW_NEG, res, pred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 neg x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  }
  {
    res = v_f32_madd_x2_vb(x0, x1, x2, x3, SW_NEG, res, vpred, 0);
    *dptr++ = res;
    // CHECK: madd.f32 neg x2 %D{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  }
}
