// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(float x0, int dest0)
{
    
    
    
   half __local *res0 = (half  __local *)dest0;
    half temp_res0 = 0;
    temp_res0 = s_convert_f32_to_f16(x0, (1) << 16, 0, 1, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: convert.f32 target_type=f16 rz %S{{[0-9]+}}, %S{{[0-9]+}}
