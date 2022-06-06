// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck %s

void main(int dest, int src, _Bool pred) {
  short128 __local *res_ptr = (short128 __local *)dest;
  short128 __local *src_ptr = (short128 __local *)src;

  *res_ptr++ = v_bf16_extract_exp_b(0.8bf, 1, 0, 1, 0);
  // CHECK-DAG: extract_exp.bf16 biased %V{{[0-9]+}}, 0x3f4d

  *res_ptr++ = v_bf16_extract_exp_b(0.9bf, 0, 0, 1, 0);
  // CHECK-DAG: extract_exp.bf16 %V{{[0-9]+}}, 0x3f66

  short128 res = *src_ptr;

  res = v_bf16_extract_exp_b(0.8bf, 1, res, pred, 0);
  // CHECK-DAG: extract_exp.bf16 biased %V{{[0-9]+}}, 0x3f4d, %SP{{[0-9]+}}

  res = v_bf16_extract_exp_b(0.8bf, 0, res, pred, 0);
  // CHECK: extract_exp.bf16 %V{{[0-9]+}}, 0x3f4d, %SP{{[0-9]+}}
  
  bool256 vpred = v_i1_mov_i1_b(pred, 0, (bool256){0}, 1, 0);

  res = v_bf16_extract_exp_vb(0.8bf, 1, res, to_bool128(vpred), 0);
  // CHECK: extract_exp.bf16 biased %V{{[0-9]+}}, 0x3f4d, %VP{{[0-9]+}}

  res = v_bf16_extract_exp_vb(0.8bf, 0, res, to_bool128(vpred), 0);
  // CHECK: extract_exp.bf16 %V{{[0-9]+}}, 0x3f4d, %VP{{[0-9]+}}

  *res_ptr = res;
}
