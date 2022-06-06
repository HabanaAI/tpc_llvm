// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi  %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0)
{
    
    int5 indx1 = {0,x0,0,x0,0};
    int5 res0 = 0; 

    res0 = i_i32_and(1, indx1, 20, 0, res0, 1, 0);
    float64 temp0 = 0;
    v_f32_st_tnsr(res0, 1, temp0, 0, 1, 0);
    int5 res1 = 0; 

    res1 = i_i32_and(x0, indx1, 21, 0, res1, 1, 0);
    float64 temp1 = 0;
    v_f32_st_tnsr(res1, 1, temp1, 0, 1, 0);
}
//CHECK-ASM-DAG: and.i32 b10100 %I{{[0-9]}}, 0x1, %I{{[0-9]}}
//CHECK-ASM-DAG: and.i32 b10101 %I{{[0-9]}}, %S0, %I{{[0-9]}}
