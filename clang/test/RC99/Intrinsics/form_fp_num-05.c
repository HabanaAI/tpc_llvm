// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck %s

void main(float x0, int x1, int x2, int dest0, int dest1)
{
    
    float64 __local *ptr_x1 = (float64 __local *)x1;
    float64 __local *ptr_x2 = (float64 __local *)x2;
    
    float64 __local *res0 = (float64  __local *)dest0;
    float64 temp_res0 = 0;
    temp_res0 = v_f32_form_fp_num_b((float64)8, *ptr_x1, *ptr_x2, SW_PRE_LOG_FUNC, temp_res0, 1, 0);
    *res0 = temp_res0;
    
    float64 __local *res1 = (float64  __local *)dest1;
    float64 temp_res1 = 0;
    temp_res1 = v_f32_form_fp_num_b(x0, *ptr_x1, *ptr_x2, SW_PRE_LOG_FUNC, temp_res0, 1, 0);
    *res1 = temp_res1;
}
//CHECK-DAG: form_fp_num.f32 special_func=5 %V{{[0-9]+}}, 0x41000000, %V{{[0-9]+}}, %V{{[0-9]+}}
//CHECK-DAG: form_fp_num.f32 special_func=5 %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
