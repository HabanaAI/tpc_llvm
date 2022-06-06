// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(int x0, int x2, int dest0)
{
    
    
    
   bf16 __local *res0 = (bf16  __local *)dest0;
    bf16 temp_res0 = 0;
    temp_res0 = s_convert_i32_to_bf16(x0, (1) << 16, temp_res0, x2, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: convert.i32 target_type=bf16 rz %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
