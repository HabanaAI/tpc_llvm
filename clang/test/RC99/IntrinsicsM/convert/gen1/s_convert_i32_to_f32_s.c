// RUN: %codegen -S -O1 -triple tpc-none-none -tpc-special -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int dest0)
{
    
    
    
   float __local *res0 = (float  __local *)dest0;
    float temp_res0 = 0;
    temp_res0 = s_convert_i32_to_f32_s(x0, e_round_down);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: convert.i32 target_type=fp32 rd %S{{[0-9]+}}, %S{{[0-9]+}}, %SP0
