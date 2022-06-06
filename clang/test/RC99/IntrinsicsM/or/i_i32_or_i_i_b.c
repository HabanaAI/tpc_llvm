// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x3)
{
    
    int5 indx0 = {0,1,0,0,0};
    int5 indx1 = {0,0,1,0,0};
    int5 res0 = 0; 

    res0 = i_i32_or(indx0, indx1, 20, 0, res0, x3, 0);
    float64 temp0 = 0;
    v_f32_st_tnsr(res0, 1, temp0, 0, 1, 0);
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: or.i32  b10100  %I2, %I3, %I4, %SP1
