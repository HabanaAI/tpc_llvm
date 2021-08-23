// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi  %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0)
{
    
    int5 indx1 = {0,x0,0,x0,0};
    int5 res0 = 0; 

    res0 = i_i32_xor_s_i(1, indx1, res0, 20);
    float64 temp0 = 0;
    f32_st_tnsr_i_v(res0, 1, temp0);
    int5 res1 = 0; 

    res1 = i_i32_xor_s_i(x0, indx1, res1, 21);
    float64 temp1 = 0;
    f32_st_tnsr_i_v(res1, 1, temp1);
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: xor.i32 b10100 %I{{[0-9]}}, 0x1, %I{{[0-9]}}
//CHECK-ASM-DAG: xor.i32 b10101 %I{{[0-9]}}, %S0, %I{{[0-9]}}
