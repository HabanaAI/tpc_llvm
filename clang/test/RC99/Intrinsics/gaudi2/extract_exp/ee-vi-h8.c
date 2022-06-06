// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck %s

void main(int src, int dest, _Bool pred) {
  minihalf256 __local *src_ptr = (minihalf256 __local *)src;
  minihalf256 x0 = *src_ptr++;
  char256 __local *res_ptr = (char256 __local *)dest;
  char256 income = *res_ptr;
  bool256 vpred = v_i1_mov_i1_b(pred, 0, (bool256){0}, 1, 0);


  *res_ptr++ = v_h8_extract_exp_vb((minihalf)0.8, 1, income, vpred, 0);
  // CHECK: extract_exp.f8_152 biased %V{{[0-9]+}}, 0x3a, %VP{{[0-9]+}}

}
