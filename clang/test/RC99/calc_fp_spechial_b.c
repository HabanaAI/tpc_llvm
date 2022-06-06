// RUN: %clang_cc1 -triple -tpc-none-none -std=rc99 -S -O1 -target-cpu goya2 %s -o - | FileCheck %s

void main(tensor in,tensor out) {
  int5 coords = {0, 0, 0, 0, 0};
  // CHECK: calc_fp_special.f16 func=tanh [[CALC_RES:%V[0-9]+]], [[CALC_ARG:%V[0-9]+]], %V{{[0-9]+}}
  // CHECK: cmp_eq.f16 mask_eq_zero [[CMP_RES:%VP[0-9]+]], [[CALC_ARG]], 0xbd
  // CHECK: convert.bf16 target_type=f16 rhne %V{{[0-9]+}}, [[CALC_RES]], ![[CMP_RES]]
  half128   income = 0;
  income = v_f16_ld_tnsr_b (coords, in, 0, income,1,0);
  half128 input_class = v_f16_fclass_b(income, 1, 0, 1, 0);
  half128 result = v_f16_calc_fp_special_b(input_class, input_class, 5, income, 1, 0);
  v_f16_st_tnsr(coords, out, result, 0,1,0);
}

