// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s


void main(int dest, int src, _Bool pred) {
  short128 __local *res_ptr = (short128 __local *)dest;
  short128 __local *src_ptr = (short128 __local *)src;

  *res_ptr++ = v_bf16_extract_exp_s(0.8bf, 1);
  // CHECK-DAG: extract_exp.bf16 biased %V{{[0-9]+}}, 0x3f4d, %SP0

  *res_ptr++ = v_bf16_extract_exp_s(0.9bf, 0);
  // CHECK-DAG: extract_exp.bf16 %V{{[0-9]+}}, 0x3f66, %SP0

  short128 res = *src_ptr;

  res = v_bf16_extract_exp_s_b(0.8bf, res, 1, pred, 0);
  // CHECK-DAG: extract_exp.bf16 biased %V{{[0-9]+}}, 0x3f4d, %SP{{[0-9]+}}

  res = v_bf16_extract_exp_s_b(0.8bf, res, 0, pred, 0);
  // CHECK: extract_exp.bf16 %V{{[0-9]+}}, 0x3f4d, %SP{{[0-9]+}}
  
  bool256 vpred = bv_b_mov_b(pred);

  res = v_bf16_extract_exp_s_vb(0.8bf, res, 1, vpred, 0);
  // CHECK: extract_exp.bf16 biased %V{{[0-9]+}}, 0x3f4d, %VP{{[0-9]+}}

  res = v_bf16_extract_exp_s_vb(0.8bf, res, 0, vpred, 0);
  // CHECK: extract_exp.bf16 %V{{[0-9]+}}, 0x3f4d, %VP{{[0-9]+}}

  *res_ptr = res;
}
