// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s


void main(int src, int dest, _Bool pred) {
  bfloat128 __local *src_ptr = (bfloat128 __local *)src;
  bfloat128 x0 = *src_ptr++;
  short128 __local *res_ptr = (short128 __local *)dest;

  *res_ptr++ = v_bf16_extract_exp_v(x0, 1);
  // CHECK: extract_exp.bf16 biased %V{{[0-9]+}}, %V{{[0-9]+}}, %SP0
  x0 = *src_ptr++;

  *res_ptr++ = v_bf16_extract_exp_v(x0, 0);
  // CHECK: extract_exp.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, %SP0
  x0 = *src_ptr++;

  int res = 0;

  *res_ptr++ = v_bf16_extract_exp_v_b(x0, res, 1, pred, 0);
  // CHECK: extract_exp.bf16 biased %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = *src_ptr++;

  *res_ptr++ = v_bf16_extract_exp_v_b(x0, res, 0, pred, 0);
  // CHECK: extract_exp.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = *src_ptr++;
  
  bool256 vpred = bv_b_mov_b(pred);

  *res_ptr++ = v_bf16_extract_exp_v_vb(x0, res, 1, vpred, 0);
  // CHECK: extract_exp.bf16 biased %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  x0 = *src_ptr++;

  *res_ptr++ = v_bf16_extract_exp_v_vb(x0, res, 0, vpred, 0);
  // CHECK: extract_exp.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  x0 = *src_ptr++;
}
