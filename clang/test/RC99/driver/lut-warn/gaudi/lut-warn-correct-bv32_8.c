// RUN: %tpc_clang %s -S -march=gaudi 2>&1 | FileCheck %s -allow-empty

void main(int x0, int x2, int x3, int dest0, int dest1, int dest2, int dest3) {

  uint64 __local *ptr_x0 = (uint64 __local *)x0;
  uint64 __local *ptr_x1 = (uint64 __local *)x2;

  float64 __local *res0 = (float64 __local *)dest0;
  float64 __local *res1 = (float64 __local *)dest1;
  float64 __local *res2 = (float64 __local *)dest2;
  float64 __local *res3 = (float64 __local *)dest3;

  float64 temp_res0 = 0;
  float64 temp_res1 = 0;
  float64 temp_res2 = 0;
  float64 temp_res3 = 0;

  temp_res0 = v_f32_lookup_1c_v_b(*ptr_x0, temp_res0, 0, e_fp32_pow2, x3, 0);
  temp_res1 = v_f32_lookup_1c_v_b(*ptr_x1, temp_res0, 2, e_i8_exp_linear, x3, 0);
  temp_res2 = v_f32_lookup_1c_v_b(*ptr_x1, temp_res0, 3, e_i8_tanh_linear, x3, 0);
  temp_res3 = v_f32_lookup_1c_v_b(*ptr_x1, temp_res0, 3, e_i8_sigmoid_linear, x3, 0);

  *res0 = temp_res0;
  *res1 = temp_res1;
  *res2 = temp_res2;
  *res3 = temp_res3;
}

//CHECK-NOT: Performance warning: number of requested special function IDs exceeds LUT cache capacity for the Gen2+ architecture, this will cause LUT misses.

