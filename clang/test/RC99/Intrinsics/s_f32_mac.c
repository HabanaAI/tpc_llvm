// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi -bfloat16 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(float x0, float x1, int dest, _Bool pred) {
  float __local *dptr = (float  __local *)dest;
  float res = 0;

  res = s_f32_mac(x0, x1, res, (0) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %S{{[0-9]+}}, %S0, %S1

  res = s_f32_mac(x0, 1.5, res, (0) << 1, 1, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %S{{[0-9]+}}, %S0, 0x3fc00000

  res = s_f32_mac(x0, x1, res, (0) << 1, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %S{{[0-9]+}}, %S0, %S1, %SP{{[0-9]+}}

  res = s_f32_mac(x0, 1.5, res, (0) << 1, pred, 0);
  *dptr++ = res;
  // CHECK: mac.f32 %S{{[0-9]+}}, %S0, 0x3fc00000, %SP{{[0-9]+}}
}
