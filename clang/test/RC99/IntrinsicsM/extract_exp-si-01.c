// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-GAUDI %s


void main(int dest,_Bool pred) {
  volatile int __local *dptr = (int __local *)dest;

  int res = *dptr++;

  res = s_f32_extract_exp_s_b(0.8, res, 1, pred, 0);
  // CHECK:       extract_exp.f32 subtract_bias %S{{[0-9]+}}, 0x3f4ccccd, %SP{{[0-9]+}}
  // CHECK-GAUDI: extract_exp.f32 biased %S{{[0-9]+}}, 0x3f4ccccd, %SP{{[0-9]+}}
  *dptr++ = res;

  res = s_f32_extract_exp_s_b(0.8, res, 0, pred, 0);
  // CHECK:       extract_exp.f32 %S{{[0-9]+}}, 0x3f4ccccd, %SP{{[0-9]+}}
  // CHECK-GAUDI: extract_exp.f32 %S{{[0-9]+}}, 0x3f4ccccd, %SP{{[0-9]+}}
  *dptr++ = res;

  res = s_f32_extract_exp_s(0.8, 1);
  // CHECK:       extract_exp.f32 subtract_bias %S{{[0-9]+}}, 0x3f4ccccd, %SP0
  // CHECK-GAUDI: extract_exp.f32 biased %S{{[0-9]+}}, 0x3f4ccccd, %SP0
  *dptr++ = res;

  res = s_f32_extract_exp_s(0.8, 0);
  // CHECK:       extract_exp.f32 %S{{[0-9]+}}, 0x3f4ccccd, %SP0
  // CHECK-GAUDI: extract_exp.f32 %S{{[0-9]+}}, 0x3f4ccccd, %SP0
  *dptr++ = res;
}
