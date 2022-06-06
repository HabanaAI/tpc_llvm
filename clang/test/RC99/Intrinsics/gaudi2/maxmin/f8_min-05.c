// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu doron1 %s -o - | FileCheck %s

void main(minifloat x0, minifloat x1, int dest0, int dest1)
{
   minifloat __local *res0 = (minifloat  __local *)dest0;
    minifloat temp_res0 = 0;
    temp_res0 = s_f8_min(x0, x1,SW_SUP_NAN,temp_res0,1,0);
    *res0 = temp_res0;
    
    minifloat __local *res1 = (minifloat  __local *)dest1;
    minifloat temp_res1 = 0;
    temp_res1 = s_f8_min(x0, (minifloat)8,SW_SUP_NAN,temp_res0,1,0);
    *res1 = temp_res1;
}
//CHECK: min.f8_143 sup_nan %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
//CHECK: min.f8_143 sup_nan %S{{[0-9]+}}, %S{{[0-9]+}}, 0x50
