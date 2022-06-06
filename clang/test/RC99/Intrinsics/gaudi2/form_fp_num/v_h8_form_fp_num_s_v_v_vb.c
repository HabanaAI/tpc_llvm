// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(minihalf x0, int x1, int x2, int dest0, int dest1)
{
    unsigned a = 1;
    minihalf256 __local *ptr_x1 = (minihalf256 __local *)x1;
    minihalf256 __local *ptr_x2 = (minihalf256 __local *)x2;
    bool256 pred4;
    pred4 =  v_i1_mov_i1_b(1, 0, (bool256){0}, 1, 0);
    
   minihalf256 __local *res0 = (minihalf256  __local *)dest0;
    minihalf256 temp_res0 = 0;
    temp_res0 = v_h8_form_fp_num_vb((minihalf)8, *ptr_x1, *ptr_x2, SW_ADD_BIAS, temp_res0, (pred4), 0);
    *res0 = temp_res0;
    
   minihalf256 __local *res1 = (minihalf256  __local *)dest1;
    minihalf256 temp_res1 = 0;
    temp_res1 = v_h8_form_fp_num_vb(x0, *ptr_x1, *ptr_x2, SW_ADD_BIAS, temp_res1, (pred4), 0);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: form_fp_num.f8_152 exp_add_bias %V{{[0-9]+}}, 0x48, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
//CHECK-ASM-DAG: form_fp_num.f8_152 exp_add_bias %V{{[0-9]+}}, %S{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
