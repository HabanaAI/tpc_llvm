// RUN: %tpc_clang %s -S -march=gaudi 2>&1 | FileCheck %s -allow-empty
// XFAIL: *
void main(int x0, int x2, int x3, int dest0, int dest1, int dest2, int dest3, int dest4) {

  uint64 __local *ptr_x0 = (uint64 __local *)x0;
  uint64 __local *ptr_x1 = (uint64 __local *)x2;

  float64 __local *res0 = (float64 __local *)dest0;
  float64 __local *res1 = (float64 __local *)dest1;
  float64 __local *res2 = (float64 __local *)dest2;
  float64 __local *res3 = (float64 __local *)dest3;
  float64 __local *res4 = (float64 __local *)dest4;

  float64 temp_res0 = 0;
  float64 temp_res1 = 0;
  float64 temp_res2 = 0;
  float64 temp_res3 = 0;
  float64 temp_res4 = 0;

  temp_res0 = v_f32_lookup(*ptr_x0, e_fp32_pow2, 0, temp_res0, x3, 0);
  temp_res1 = v_f32_lookup(*ptr_x1, e_bf16_tanh, 0, temp_res0, x3, 0);
  temp_res2 = v_f32_lookup(*ptr_x1, e_bf16_rcp_scalar_m7, 1, temp_res0, x3, 0);
  temp_res3 = v_f32_lookup(*ptr_x1, e_bf16_rcp_scalar_m7, 1, temp_res0, x3, 0);
  temp_res4 = v_f32_lookup(*ptr_x1, e_bf16_sqrt_poly2_m4_c0, 1, temp_res0, x3, 0);

  *res0 = temp_res0;
  *res1 = temp_res1;
  *res2 = temp_res2;
  *res3 = temp_res3;
  *res4 = temp_res4;

}

//CHECK: Performance warning: number of requested special function IDs exceeds LUT cache capacity for the Gen2+ architecture, this will cause LUT misses.

