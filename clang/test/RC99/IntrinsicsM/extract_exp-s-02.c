// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-GAUDI %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck --check-prefix=CHECK-GAUDI %s

void main(_BFloat16 x0, int dest, _Bool pred) {
  int __local *res_ptr = (int __local *)dest;

  *res_ptr++ = s_bf16_extract_exp(x0, 1, 0, 1, 0);
  // CHECK: extract_exp.bf16 biased %S{{[0-9]+}}, %S0
  // CHECK-GAUDI: extract_exp.bf16 biased %S{{[0-9]+}}, %S{{[0-9]+}}
  x0 = s_bf16_ld_l(4, 0, 0, 1, 0);

  *res_ptr++ = s_bf16_extract_exp(x0, 0, 0, 1, 0);
  // CHECK: extract_exp.bf16 %S{{[0-9]+}}, %S0
  // CHECK-GAUDI: extract_exp.bf16 %S{{[0-9]+}}, %S{{[0-9]+}}
  x0 = s_bf16_ld_l(4, 0, 0, 1, 0);

  int res = 0;

  *res_ptr++ = s_bf16_extract_exp(x0, 1, res, pred, 0);
  // CHECK: extract_exp.bf16 biased %S{{[0-9]+}}, %S0, %SP{{[0-9]+}}
  // CHECK-GAUDI: extract_exp.bf16 biased %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = s_bf16_ld_l(4, 0, 0, 1, 0);

  *res_ptr++ = s_bf16_extract_exp(x0, 0, res, pred, 0);
  // CHECK: extract_exp.bf16 %S{{[0-9]+}}, %S0, %SP{{[0-9]+}}
  // CHECK-GAUDI: extract_exp.bf16 %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = s_bf16_ld_l(4, 0, 0, 1, 0);
}
