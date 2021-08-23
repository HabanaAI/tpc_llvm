// RUN: %codegen -S -O0 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s


void main(int dest, _Bool pred) {
  int __local *res_ptr = (int __local *)dest;

  *res_ptr++ = s_bf16_extract_exp_s(0.8, 1);
  // CHECK: extract_exp.bf16 biased %S{{[0-9]+}}, 0x3f4d, %SP0

  *res_ptr++ = s_bf16_extract_exp_s(0.8, 0);
  // CHECK: extract_exp.bf16 %S{{[0-9]+}}, 0x3f4d, %SP0

  int res = 0;

  *res_ptr++ = s_bf16_extract_exp_s_b(0.8, res, 1, pred, 0);
  // CHECK: extract_exp.bf16 biased %S{{[0-9]+}}, 0x3f4d, %SP{{[0-9]+}}

  *res_ptr++ = s_bf16_extract_exp_s_b(0.8, res, 0, pred, 0);
  // CHECK: extract_exp.bf16 %S{{[0-9]+}}, 0x3f4d, %SP{{[0-9]+}}
}
