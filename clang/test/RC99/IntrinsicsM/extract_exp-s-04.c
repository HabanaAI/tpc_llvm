// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck %s

void main(float x0, int dest, _Bool pred) {
  int __local *res_ptr = (int __local *)dest;

  *res_ptr++ = s_f32_extract_exp(x0, SW_PRE_LOG, 0, 1, 0);
  // CHECK: extract_exp.f32 pre_log %S{{[0-9]+}}, %S{{[0-9]+}}
  x0 = s_f32_ld_l(4, 0, 0, 1, 0);

  int res = 0;

  *res_ptr++ = s_f32_extract_exp(x0, SW_PRE_LOG, res, pred, 0);
  // CHECK: extract_exp.f32 pre_log %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = s_f32_ld_l(4, 0, 0, 1, 0);
}
