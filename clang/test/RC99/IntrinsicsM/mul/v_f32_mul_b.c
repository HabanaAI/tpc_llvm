// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(float a, float b, int dest, int src, _Bool pred) {
  float64 __local *dptr = (float64 __local *)dest;
  float64 x0 = *(float64 __local *)(src + 0 * 256);
  float64 x1 = *(float64 __local *)(src + 1 * 256);
  float64 res = 0;

  res = v_f32_mul_b(x0, x1, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

  res = v_f32_mul_b(x0, x1, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, !%SP{{[0-9]+}}

  res = v_f32_mul_b(x0, x1, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}

  res = v_f32_mul_b(x0, a, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S0, %SP{{[0-9]+}}

  res = v_f32_mul_b(x0, a, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S0, !%SP{{[0-9]+}}

  res = v_f32_mul_b(x0, a, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, %S0

  res = v_f32_mul_b(x0, 1.5, 0, res, pred, 0);
  *dptr++ = res;
  // CHECK: mul.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, %SP{{[0-9]+}}

  res = v_f32_mul_b(x0, 1.5, 0, res, pred, 1);
  *dptr++ = res;
  // CHECK: mul.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000, !%SP{{[0-9]+}}

  res = v_f32_mul_b(x0, 1.5, 0, res, 1, 0);
  *dptr++ = res;
  // CHECK: mul.f32 %V{{[0-9]+}}, %V{{[0-9]+}}, 0x3fc00000
}
