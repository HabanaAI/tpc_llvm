// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -tpc-special -mllvm -tpc-hwwa-conv-maxint=0  %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(float x0, int x2, int dest0)
{
    
    
    
   short __local *res0 = (short  __local *)dest0;
    short temp_res0 = 0;
    temp_res0 = s_convert_f32_to_i16_s_b(x0, temp_res0, e_round_down, x2, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: convert.f32 target_type=int16 rd %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
