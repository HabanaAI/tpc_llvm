// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(minifloat x0, int x1, int x2, int x4, int dest0, int dest1)
{
    
    minifloat256 __local *ptr_x1 = (minifloat256 __local *)x1;
    minifloat256 __local *ptr_x2 = (minifloat256 __local *)x2;
    
   minifloat256 __local *res0 = (minifloat256  __local *)dest0;
    minifloat256 temp_res0 = 0;
    temp_res0 = v_f8_form_fp_num_b((minifloat)8, *ptr_x1, *ptr_x2, SW_ADD_BIAS, temp_res0, x4, 0);
    *res0 = temp_res0;
    
   minifloat256 __local *res1 = (minifloat256  __local *)dest1;
    minifloat256 temp_res1 = 0;
    temp_res1 = v_f8_form_fp_num_b(x0, *ptr_x1, *ptr_x2, SW_ADD_BIAS, temp_res1, x4, 0);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: form_fp_num.f8_143 exp_add_bias %V{{[0-9]+}}, 0x50, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
//CHECK-ASM-DAG: form_fp_num.f8_143 exp_add_bias %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
