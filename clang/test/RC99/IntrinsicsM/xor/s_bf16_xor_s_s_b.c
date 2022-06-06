// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(bf16 x0, bf16 x1, int x2, int dest0, int dest1)
{
    
    
    
   bf16 __local *res0 = (bf16  __local *)dest0;
    bf16 temp_res0 = 0;
    temp_res0 = s_bf16_xor(x0, x1, 0, temp_res0, x2, 0);
    *res0 = temp_res0;
    
   bf16 __local *res1 = (bf16  __local *)dest1;
    bf16 temp_res1 = 0;
    temp_res1 = s_bf16_xor(x0, 8.bf, 0, temp_res1, x2, 0);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: xor.bf16 %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
//CHECK-ASM-DAG: xor.bf16 %S{{[0-9]+}}, %S{{[0-9]+}}, 0x4100, %SP{{[0-9]+}}
