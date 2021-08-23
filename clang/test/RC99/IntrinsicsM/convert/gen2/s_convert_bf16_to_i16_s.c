// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(bf16 x0, int dest0)
{
    
    
    
   short __local *res0 = (short  __local *)dest0;
    short temp_res0 = 0;
    temp_res0 = s_convert_bf16_to_i16_s(x0, 1);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: convert.bf16 target_type=int16 rz %S{{[0-9]+}}, %S{{[0-9]+}}, %SP0
