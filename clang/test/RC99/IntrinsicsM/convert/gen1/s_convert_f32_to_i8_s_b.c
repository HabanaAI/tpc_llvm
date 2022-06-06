// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -tpc-special -mllvm -tpc-hwwa-conv-maxint=0  %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(float x0, int x2, int dest0)
{
    
    
    
   signed char __local *res0 = (signed char  __local *)dest0;
    signed char temp_res0 = 0;
    temp_res0 = s_convert_f32_to_i8(x0, SW_RD, temp_res0, x2, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: convert.f32 target_type=int8 rd %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
