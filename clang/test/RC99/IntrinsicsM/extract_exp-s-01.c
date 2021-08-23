// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-GAUDI %s


void main(float x0, int dest, _Bool pred) {
  int __local *res_ptr = (int __local *)dest;

  *res_ptr++ = s_f32_extract_exp_s(x0, 1);
  // CHECK:       extract_exp.f32 subtract_bias %S{{[0-9]+}}, %S{{[0-9]+}}, %SP0
  // CHECK-GAUDI: extract_exp.f32 biased %S{{[0-9]+}}, %S{{[0-9]+}}, %SP0
  x0 = s_f32_ld_l_s(4, 0);

  *res_ptr++ = s_f32_extract_exp_s(x0, 0);
  // CHECK:       extract_exp.f32 %S{{[0-9]+}}, %S{{[0-9]+}}, %SP0
  // CHECK-GAUDI: extract_exp.f32 %S{{[0-9]+}}, %S{{[0-9]+}}, %SP0
  x0 = s_f32_ld_l_s(4, 0);

  int res = 0;

  *res_ptr++ = s_f32_extract_exp_s_b(x0, res, 1, pred, 0);
  // CHECK:       extract_exp.f32 subtract_bias %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
  // CHECK-GAUDI: extract_exp.f32 biased %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = s_f32_ld_l_s(4, 0);

  *res_ptr++ = s_f32_extract_exp_s_b(x0, res, 0, pred, 0);
  // CHECK:       extract_exp.f32 %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
  // CHECK-GAUDI: extract_exp.f32 %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = s_f32_ld_l_s(4, 0);
}
