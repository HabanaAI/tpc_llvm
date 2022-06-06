// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, float xs, int dest, _Bool pred, int vpreda) {
  float64 __local *x0ptr = (float64 __local *)x0a;
  float64 __local *x1ptr = (float64 __local *)x1a;
  float64 __local *dptr  = (float64 __local *)dest;
  bool64 __local *vpptr  = (bool64 __local *)vpreda;
  float64 res = 0;
  float64 x0 = *x0ptr;
  float64 x1 = *x1ptr;
  bool64 vpred = *vpptr;

  // Vector + Vector

  res = v_f32_mac_b(x0, x1, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_f32_mac_b(x0, x1, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f32 neg %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_f32_mac_b(x0, x1, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_f32_mac_b(x0, x1, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_f32_mac_vb(x0, x1, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_f32_mac_vb(x0, x1, res, SW_NEG, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 neg %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_f32_mac_vb(x0, x1, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}


  // Vector + Scalar

  res = v_f32_mac_b(x0, xs, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_f32_mac_b(x0, xs, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f32 neg %V{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_f32_mac_b(x0, xs, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP{{[0-9]+}}

  res = v_f32_mac_b(x0, xs, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%SP{{[0-9]+}}

  res = v_f32_mac_vb(x0, xs, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_f32_mac_vb(x0, xs, res, SW_NEG, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 neg %V{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_f32_mac_vb(x0, xs, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%VP{{[0-9]+}}


  // Vector + Immediate

  res = v_f32_mac_b(x0, 1.5, res, 0, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000

  res = v_f32_mac_b(x0, 1.5, res, SW_NEG, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f32 neg %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000

  res = v_f32_mac_b(x0, 1.5, res, 0, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, %SP{{[0-9]+}}

  res = v_f32_mac_b(x0, 1.5, res, 0, pred, 1);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, !%SP{{[0-9]+}}

  res = v_f32_mac_vb(x0, 1.5, res, 0, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, %VP{{[0-9]+}}

  res = v_f32_mac_vb(x0, 1.5, res, SW_NEG, vpred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 neg %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, %VP{{[0-9]+}}

  res = v_f32_mac_vb(x0, 1.5, res, 0, vpred, 1);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, !%VP{{[0-9]+}}


  // Compatibility functions
  bool256 vpred_c = from_bool64(vpred);

  // Vector + Vector

  res = v_f32_mac_b(x0, x1, res, (0) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_f32_mac_b(x0, x1, res, (1) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f32 neg %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_f32_mac_b(x0, x1, res, (0) << 1, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_f32_mac_b(x0, x1, res, (1) << 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.f32 neg %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_f32_mac_vb(x0, x1, res, (0) << 1, to_bool64(vpred_c), 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

  res = v_f32_mac_vb(x0, x1, res, (1) << 1, to_bool64(vpred_c), 1);
  *dptr++ = res;
  // CHECK: mac.f32 neg %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%VP{{[0-9]+}}

  // Vector + Scalar

  res = v_f32_mac_b(x0, xs, res, (0) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_f32_mac_b(x0, xs, res, (1) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f32 neg %V{{[0-9]+}}, %V{{[0-9]+}}, %S2

  res = v_f32_mac_b(x0, xs, res, (0) << 1, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S2, %SP{{[0-9]+}}

  res = v_f32_mac_b(x0, xs, res, (1) << 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.f32 neg %V{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%SP{{[0-9]+}}

  res = v_f32_mac_vb(x0, xs, res, (0) << 1, to_bool64(vpred_c), 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S2, %VP{{[0-9]+}}

  res = v_f32_mac_vb(x0, xs, res, (1) << 1, to_bool64(vpred_c), 1);
  *dptr++ = res;
  // CHECK: mac.f32 neg %V{{[0-9]+}}, %V{{[0-9]+}}, %S2, !%VP{{[0-9]+}}

  // Vector + Immediate

  res = v_f32_mac_b(x0, 1.5, res, (0) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000

  res = v_f32_mac_b(x0, 1.5, res, (1) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f32 neg %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000

  res = v_f32_mac_b(x0, 1.5, res, (0) << 1, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, %SP{{[0-9]+}}

  res = v_f32_mac_b(x0, 1.5, res, (1) << 1, pred, 1);
  *dptr++ = res;
  // CHECK: mac.f32 neg %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, !%SP{{[0-9]+}}

  res = v_f32_mac_vb(x0, 1.5, res, (0) << 1, to_bool64(vpred_c), 0);
  *dptr++ = res;
  // CHECK: mac.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, %VP{{[0-9]+}}

  res = v_f32_mac_vb(x0, 1.5, res, (1) << 1, to_bool64(vpred_c), 1);
  *dptr++ = res;
  // CHECK: mac.f32 neg %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, !%VP{{[0-9]+}}
}
