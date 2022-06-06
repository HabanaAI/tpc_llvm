// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck %s

void main(int src, int dest, _Bool pred) {
  minifloat256 __local *src_ptr = (minifloat256 __local *)src;
  minifloat256 x0 = *src_ptr++;
  char256 __local *res_ptr = (char256 __local *)dest;
  char256 income = *res_ptr;
  *res_ptr++ = v_f8_extract_exp_b(x0, 1,income,1,0);
  // CHECK: extract_exp.f8_143 biased %V{{[0-9]+}}, %V{{[0-9]+}}
  x0 = *src_ptr++;

  *res_ptr++ = v_f8_extract_exp_b(x0, 0,income,1,0);
  // CHECK: extract_exp.f8_143 %V{{[0-9]+}}, %V{{[0-9]+}}
  x0 = *src_ptr++;

  int res = 0;

  *res_ptr++ = v_f8_extract_exp_b(x0, 1,income, pred, 0);
  // CHECK: extract_exp.f8_143 biased %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = *src_ptr++;

  *res_ptr++ = v_f8_extract_exp_b(x0, 0, income, pred, 0);
  // CHECK: extract_exp.f8_143 %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = *src_ptr++;
  
  bool256 vpred = v_i1_mov_i1_b(pred, 0, (bool256){0}, 1, 0);

  *res_ptr++ = v_f8_extract_exp_vb(x0, 1, 1, vpred, 0);
  // CHECK: extract_exp.f8_143 biased %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  x0 = *src_ptr++;
  
  *res_ptr++ = v_f8_extract_exp_vb(x0, 0, income, vpred, 0);
  // CHECK: extract_exp.f8_143 %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
  x0 = *src_ptr++;

}
