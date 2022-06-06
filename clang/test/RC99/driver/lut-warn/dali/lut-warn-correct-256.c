// RUN: %tpc_clang %s -S 2>&1 | FileCheck %s -allow-empty

void main(int x0, int x2, int x3, int dest0, int dest1)
{

    uint64 __local *ptr_x0 = (uint64 __local *)x0;
    uint64 __local *ptr_x1 = (uint64 __local *)x2;


   float64 __local *res0 = (float64  __local *)dest0;
   float64 __local *res1 = (float64  __local *)dest1;

    float64 temp_res0 = 0;
    float64 temp_res1 = 0;

    temp_res0 = v_f32_lookup_c0(*ptr_x0, e_fp32_tanh, 1, temp_res0, x3, 0);
    temp_res1 = v_f32_lookup_c0(*ptr_x1, e_fp32_tanh, 1, temp_res0, x3, 0);
    *res0 = temp_res0;
    *res1 = temp_res1;
}

//CHECK-NOT: Performance warning: number of requested special function IDs exceeds LUT cache capacity for the Goya architecture, this will cause LUT misses. The cache can hold 1 special function with 256 intervals or 2 special functions, each with 128 intervals or 4 special functions, each with 64 intervals.

