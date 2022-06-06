// RUN: %tpc_clang %s -S -march=gaudi 2>&1 | FileCheck %s -allow-empty


void main(int x0, int x2, int x3, int dest0)
{
    uint64 __local *ptr_x0 = (uint64 __local *)x0;
   float64 __local *res0 = (float64  __local *)dest0;

    float64 temp_res0 = 0;

    temp_res0 = v_f32_lookup_1c(*ptr_x0, e_fp32_sqrt, 0, temp_res0, x3, 0);

    *res0 = temp_res0;
}

//CHECK-NOT: Warning: Producer wasn't found in latencies DB therefore returning defualt latency


