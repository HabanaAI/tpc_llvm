// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck %s

void main(int src, int dest, _Bool pred) {
  bfloat128 __local *src_ptr = (bfloat128 __local *)src;
  bfloat128 x0 = *src_ptr++;
  short128 __local *res_ptr = (short128 __local *)dest;

  *res_ptr++ = v_bf16_extract_exp_b(x0, 1, 0, 1, 0);
  // CHECK: extract_exp.bf16 biased %V{{[0-9]+}}, %V{{[0-9]+}}
  x0 = *src_ptr++;

  *res_ptr++ = v_bf16_extract_exp_b(x0, 0, 0, 1, 0);
  // CHECK: extract_exp.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}
  x0 = *src_ptr++;

  int res = 0;

  *res_ptr++ = v_bf16_extract_exp_b(x0, 1, res, pred, 0);
  // CHECK: extract_exp.bf16 biased %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = *src_ptr++;

  *res_ptr++ = v_bf16_extract_exp_b(x0, 0, res, pred, 0);
  // CHECK: extract_exp.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = *src_ptr++;
  
  bool256 vpred = v_i1_mov_i1_b(pred, 0, (bool256){0}, 1, 0);

  *res_ptr++ = v_bf16_extract_exp_vb(x0, 1, res, to_bool128(vpred), 0);
  // CHECK: extract_exp.bf16 biased %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  x0 = *src_ptr++;

  *res_ptr++ = v_bf16_extract_exp_vb(x0, 0, res, to_bool128(vpred), 0);
  // CHECK: extract_exp.bf16 %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  x0 = *src_ptr++;
}
