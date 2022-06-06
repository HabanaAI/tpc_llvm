// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck --check-prefix=CHECK-ASM %s

void main(short x0, int dest0)
{
    
    
    
   int __local *res0 = (int  __local *)dest0;
    int temp_res0 = 0;
    temp_res0 = s_convert_i16_to_i32(x0, SW_CSR, 0, 1, 0);
    *res0 = temp_res0;
}
//CHECK-ASM: .globl main
//CHECK-ASM-DAG: convert.i16 target_type=int32 %S{{[0-9]+}}, %S{{[0-9]+}}
