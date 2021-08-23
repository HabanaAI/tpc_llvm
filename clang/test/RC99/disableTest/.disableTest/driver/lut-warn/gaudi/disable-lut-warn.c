// RUN: %tpc_clang %s -march=gaudi -disable-lut-warn -S 2>&1 | FileCheck %s -allow-empty

void main(int x0, int x3, int dest0)
{

    uint64 __local *ptr_x0 = (uint64 __local *)x0;

    float64 __local *res0 = (float64  __local *)dest0;
    float64 temp_res0 = 0;
    temp_res0 = v_f32_lookup_1c_v_b(*ptr_x0, temp_res0, 1, e_fp32_tanh, x3, 0);
    temp_res0 = v_f32_lookup_1c_v_b(*ptr_x0, temp_res0, 1, e_fp32_rsqrt, x3, 0);
    *res0 = temp_res0;
}

//CHECK-NOT: Performance warning: encountered special function IDs from different interval groups, this will cause LUT misses.
