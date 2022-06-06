// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(signed char x0, int dest0)
{
    
    
    
   int __local *res0 = (int  __local *)dest0;
    int temp_res0 = 0;
    temp_res0 = s_convert_i8_to_i32(x0, SW_CSR, 0, 1, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: convert.i8 target_type=int32 %S{{[0-9]+}}, %S{{[0-9]+}}
