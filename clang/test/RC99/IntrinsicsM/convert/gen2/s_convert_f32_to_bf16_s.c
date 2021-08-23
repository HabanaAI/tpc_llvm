// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(float x0, int dest0)
{
    
    
    
   bf16 __local *res0 = (bf16  __local *)dest0;
    bf16 temp_res0 = 0;
    temp_res0 = s_convert_f32_to_bf16_s(x0, 1);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: convert.f32 target_type=bf16 rz %S{{[0-9]+}}, %S{{[0-9]+}}, %SP0
