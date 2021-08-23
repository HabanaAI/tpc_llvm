// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x3)
{
    
    int5 indx1 = {0,x0,0,x0,0};
    int5 res0 = 0; 

    res0 = i_i32_xor_s_i_b(1, indx1, res0, 20, x3, 0);
    float64 temp0 = 0;
    f32_st_tnsr_i_v(res0, 1, temp0);
    int5 res1 = 0; 

    res1 = i_i32_xor_s_i_b(x0, indx1, res1, 21, x3, 0);
    float64 temp1 = 0;
    f32_st_tnsr_i_v(res1, 1, temp1);
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: xor.i32  b10100  %I4, 0x1, %I2, %SP1
//CHECK-ASM-DAG: xor.i32 b10101  %I3, %S0, %I2, %SP1
