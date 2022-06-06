// RUN: %codegen -S -O0 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O0 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck %s

void main(int dest, _Bool pred) {
  int __local *res_ptr = (int __local *)dest;

  *res_ptr++ = s_bf16_extract_exp(0.8, 1, 0, 1, 0);
  // CHECK: extract_exp.bf16 biased %S{{[0-9]+}}, 0x3f4d

  *res_ptr++ = s_bf16_extract_exp(0.8, 0, 0, 1, 0);
  // CHECK: extract_exp.bf16 %S{{[0-9]+}}, 0x3f4d

  int res = 0;

  *res_ptr++ = s_bf16_extract_exp(0.8, 1, res, pred, 0);
  // CHECK: extract_exp.bf16 biased %S{{[0-9]+}}, 0x3f4d, %SP{{[0-9]+}}

  *res_ptr++ = s_bf16_extract_exp(0.8, 0, res, pred, 0);
  // CHECK: extract_exp.bf16 %S{{[0-9]+}}, 0x3f4d, %SP{{[0-9]+}}
}
