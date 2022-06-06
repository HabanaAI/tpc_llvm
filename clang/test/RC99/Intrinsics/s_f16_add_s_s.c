// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(half x0, half x1, int dest0, int dest1)
{
    
    
    
   half __local *res0 = (half  __local *)dest0;
    half temp_res0 = 0;
    temp_res0 = s_f16_add(x0, x1, 0, 0, 1, 0);
    *res0 = temp_res0;
    
    half __local *res1 = (half  __local *)dest1;
    half temp_res1 = 0;
    temp_res1 = s_f16_add(x0, (half)8.0, 0, 0, 1, 0);
    *res1 = temp_res1;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: add.f16 %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
//CHECK-ASM-DAG: add.f16 %S{{[0-9]+}}, %S{{[0-9]+}}, 0x4800
