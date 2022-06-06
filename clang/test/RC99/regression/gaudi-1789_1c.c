// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck %s

void main(int x0, int x3, int dest0)
{
    
    uint64 __local *ptr_x0 = (uint64 __local *)x0;
    
    float64 __local *res0 = (float64  __local *)dest0;
    float64 temp_res0 = 0;
    temp_res0 = v_f32_lookup_1c(*ptr_x0, 1, SW_X2, temp_res0, x3, 0);
    *res0 = temp_res0;

    temp_res0 = v_f32_lookup_1c(*ptr_x0, 1, SW_X2, temp_res0, x3, 0);
    *res0 = temp_res0;
}

// CHECK: cache_invalidate lu
// CHECK: lookup_1c BV32 x2 %V1, %V0, 0x1, %SP{{[0-9]+}}
// CHECK: cache_invalidate lu
// CHECK: lookup_1c BV32 x2 %V1, %V0, 0x1, %SP{{[0-9]+}}
