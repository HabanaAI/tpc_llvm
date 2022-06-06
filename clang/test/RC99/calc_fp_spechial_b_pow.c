// RUN: %clang_cc1 -triple -tpc-none-none -std=rc99 -S -O1 -target-cpu goya2 %s -o - | FileCheck %s

void main(tensor in,tensor out) {
  int5 coords = {0, 0, 0, 0, 0};
// CHECK:  calc_fp_special.f16 func=pow
// CHECK:  convert.bf16 target_type=f16 rhne
// CHECK:  sel_eq.f16
  half128   income = 0;
  income = v_f16_ld_tnsr_b (coords, in, 0, income,1,0);
  half128 input_class = v_f16_fclass_b(income, 1, 0, 1, 0);
  half128 result = v_f16_calc_fp_special_b(input_class, input_class,7, income, 1, 0);
  v_f16_st_tnsr(coords, out, result, 0,1,0);
}

