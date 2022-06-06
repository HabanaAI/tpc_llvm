// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x1, int x2, int dest0)
{
    
    minifloat256 __local *ptr_x0 = (minifloat256 __local *)x0;
    minifloat256 __local *ptr_x1 = (minifloat256 __local *)x1;
    minifloat256 __local *ptr_x2 = (minifloat256 __local *)x2;
    
   minifloat256 __local *res0 = (minifloat256  __local *)dest0;
    minifloat256 temp_res0 = 0;
    temp_res0 = v_f8_form_fp_num_b(*ptr_x0, *ptr_x1, *ptr_x2, SW_ADD_BIAS, 0, 1, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: form_fp_num.f8_143 exp_add_bias %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}
