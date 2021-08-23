// RUN: %tpc_clang %s -S 2>&1 | FileCheck %s
// XFAIL: *
void main(int x0, int x3, int dest0)
{

    uint64 __local *ptr_x0 = (uint64 __local *)x0;

   float64 __local *res0 = (float64  __local *)dest0;
    float64 temp_res0 = 0;
    temp_res0 = v_f32_lookup_c0_v_b(*ptr_x0, temp_res0, 1, e_fp32_pow2, x3, 0);
    temp_res0 = v_f32_lookup_c0_v_b(*ptr_x0, temp_res0, 1, e_fp16_tanh, x3, 0);
    temp_res0 = v_f32_lookup_c0_v_b(*ptr_x0, temp_res0, 1, e_i16_rcp, x3, 0);
    temp_res0 = v_f32_lookup_c0_v_b(*ptr_x0, temp_res0, 1, e_i16_tanh, x3, 0);
    temp_res0 = v_f32_lookup_c0_v_b(*ptr_x0, temp_res0, 1, e_i8_exp_linear, x3, 0);
 
    *res0 = temp_res0;
}

//CHECK: Performance warning: number of requested special function IDs exceeds LUT cache capacity for the Goya architecture, this will cause LUT misses. The cache can hold 1 special function with 256 intervals or 2 special functions, each with 128 intervals or 4 special functions, each with 64 intervals.
