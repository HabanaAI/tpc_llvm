// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main()
{
    
    int5 indx0 = {0,0,0,0,0};
    int5 indx1 = {0,0,0,0,0};
    int5 res0 = 0; 

    res0 = i_i32_and_i_i(indx0, indx1, res0, 1);
    float64 temp0 = 0;
    f32_st_tnsr_i_v(res0, 1, temp0);
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: and.i32 b00001 %I2, 0x0, %I2, %SP0
